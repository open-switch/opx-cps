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

#include "cps_string_utils.h"

#include "event_log.h"

#include <poll.h>
#include <netinet/tcp.h>
#include <unordered_map>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <functional>


const static ssize_t MAX_RETRY=1;

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
    if (_ctx!=nullptr) {
        redisFree(static_cast<redisContext*>(_ctx));
        _ctx = nullptr;
    }
}

bool cps_db::connection::reconnect() {
    if (_addr.size()==0) return false;
    disconnect();
    return connect(_addr);
}

int cps_db::connection::get_fd() {
    return static_cast<redisContext*>(_ctx)->fd;
}


pthread_once_t __thread_init = PTHREAD_ONCE_INIT;

bool cps_db::connection::clone(connection &conn) {
    conn.disconnect();
    return conn.connect(_addr);
}

bool cps_db::connection::connect(const std::string &s_, const std::string &db_instance_, bool async_) {
    size_t _port_pos = s_.rfind(":");
    if (_port_pos==std::string::npos) {
        EV_LOG(INFO,DSAPI,0,"RED-CON","Failed to connect to server... bad address (%s)",s_.c_str());
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
            EV_LOG(INFO,DSAPI,0,"RED-CON","Failed to connect to server... bad address (%s)",s_.c_str());
            return false;
        }
    }
    _addr = s_;
    _async = async_;

    _ip = s_.substr(_start_ip,_end_ip);

    if (_async) {
        _ctx = redisAsyncConnect(_ip.c_str(), _port);
    } else {
        timeval tv = { 2 /*Second */ , 0 };
        _ctx = redisConnectWithTimeout(_ip.c_str(), _port,tv);
    }

    if (db_instance_.size()!=0) {
        select_db(*this,db_instance_);
    }

    if (_ctx!=nullptr && ((redisContext*)_ctx)->fd >=0) {
        auto fd = ((redisContext*)_ctx)->fd;
        int on = 1;

        if (setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,&on,sizeof(on))<0) {
            EV_LOGGING(DSAPI,DEBUG,"CPS-DB-CONN","Failed to set keepalive option on fd %d",fd);
        }
        int retries = 4;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPCNT,&retries,sizeof(retries))<0) {
            EV_LOGGING(DSAPI,DEBUG,"CPS-DB-CONN","Failed to set keepcount option on fd %d",fd);
        }

        int interval = 1;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPINTVL,&interval,sizeof(interval))<0) {
            EV_LOGGING(DSAPI,DEBUG,"CPS-DB-CONN","Failed to set interval option on fd %d",fd);
        }

        int idle = 2;
        if (setsockopt(fd,IPPROTO_TCP,TCP_KEEPIDLE,&idle,sizeof(idle))<0) {
            EV_LOGGING(DSAPI,DEBUG,"CPS-DB-CONN","Failed to set idle time option on fd %d",fd);
        }
    }

    return _ctx !=nullptr;
}

namespace {

struct request_walker_contexct_t {
    static const size_t MAX_EXP_CMD{10};

    const char *cmds[MAX_EXP_CMD];
    size_t cmds_lens[MAX_EXP_CMD];

    const char **cmds_ptr = cmds;
    size_t *cmds_lens_ptr = cmds_lens;

    std::vector<std::vector<char>> key_scratch;
    ssize_t key_scratch_len=-1;

    std::vector<std::vector<char>> key;  //just a buffer to hold the key portion of the data structure until we call into REDIS
    size_t scratch_pad_ix=0;

    bool set(cps_db::connection::db_operation_atom_t * lst_,size_t len_);

    request_walker_contexct_t(cps_db::connection::db_operation_atom_t * lst_,size_t len_) {
        if (!set(lst_,len_)) {
            cmds_ptr = cmds;
            cmds_lens_ptr = cmds_lens;
        }
    }

    request_walker_contexct_t() {
        cmds_ptr = cmds;
        cmds_lens_ptr = cmds_lens;
    }

    cps_db::connection::db_operation_atom_t * _cur=nullptr;

    bool enough(size_t cnt) {
        if ((cmds_ptr + cnt) > (cmds + MAX_EXP_CMD)) {
            return false;
        }
        return true;
    }
    void set_current_entry(const char *data, size_t len) {
        *(cmds_ptr++) = data;
        *(cmds_lens_ptr++) = len;
    }
    bool valid() { return cmds_ptr!=cmds; }
};
bool handle_str(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    ctx.set_current_entry(ctx._cur->_string,ctx._cur->_len);
    return true;
}

bool handle_class_key(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    ctx.key_scratch.resize(ctx.key_scratch.size()+1);
    if (!cps_db::dbkey_from_class_key(ctx.key_scratch[++ctx.key_scratch_len],cps_api_object_key(ctx._cur->_object))) return false;
    ctx.set_current_entry(&ctx.key_scratch[ctx.key_scratch_len][0],ctx.key_scratch[ctx.key_scratch_len].size());
    return true;
}

bool handle_instance_key(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;
    ctx.key_scratch.resize(ctx.key_scratch.size()+1);
    if (!cps_db::dbkey_from_instance_key(ctx.key_scratch[++ctx.key_scratch_len],ctx._cur->_object,false)) return false;
    ctx.set_current_entry(&ctx.key_scratch[ctx.key_scratch_len][0],ctx.key_scratch[ctx.key_scratch_len].size());
    return true;
}

bool handle_object(request_walker_contexct_t &ctx) {
    if (!handle_instance_key(ctx)) return false;
    if (!ctx.enough(2)) return false;

    ctx.set_current_entry("object",strlen("object"));

    const char *d = (const char*)(const void *)cps_api_object_array(ctx._cur->_object);
    size_t dl = cps_api_object_to_array_len(ctx._cur->_object);

    ctx.set_current_entry(d,dl);
    return true;
}

bool handle_object_data(request_walker_contexct_t &ctx) {
    if (!ctx.enough(1)) return false;

    const char *d = (const char*)(const void *)cps_api_object_array(ctx._cur->_object);
    size_t dl = cps_api_object_to_array_len(ctx._cur->_object);

    ctx.set_current_entry(d,dl);
    return true;
}

bool handle_event_fields(request_walker_contexct_t &ctx) {
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
        if (!_map->at((int)_cur->_atom_type)(*this)) return false;
    }
    return true;
}

}

bool cps_db::connection::operation(db_operation_atom_t * lst_,size_t len_, bool force_push,size_t timeoutms) {
    request_walker_contexct_t ctx(lst_,len_);

    if (!ctx.valid()) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-OP","The DB context is invalid.");
        return false;
    }

    bool _success = false;
    ssize_t retry = MAX_RETRY;
    do {
        if (redisAppendCommandArgv(static_cast<redisContext*>(_ctx),ctx.cmds_ptr - ctx.cmds,ctx.cmds,ctx.cmds_lens)==REDIS_OK) {
            _success = true; break;
        }
        EV_LOG(ERR,DSAPI,0,"CPS-RED-CON-OP","Seems to be an issue with the REDIS request - (first entry: %s)",ctx.cmds[0]);
        reconnect();
    } while (retry-->0);

    if (!_success) return false;
    if (!force_push) return true;
    return flush(timeoutms);
}

bool cps_db::connection::flush(size_t timeoutms) {
    int _is_done=0;
    while (_is_done==0) {

        struct pollfd _fds;
        _fds.fd = get_fd();
        _fds.events = POLLOUT;
        _fds.revents =0;
        if (poll(&_fds,1,timeoutms)!=1) {
            reconnect(); return false;
        }
        if (_fds.revents != POLLOUT) {
            reconnect(); return false;
        }

        if (redisBufferWrite(static_cast<redisContext*>(_ctx),&_is_done)==REDIS_ERR) {
            reconnect();
            return false;
        }
    }
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

bool cps_db::connection::response(response_set &data_, bool expect_events,size_t timeoutms) {
    bool _rc = true;
    if (!flush(timeoutms)) {
        _rc=false;
    }

    while (_rc) {
        void *reply;

        if (redisGetReplyFromReader(static_cast<redisContext*>(_ctx),&reply)!=REDIS_OK) {
            EV_LOG(ERR,DSAPI,0,"DB-RESP","Failed to get reply from %s",_addr.c_str());
            //clean up response so no partial responses
            _rc=false; continue;
        }

        if (reply==nullptr) {
            struct pollfd _fds;
            _fds.fd = get_fd();
            _fds.events = POLLIN;
            _fds.revents =0;
            if (poll(&_fds,1,timeoutms)!=1) {
                _rc=false; continue;
            }
            if (_fds.revents != POLLIN) {
                _rc=false; continue;
            }
            if (redisBufferRead(static_cast<redisContext*>(_ctx))!=REDIS_OK) {
                _rc=false; continue;
            }
            continue;
        }

        if (expect_events && is_event_message(reply)) {
            _pending_events.push_back(reply);
            continue;
        }
        data_.add(reply);
        break;
    }
    if (_rc==false) {
        reconnect();
    }
    return _rc;
}

bool cps_db::connection::command(db_operation_atom_t * lst,size_t len,response_set &set,size_t timeoutms) {
    if (!operation(lst,len,true,timeoutms)) {
        return false;
    }
    return response(set,false,timeoutms);
}


void cps_db::connection::update_used() {
    _last_used = std_get_uptime(nullptr);
}

bool cps_db::connection::timedout(uint64_t relative) {
    return std_time_is_expired(_last_used,relative);
}

bool cps_db::connection::has_event(bool &err) {
    void *rep ;
    int rc = REDIS_OK;
    do {
        if ((rc=redisReaderGetReply(static_cast<redisContext*>(_ctx)->reader,&rep))==REDIS_OK) {
            if (rep==nullptr) break;
            if (is_event_message(rep)) _pending_events.push_back(rep);
            else freeReplyObject(rep);
        } else {
            err = true;
        }
    } while (rc==REDIS_OK);
    return _pending_events.size()>0;
}

#include "std_select_tools.h"

bool cps_db::connection::get_event(response_set &data, bool &err_occured) {
    do {
        if (_pending_events.size() == 0) {
            struct pollfd _fds;
            _fds.fd = get_fd();
            _fds.events = POLLIN;
            _fds.revents =0;
            if (poll(&_fds,1,_SELECT_MS_WAIT)!=1) {
                err_occured=true;
                break;
            }

            if (redisBufferRead(static_cast<redisContext*>(_ctx))==REDIS_ERR) {
                err_occured=true;
                break;
            }
            if (!has_event(err_occured)) break;

        }
        if (_pending_events.size()>0) {
            data.add(*_pending_events.begin());
            _pending_events.erase(_pending_events.begin());
            return true;
        }
    } while (0);
    return false;
}

static pthread_once_t  onceControl = PTHREAD_ONCE_INIT;
static cps_db::connection_cache * _cache;
static cps_db::connection_cache_events * _event_cache;

void __init(void) {
    _cache = new cps_db::connection_cache;
    _event_cache = new cps_db::connection_cache_events;
    STD_ASSERT(_cache!=nullptr && _event_cache!=nullptr);
}

cps_db::connection_cache & cps_db::ProcessDBCache() {
    pthread_once(&onceControl,__init);
    return *_cache;
}

cps_db::connection_cache_events & cps_db::ProcessDBEvents() {
    pthread_once(&onceControl,__init);
    return *_event_cache;
}

cps_db::connection * cps_db::connection_cache::get(const std::string &name, bool check_alive) {
    std::lock_guard<std::mutex> l(_mutex);

    const auto &_connect = [] (const std::string &name) -> cps_db::connection *{
        auto *_conn = new cps_db::connection;
        if (!_conn->connect(name)) {
            delete _conn;
            _conn  = nullptr;
        }
        return _conn;
    };

    while (true) {
        auto it = _pool.find(name);
        if (it==_pool.end() || it->second.size()==0) break;

        auto ptr = it->second.back().release();
        it->second.pop_back();
        if (check_alive && ptr->timedout(CONN_TIMEOUT_CHECK)) {
            bool rc = cps_db::ping(*ptr);
            if (!rc) {
                EV_LOGGING(CPS,WARNING,"CPS-CONN-CACHE","Cache entry for DB connection stale for %s - getting second",it->first.c_str());
                delete ptr;
                continue;
            }
        }
        return ptr;
    }
    return _connect(name);
}

void cps_db::connection_cache::put(const std::string &name, connection* conn) {
    std::lock_guard<std::mutex> l(_mutex);
    const static size_t MAX_OPEN_CONN=3;
    auto _ptr = std::unique_ptr<cps_db::connection>(conn);
    if (_ptr.get()!=nullptr) _ptr->update_used();

    if (_pool[name].size()<MAX_OPEN_CONN) {
        _pool[name].push_back(std::move(_ptr));
    }
}

void cps_db::connection_cache::remove(const std::string &name) {
    std::lock_guard<std::mutex> l(_mutex);
    _pool.erase(name);
}

cps_db::connection_request::connection_request(cps_db::connection_cache & cache,const char *addr) : _cache(cache) {
    _name = addr;
    _conn = _cache.get(addr);
}

cps_db::connection_request::~connection_request() {
    if(_conn!=nullptr) {
        _cache.put(_name,_conn);
        _conn = nullptr;
    }
}



