/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

#include "cps_api_db.h"
#include "cps_api_db_response.h"

#include "cps_string_utils.h"

#include "event_log.h"

#include <unordered_map>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <functional>

static bool __added_logging = true;

void cps_db::connection::db_operation_atom_t::from_string(const char *str, size_t len) {
    _atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_STRING;
    _string = str;
    _len = len;
}

void cps_db::connection::db_operation_atom_t::from_string(const char *str) {
    return from_string(str,strlen(str));
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

bool cps_db::connection::connect(const std::string &s_, const std::string &db_instance_, bool async_) {
    auto lst = cps_string::split(s_,":");
    if (lst.size()!=2) {
        EV_LOG(ERR,DSAPI,0,"RED-CON","Failed to connect to server... bad address (%s)",s_.c_str());
        return false;
    }
    _addr = s_;
    _async = async_;

    size_t port = std::atoi(lst[1].c_str());

    if (_async) {
        _ctx = redisAsyncConnect(lst[0].c_str(), port);
    } else {
        _ctx = redisConnect(lst[0].c_str(), port);
    }

    if (db_instance_.size()!=0) {
        select_db(*this,db_instance_);
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
    if (!cps_db::dbkey_from_instance_key(ctx.key_scratch[++ctx.key_scratch_len],ctx._cur->_object)) return false;
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

bool request_walker_contexct_t::set(cps_db::connection::db_operation_atom_t * lst_,size_t len_) {
    static const std::unordered_map<int,std::function<bool(request_walker_contexct_t&)>> _map = {
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_STRING,handle_str},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_CLASS,handle_class_key},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_INSTANCE,handle_instance_key},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_KEY_AND_DATA,handle_object},
        {(int)cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_DATA,handle_object_data},
    };

    size_t iter = 0;
    for ( ; iter < len_; ++iter ) {
        _cur = lst_+iter;
        if (!_map.at((int)_cur->_atom_type)(*this)) return false;
    }
    return true;
}

}

static void __redisCallbackFn__(struct redisAsyncContext*c, void *reply, void *app_context) {

}

bool cps_db::connection::operation(db_operation_atom_t * lst_,size_t len_, bool no_response_) {
    request_walker_contexct_t ctx(lst_,len_);

    if (!ctx.valid()) return false;

    size_t MAX_RETRY=3;
    do {
        if (!_async) {
            if (redisAppendCommandArgv(static_cast<redisContext*>(_ctx),ctx.cmds_ptr - ctx.cmds,ctx.cmds,ctx.cmds_lens)==REDIS_OK) {
                break;
            }
        } else {
            if (redisAsyncCommandArgv(static_cast<redisAsyncContext*>(_ctx),__redisCallbackFn__,nullptr,
                    ctx.cmds_ptr - ctx.cmds,ctx.cmds,ctx.cmds_lens)==REDIS_OK) {
                break;
            }
        }
        EV_LOG(ERR,DSAPI,0,"CPS-RED-CON-OP","Seems to be an issue with the REDIS request - (first entry: %s)",ctx.cmds[0]);
        _pending = 0;
        reconnect();
    } while (MAX_RETRY-->0);

    if (MAX_RETRY < 0) {
        return false;
    }

    if (!no_response_) ++_pending;

    return true;
}

bool cps_db::connection::command(db_operation_atom_t * lst,size_t len,response_set &set) {
    request_walker_contexct_t ctx(lst,len);

    if (!ctx.valid()) return false;
    redisReply *r = (redisReply*)redisCommandArgv(static_cast<redisContext*>(_ctx),ctx.cmds_ptr - ctx.cmds,ctx.cmds,ctx.cmds_lens);
    if (r==nullptr) {
        return false;
    }
    set.add(r);
    return true;
}

bool cps_db::connection::response(response_set &data_, ssize_t at_least_) {
    size_t _needed = at_least_;

    if (_pending<=0 && _needed<=0) {
        EV_LOG(ERR,DSAPI,0,"CPS-DB-RESP","response get called with no expected events");
        return false; //no data to read
    }
    if (_needed <=0 ) _needed = _pending;

    while (_needed-- > 0) {
        void *reply;
        int rc = redisGetReply(static_cast<redisContext*>(_ctx),&reply);
        if (rc!=REDIS_OK) {
            EV_LOG(ERR,DSAPI,0,"DB-RESP","Failed to get reply from %s",_addr.c_str());
            //clean up response so no partial responses
            return false;
        }
        data_.add(reply);
        --_pending;
    }
    if (_pending <=0 ) _pending = 0;
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

bool cps_db::connection::has_event() {
    void *rep ;
    int rc = REDIS_OK;
    do {
        if ((rc=redisReaderGetReply(static_cast<redisContext*>(_ctx)->reader,&rep))==REDIS_OK) {
            if (rep==nullptr) break;
            if (is_event_message(rep)) _pending_events.push_back(rep);
            else freeReplyObject(rep);
        }
    } while (rc==REDIS_OK);
    return _pending_events.size()>0;
}

#include "std_select_tools.h"

bool cps_db::connection::get_event(response_set &data) {
    do {
        if (_pending_events.size() == 0) {
            int rc = 0;
            if (__added_logging) {
                fd_set _set;
                FD_ZERO(&_set);
                FD_SET(static_cast<redisContext*>(_ctx)->fd,&_set);
                timeval tv = {0,0};
                rc = std_select_ignore_intr(static_cast<redisContext*>(_ctx)->fd+1,&_set,nullptr,nullptr,&tv,nullptr);
                if (rc <=0) {
                    EV_LOG(ERR,DSAPI,0,"CPS-DB-EVT","Called to get event when no data waiting.");
                    break;
                }
            }
            rc = redisBufferRead(static_cast<redisContext*>(_ctx));
            if (rc==REDIS_ERR) break;
            if (!has_event()) break;

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

void __init(void) {
    _cache = new cps_db::connection_cache;
    STD_ASSERT(_cache!=nullptr);
}

cps_db::connection_cache & cps_db::ProcessDBCache() {
    pthread_once(&onceControl,__init);
    return *_cache;
}

cps_db::connection * cps_db::connection_cache::get(const std::string &name) {
    std::lock_guard<std::mutex> l(_mutex);
    auto it = _pool.find(name);
    if (it==_pool.end()) return nullptr;
    auto ptr = it->second.release();
    _pool.erase(it);
    return ptr;
}

void cps_db::connection_cache::put(const std::string &name, connection* conn) {
    std::lock_guard<std::mutex> l(_mutex);
    _pool[name] = std::unique_ptr<cps_db::connection>(conn);
}
void cps_db::connection_cache::remove(const std::string &name) {
    std::lock_guard<std::mutex> l(_mutex);
    _pool.erase(name);
}

cps_db::connection_request::connection_request(cps_db::connection_cache & cache,const char *addr) : _cache(cache) {
    _name = addr;
    _conn = _cache.get(addr);
    if (_conn==nullptr) {
        _conn = new cps_db::connection;
        if (!_conn->connect(addr)) {
            delete _conn;
            _conn  = nullptr;
            return;
        }
    }
}

cps_db::connection_request::~connection_request() {
    if(_conn!=nullptr) {
        _cache.put(_name,_conn);
        _conn = nullptr;
    }
}



