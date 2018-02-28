/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

#include "cps_api_db.h"
#include "cps_api_db_response.h"
#include "cps_api_db_connection_tools.h"

#include "cps_class_map_query.h"
#include "cps_string_utils.h"
#include "cps_api_core_utils.h"
#include "event_log.h"


#include <netinet/tcp.h>
#include <unordered_map>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static pthread_once_t  onceControl = PTHREAD_ONCE_INIT;

static ssize_t _operation_retry_flags=1;

static ssize_t _timeout_local=2000;
static ssize_t _timeout_remote=2000;

static void __init(void) {
    cps_api_update_ssize_on_param_change("cps.connect.operation-retry",
            &_operation_retry_flags);
    cps_api_update_ssize_on_param_change("cps.channel.timeout-remote",
            &_timeout_remote);
    cps_api_update_ssize_on_param_change("cps.channel.timeout-local",
            &_timeout_local);
    cps_api_update_ssize_on_param_change("cps.channel.timeout-global",
            &_timeout_remote);
    cps_api_update_ssize_on_param_change("cps.channel.timeout-global",
            &_timeout_local);
}

void cps_db::connection::db_operation_atom_t::from_string(const char *str, size_t len) {
    _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_STRING;
    _string = str;
    _len = len;
}

void cps_db::connection::db_operation_atom_t::from_string(const char *str) {
    return from_string(str,strlen(str));
}

void cps_db::connection::db_operation_atom_t::for_event(cps_api_object_t obj) {
    _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_EVENT_DATA;
    _object = obj;
}

void cps_db::connection::db_operation_atom_t::from_object(cps_api_object_t obj, bool instance, bool data) {
    if (!instance) _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_CLASS;
    else if (instance && data) _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_KEY_AND_DATA;
    else _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_INSTANCE;
    _object = obj;
}

void cps_db::connection::disconnect() {
    EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-DISCONNECT","Disconnected from %s("
                "source address %s) - was connected (%d) - stack trace (%s)",
                _addr.c_str(),_local.c_str(),_ctx!=nullptr,cps_api_stacktrace().c_str());

    if (_ctx!=nullptr) {
        redisFree(static_cast<redisContext*>(_ctx));
        _ctx = nullptr;
    }
    _event_connection = false;
    for ( auto & it : _pending_events) {
        freeReplyObject(it);
    }
    _pending_events.clear();
}

bool cps_db::connection::reconnect() {
    if (_addr.size()==0) return false;
    disconnect();
    return connect(_addr);
}

int cps_db::connection::get_fd() {
    if (_ctx==nullptr) return -1;
    return static_cast<redisContext*>(_ctx)->fd;
}

bool cps_db::connection::clone(connection &conn) {
    conn.disconnect();
    return conn.connect(_addr);
}

bool cps_db::connection::connect(const std::string &s_, const std::string &db_instance_) {
    pthread_once(&onceControl,__init);

    _last_used = 0;

    size_t _port_pos = s_.rfind(":");
    if (_port_pos==std::string::npos) {
        EV_LOGGING(CPS-DB-CONN,INFO,"RED-CON","Failed to connect to server... bad address (%s)",s_.c_str());
        return false;
    }

    int _port = atoi(s_.c_str()+_port_pos+1);

    size_t _end_ip = _port_pos;
    size_t _start_ip = 0;

    std::string _ip;

    if (s_[0]=='[') {
        ++_start_ip;    //first spot is [
        _end_ip = s_.rfind("]:");
        if (_end_ip==std::string::npos) {
            EV_LOGGING(CPS-DB-CONN,INFO,"RED-CON","Failed to connect to server... bad address (%s)",s_.c_str());
            return false;
        }
    }
    _addr = s_;

    _ip = s_.substr(_start_ip,_end_ip);

    timeval tv = { _timeout_remote/1000 /*millisec to sec*/ , 0 /*ignore remainder currently*/ };
    _ctx = redisConnectWithTimeout(_ip.c_str(), _port,tv);

    if (_ctx==nullptr) {
        EV_LOGGING(CPS-DB-CONN,INFO,"RED-CON","Failed to connect to server... malloc failed for (%s)",s_.c_str());
    //memory allocation
        return false;
    }
    bool _connected = (((redisContext*)_ctx)->flags & REDIS_CONNECTED)!=0;
    if (!_connected) {
        EV_LOGGING(CPS-DB-CONN,INFO,"RED-CON","Failed to connect to server - no channel (%s)",s_.c_str());
        redisFree(static_cast<redisContext*>(_ctx));
        _ctx = nullptr;
        return false;
    }

    if (db_instance_.size()!=0) {
        select_db(*this,db_instance_);
    }

    if (_ctx!=nullptr && ((redisContext*)_ctx)->fd >=0) {
        auto fd = ((redisContext*)_ctx)->fd;
        int on = 1;

        { //peer display
        socklen_t _len;
        struct sockaddr_storage addr;
        _len = sizeof(addr);
        if (getsockname(fd,(struct sockaddr*)&addr,&_len)==0) {
            char buff[INET6_ADDRSTRLEN];
            int port;
            if (addr.ss_family==AF_INET) {
                struct sockaddr_in *s = (struct sockaddr_in*)&addr;
                port = ntohs(s->sin_port);
                inet_ntop(AF_INET,&s->sin_addr,buff,sizeof(buff));
                _local = cps_string::sprintf("%s:%d",buff,port);
            } else if (addr.ss_family==AF_INET6) {
                struct sockaddr_in6 *s = (struct sockaddr_in6*)&addr;
                port = ntohs(s->sin6_port);
                inet_ntop(AF_INET6,&s->sin6_addr,buff,sizeof(buff));
                _local = cps_string::sprintf("%s[%d]",buff,port);
            }
            EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-PEER","Connected to %s from "
                "%s on fd %d",_addr.c_str(),_local.c_str(),fd);
        }
        }
        //set up poll structures for later use
        _rd.fd = fd;
        _rd.events = POLLIN;
        _rd.revents =0;

        _wr.fd = fd;
        _wr.events = POLLOUT;
        _wr.revents =0;

        //attempt to set socket options for keepalive
        if (setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on))<0) {
            EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-CONN","Failed to set keepalive option on fd %d",fd);
        }
        int retries = 4;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPCNT,&retries,sizeof(retries))<0) {
            EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-CONN","Failed to set keepcount option on fd %d",fd);
        }

        int interval = 1;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&interval,sizeof(interval))<0) {
            EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-CONN","Failed to set interval option on fd %d",fd);
        }

        int idle = 2;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&idle,sizeof(idle))<0) {
            EV_LOGGING(CPS-DB-CONN,DEBUG,"CPS-DB-CONN","Failed to set idle time option on fd %d",fd);
        }
    }
    return _ctx !=nullptr;
}

namespace {


struct request_walker_contexct_t {
    static const size_t MAX_EXP_CMD = 10;

    //buffers
    const char *cmds[MAX_EXP_CMD];
    size_t cmds_lens[MAX_EXP_CMD];

    //temp locations
    const char **cmds_ptr = cmds;
    size_t *cmds_lens_ptr = cmds_lens;
    size_t _cmds_used = 0;

    std::vector<std::vector<char>> key_scratch;
    ssize_t key_scratch_len=-1;

    std::vector<std::vector<char>> key;  //just a buffer to hold the key portion of the data structure until we call into REDIS
    size_t scratch_pad_ix=0;

    cps_db::connection::db_operation_atom_t * _cur=nullptr;

    bool set(cps_db::connection::db_operation_atom_t * lst_,size_t len_);

    request_walker_contexct_t(cps_db::connection::db_operation_atom_t * lst_,size_t len_) {
        if (!set(lst_,len_)) {
            cmds_ptr = cmds;
            cmds_lens_ptr = cmds_lens;
        }
    }

    std::vector<char> &alloc_key() {
    	size_t _ix = key.size();
    	key.resize(_ix+1);
    	return key[_ix];
    }

    void reset() {
    	_cmds_used = 0;
        cmds_ptr = cmds;
        cmds_lens_ptr = cmds_lens;
    }

    //most class variables are automatically done via class declared initialization
    request_walker_contexct_t() {
    	reset();
    }

    bool enough(size_t cnt) {
    	return ((_cmds_used+cnt) < MAX_EXP_CMD);
    }

    void set_current_entry(const char *data, size_t len) {
        *(cmds_ptr++) = data;
        *(cmds_lens_ptr++) = len;
        ++_cmds_used;
    }
    bool valid() { return (_cmds_used > 0) && cmds_ptr!=cmds; }

};

static bool handle_str(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    ctx.set_current_entry(ctx._cur->_string,ctx._cur->_len);
    return true;
}

static bool handle_class_key(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    auto &_key_space = ctx.alloc_key();

    if (!cps_db::dbkey_from_class_key(_key_space,cps_api_object_key(ctx._cur->_object))) return false;
    ctx.set_current_entry(&_key_space[0],_key_space.size());

    return true;
}

static bool handle_instance_key(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    auto &_key_space = ctx.alloc_key();

    if (!cps_db::dbkey_from_instance_key(_key_space,ctx._cur->_object,false)) return false;
    ctx.set_current_entry(&_key_space[0],_key_space.size());
    return true;
}

static bool handle_object_data(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;

    const char *d = (const char *)(const void *)cps_api_object_array(ctx._cur->_object);
    size_t dl = cps_api_object_to_array_len(ctx._cur->_object);

    ctx.set_current_entry(d,dl);
    return true;
}

static bool handle_object(request_walker_contexct_t &ctx) {
    if (!handle_instance_key(ctx)) return false;

    if (!ctx.enough(1)) return false;

    static const char *_obj_name = "object";
    static const size_t _obj_len = strlen("object");
    ctx.set_current_entry(_obj_name,_obj_len);

    return handle_object_data(ctx);
}

static bool handle_event_fields(request_walker_contexct_t &ctx) {
    if (!handle_instance_key(ctx)) return false;
    return handle_object_data(ctx);
}

bool request_walker_contexct_t::set(cps_db::connection::db_operation_atom_t * lst_,size_t len_) {
    static const auto *_map = new std::unordered_map<int,std::function<bool(request_walker_contexct_t&)>> {
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_STRING,handle_str},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_CLASS,handle_class_key},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_INSTANCE,handle_instance_key},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_KEY_AND_DATA,handle_object},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_DATA,handle_object_data},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_EVENT_DATA,handle_event_fields},
    };

    size_t iter = 0;
    for ( ; iter < len_; ++iter ) {
        _cur = lst_+iter;
        if (!_map->at((int)_cur->_atom_type)(*this)) {
        	reset();
        	return false;
        }
    }
    return true;
}

inline void SET_RC(cps_api_return_code_t *rc, cps_api_return_code_t val) {
	if (rc!=nullptr) *rc = val;
}

inline void SET_RC_FROM_ERRNO(cps_api_return_code_t *rc, int poll_rc) {
    if (poll_rc==-1) SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);
    if (poll_rc==0) SET_RC(rc,cps_api_ret_code_TIMEOUT);
}

}


bool cps_db::connection::writable(size_t timeoutms, cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_ERR);
    int _rc = poll(&_wr,1,timeoutms);
    SET_RC_FROM_ERRNO(rc,_rc);

    bool _writable =(_rc==1 && (_wr.revents&POLLOUT)!=0);

    if (_writable) SET_RC(rc,cps_api_ret_code_OK);
    return _writable;
}

bool cps_db::connection::readable(size_t timeoutms, cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_ERR);

    int _rc = poll(&_rd,1,timeoutms);
    SET_RC_FROM_ERRNO(rc,_rc);

    bool _readable = _rc==1 && (_rd.revents&POLLIN)!=0;
    if (_readable) SET_RC(rc,cps_api_ret_code_OK);

    return _readable;
}

bool cps_db::connection::operation(db_operation_atom_t * lst_,size_t len_,
		bool force_push,size_t timeoutms, cps_api_return_code_t *rc) {
    request_walker_contexct_t ctx(lst_,len_);

    SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);

    if (!ctx.valid()) {
        EV_LOGGING(CPS-DB-CONN,ERR,"CPS-DB-OP","The DB context is invalid.");
        return false;
    }

    if (_ctx==nullptr) //if the redis connection is not valid its not going to work
        return false;

    bool _success = false;
    ssize_t retry = _operation_retry_flags;

    do {
        if (redisAppendCommandArgv(static_cast<redisContext*>(_ctx),ctx.cmds_ptr - ctx.cmds,ctx.cmds,ctx.cmds_lens)==REDIS_OK) {
            _success = true; break;
        }

        EV_LOGGING(CPS-DB-CONN,ERR,"CPS-RED-CON-OP","Seems to be an issue with the REDIS request - (first entry: %s)",ctx.cmds[0]);
        if (!reconnect()) {
        	return false;
        }
    } while (retry-->0);

    if (!_success) return false;

    SET_RC(rc,cps_api_ret_code_OK);

    if (!force_push) return true;
    return flush(timeoutms,rc);
}



bool cps_db::connection::flush(size_t timeoutms, cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);

    int _is_done=0;
    while (_is_done==0) {
        if (!cps_api_db_is_local_node(_addr.c_str())) {
            if (timeoutms==_SELECT_MS_WAIT)timeoutms = _timeout_remote;
        }
        if (!writable(timeoutms,rc)) {
            EV_LOGGING(CPS-DB-CONN,ERR,"FLUSH","Sending buffer full - terminating.. (%d)",(timeoutms));
            reconnect(); return false;
        }
        if (_ctx==nullptr) //if the redis connection is not valid its not going to work
            return false;

        if (redisBufferWrite(static_cast<redisContext*>(_ctx),&_is_done)==REDIS_ERR) {
            reconnect();
            return false;
        }
        update_used();
    }
    SET_RC(rc,cps_api_ret_code_OK);
    return true;
}

static bool is_event_message(void *resp) {
    cps_db::response r(resp);
    if (r.elements() >=3) {
        cps_db::response hdr(r.element_at(0));
        if (hdr.is_str()) {
            if (strstr(hdr.get_str(),"message")!=nullptr) { //first entry of the array should indicate message or pmessage
                return true;
            }
        }
    }
    return false;
}

bool cps_db::connection::response(response_set &data_, size_t timeoutms,
		cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);

    if (_ctx==nullptr)
        return false;

    bool _rc = true;
    if (!flush(timeoutms,rc)) {
        _rc=false;
    }
    //clear all previous entries
    data_.clear();
    while (_rc) {
        void *reply;

        if (redisGetReplyFromReader(static_cast<redisContext*>(_ctx),&reply)!=REDIS_OK) {
            EV_LOGGING(CPS-DB-CONN,ERR,"DB-RESP","Failed to get reply from %s",_addr.c_str());
            //clean up response so no partial responses
            _rc=false; continue;
        }

        if (reply==nullptr) {
            if (!cps_api_db_is_local_node(_addr.c_str())) {
                if (timeoutms==_SELECT_MS_WAIT)timeoutms = _timeout_remote;
                if (!readable(timeoutms,rc)) {
                    _rc=false; continue;
                }
            }

            if (redisBufferRead(static_cast<redisContext*>(_ctx))!=REDIS_OK) {
                _rc=false; continue;
            }
            update_used();
            continue;
        }

        if (_event_connection && is_event_message(reply)) {
            _pending_events.push_back(reply);
            EV_LOGGING(CPS-DB-CONN,DEBUG,"RESP","Interleaved message received");
            continue;
        }
        data_.add(reply);
        break;
    }
    if (_rc==false) {
        reconnect();
    }
    SET_RC(rc,cps_api_ret_code_OK);
    return _rc;
}

bool cps_db::connection::command(db_operation_atom_t * lst,size_t len,
		response_set &set,size_t timeoutms,
		cps_api_return_code_t *rc) {

    if (!operation(lst,len,true,timeoutms,rc)) {
        return false;
    }
    return response(set,timeoutms,rc);
}

void cps_db::connection::update_used() {
    _last_used = std_get_uptime(nullptr);
}

bool cps_db::connection::used_within(uint64_t relative) {
    return std_time_is_expired(_last_used,relative);
}

bool cps_db::connection::has_event(bool &err, cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);
    if (_ctx==nullptr)
        return false;

    void *rep ;
    int _rc = REDIS_OK;
    err = false;
    do {
        //check redis contexct for already read in data
        if ((_rc=redisReaderGetReply(static_cast<redisContext*>(_ctx)->reader,&rep))==REDIS_OK) {
            if (rep==nullptr) break;
            if (is_event_message(rep)) _pending_events.push_back(rep);
            else freeReplyObject(rep);
        } else {
            err = true;
            EV_LOGGING(CPS-DB-CONN,ERR,"DISCON","DB comm channel buffer seems to be corrupted or invalid.  Disconnect required");
        }
    } while (_rc==REDIS_OK);
    if (!err) SET_RC(rc,cps_api_ret_code_OK);

    return _pending_events.size()>0;
}

#include "std_select_tools.h"

bool cps_db::connection::get_event(response_set &data, bool &err_occured,
		cps_api_return_code_t *rc) {
	SET_RC(rc,cps_api_ret_code_COMMUNICATION_ERROR);

    if (_ctx==nullptr)
        return false;

    data.clear();
    err_occured = false;
    do {
        if (_pending_events.size() == 0) {
            ssize_t timeoutms = _timeout_local;
            if (!cps_api_db_is_local_node(_addr.c_str())) {
                timeoutms = _timeout_remote;
            }
            if (!readable(timeoutms,rc)) {
                EV_LOGGING(CPS-DB-CONN,DEBUG,"DISCON","Read timeout while waiting for data - ignored");
                break;
            }

            if (redisBufferRead(static_cast<redisContext*>(_ctx))==REDIS_ERR) {
                EV_LOGGING(CPS-DB-CONN,ERR,"DISCON","DB event channel back end read error.  Event channel needs to be reconnected");
                err_occured=true;
                break;
            }
            update_used();
            if (!has_event(err_occured,rc)) {
                if (err_occured) EV_LOGGING(CPS-DB-CONN,DEBUG,"DISCON","event caching failure");
                break;
            }

        }
        if (_pending_events.size()>0) {
            data.add(*_pending_events.begin());
            _pending_events.erase(_pending_events.begin());
            SET_RC(rc,cps_api_ret_code_OK);
            return true;
        }
    } while (0);
    if (err_occured==false) SET_RC(rc,cps_api_ret_code_OK);
    return false;
}


