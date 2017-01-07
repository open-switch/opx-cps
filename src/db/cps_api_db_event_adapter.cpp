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



#include "cps_api_node_private.h"
#include "cps_api_db.h"
#include "cps_api_db_response.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "cps_api_node_set.h"

#include "dell-cps.h"

#include "cps_class_map.h"
#include "cps_api_key_cache.h"
#include "cps_string_utils.h"
#include "cps_api_vector_utils.h"
#include "cps_api_object_tools.h"

#include "std_mutex_lock.h"
#include "event_log.h"

#include "cps_api_select_utils.h"
#include "std_time_tools.h"

#include <thread>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <set>


using _key_list = std::vector<std::vector<char>>;

using _key_list_pointer = std::shared_ptr<_key_list>;

using _peer_list = std::unordered_set<std::string>;

using _group_details = std::unordered_map<std::string,std::vector<std::string>>;

using __db_ptr_t = std::shared_ptr<cps_db::connection>;

struct __db_conn_entry_t {
    static const uint64_t                             timeout = MILLI_TO_MICRO(30*1000) ; //xx seconds
    std::string                                     _name;

    using cv_hash = cps_utils::vector_hash<char>;

    std::unordered_set<std::vector<char>,cv_hash>     _keys;
    uint64_t                                         _last_msg;

    std::unordered_map<std::string,bool>             _group_reg;
    std::unique_ptr<cps_db::connection>                _connection;
    ~__db_conn_entry_t() {

    }
    bool connect(const std::string &addr) {
        std::unique_ptr<cps_db::connection> c(new cps_db::connection);
        if (!c->connect(addr)) {
            EV_LOG(TRACE,DSAPI,0,"CPS-EVT-CONN","Failed to connect to the remote node %s",addr.c_str());
            return false;
        }
        _name = addr;
        _connection = std::move(c);
        return true;
    }

    void reset() {
        _group_reg.clear();
        _keys.clear();
        _last_msg = 0;
        _connection.reset();
        _name = "";
    }

    void communicated() {
        _last_msg = std_get_uptime(nullptr);
    }

    bool expired() {
        if (std_time_is_expired(_last_msg,timeout)) {
            return true;
        }
        return false;
    }

    int getfd() {
        return _connection->get_fd();
    }

    bool alive() {
        if (expired()) {
            if (!cps_db::ping(*_connection.get())) {
                return false;
            }
            communicated();
        }
        return true;
    }
    bool synced(const std::string &addr) {
        return _group_reg[addr];
    }
    void set_synced_state(const std::string &addr,bool state) {
        _group_reg[addr] = state;
    }
};

#define _VALID_HANDLE_TOKEN (0xfeedC11f)

struct __db_event_handle_t {
    ssize_t _token = _VALID_HANDLE_TOKEN;
    std::recursive_mutex _mutex;

    size_t _wait_loop_count=0;

    //Each group has a set of keys
    using _group_key_list = std::unordered_map<std::string,_key_list>;

    size_t _last_connection_validation_time = 0;
    _group_key_list _group_keys;

    std::unordered_map<std::string,__db_conn_entry_t> _conn;

    std::unordered_map<int,__db_conn_entry_t*> _fd_to_conn;

    cps_api_key_cache<std::vector<cps_api_object_t>> _filters;

    cps_api_select_guard _select_set = (cps_api_select_alloc_read());

    cps_api_object_list_t _pending_events=nullptr;

    bool object_matches_filter(cps_api_object_t obj) ;
    bool object_add_filter(cps_api_object_t obj) ;
    bool object_filter_exists(cps_api_object_t obj);

    void update_connection_set() ;

    void add_connection_state_event(const std::string &node, const std::string &group, bool state) ;

    void disconnect_node(const std::string &node, bool update_fd_set=true) ;

    bool handle_registrations(const std::string &group, const std::string &node);

    bool &key_sync_flag(const std::string &group, const std::string &node);

    void associated_connection_to_group(std::string &group, std::string &node);

    bool new_connection(const std::string &addr) ;
    __db_conn_entry_t * find(const std::string &addr, bool create_noexist, bool *created) ;

    bool connection_alive(const std::string &addr);

    ~__db_event_handle_t() ;
};


bool & __db_event_handle_t::key_sync_flag(const std::string &group, const std::string &node) {
    return _conn[node]._group_reg[group];
}

bool __db_event_handle_t::connection_alive(const std::string &addr) {
    auto _rc = _conn[addr].alive();
    if (!_rc) {
        disconnect_node(addr);
    }
    return _rc;
}

bool __db_event_handle_t::new_connection(const std::string &addr) {
    auto &_mon = _conn[addr];
    _mon.reset();

    bool _rc = _conn[addr].connect(addr);
    if (!_rc) { _conn.erase(addr); return false; }

    add_connection_state_event(addr.c_str(),"",true);

    return true;
}

__db_conn_entry_t * __db_event_handle_t::find(const std::string &addr, bool create_if_not_exists, bool *created=nullptr) {
    auto &_ref = _conn[addr];
    if (_ref._connection.get()==nullptr) {
        bool _rc = _ref.connect(addr);
        if (!_rc) return nullptr;
        if (created!=nullptr) *created = true;
    }
    return &_ref;
}

__db_event_handle_t::~__db_event_handle_t() {
    auto fn = [](cps_api_key_cache<std::vector<cps_api_object_t>>::cache_data_iterator &it) -> bool {
        for ( auto list : it->second ) {
            for (auto clean_it : list.data ) {
                cps_api_object_delete(clean_it);
            }
        }
        return true;
    };
    _filters.walk(fn);

}

bool __db_event_handle_t::object_matches_filter(cps_api_object_t obj) {
    std::vector<cps_api_object_t> *filt = _filters.at(cps_api_object_key(obj),true);
    if (filt==nullptr) return true;
    for (auto it : *filt ) {
        if (cps_api_obj_tool_matches_filter(it,obj,true)) return true;
    }
    return false;
}

bool __db_event_handle_t::object_add_filter(cps_api_object_t obj) {
    cps_api_object_guard og(cps_api_object_create());
    if (og.get()==nullptr) return false;
    if (!cps_api_object_clone(og.get(),obj)) return false;

    cps_api_object_attr_delete(og.get(),CPS_OBJECT_GROUP_EXACT_MATCH);

    cps_api_key_t *key = cps_api_object_key(og.get());
    std::vector<cps_api_object_t> *filt = _filters.at(key,true);
    if (filt==nullptr) {
        std::vector<cps_api_object_t> lst;
        lst.push_back(og.get());
        _filters.insert(key,std::move(lst));
    } else {
        filt->push_back(og.get());
    }
    og.release();
    return true;
}

void __db_event_handle_t::disconnect_node(const std::string &node, bool update_fd_set) {
    _conn.erase(node);
    if (update_fd_set) update_connection_set();
    add_connection_state_event(node.c_str(),node.c_str(),false);
}

void __db_event_handle_t::add_connection_state_event(const std::string &node, const std::string &group, bool state) {
    cps_api_object_t o = cps_api_object_list_create_obj_and_append(_pending_events);
    if (o!=nullptr) {
        std::string _real_name;
        if(cps_api_db_get_node_from_ip(node,_real_name)){
            cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME, _real_name.c_str(),_real_name.size()+1);
        }
        cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY, cps_api_qualifier_TARGET);
        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP, node.c_str(),node.size()+1);

        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_GROUP, group.c_str(),group.size()+1);
        cps_api_object_attr_add_u32(o,CPS_CONNECTION_ENTRY_CONNECTION_STATE, state);
    }
}

void __db_event_handle_t::update_connection_set() {
    _select_set.remove_all_fds();
    _fd_to_conn.clear();
    for (auto &it : _conn ) {
        auto _fd = it.second.getfd();
        _select_set.add_fd(_fd);
        _fd_to_conn[_fd] = &it.second;
    }
}

bool __db_event_handle_t::handle_registrations(const std::string &group, const std::string &node) {
    auto con_ptr = find(node,false);
    if (con_ptr==nullptr) return false;
    if (con_ptr->_connection.get()==nullptr) {
        disconnect_node(node);
        return false;
    }
    if (con_ptr->synced(group)) {
        return false;
    }

    for (auto &_key : _group_keys[group]) {
        if (con_ptr->_keys.find(_key)!= con_ptr->_keys.end()) {
            continue;
        }
        if (!cps_db::subscribe(*con_ptr->_connection.get(),_key)) {
            disconnect_node(node.c_str());
            return false;
        }
        con_ptr->_keys.insert(_key);
    }
    con_ptr->set_synced_state(group,true);
    return true;
}

inline __db_event_handle_t* handle_to_data(cps_api_event_service_handle_t handle) {
    return (__db_event_handle_t*) handle;
}

static void __resync_regs(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nd = handle_to_data(handle);

    for (auto it : nd->_group_keys) {
        cps_api_node_set_iterate(it.first,
                [nd,&it,&handle](const std::string &node) ->bool {
            //occurs one for each node in the group
            nd->handle_registrations(it.first,node);
            return true;
        });
    }
}

static bool __check_connections(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nd = handle_to_data(handle);
    std::set<std::string> nodes;

    for ( auto &it : nd->_conn) {        //generate a list of connections
        nodes.insert(it.first);
    }

    bool _changed = false;

    for (auto it : nd->_group_keys) {
        cps_api_node_set_iterate(it.first,
                [&](const std::string &node) -> bool {
                bool _created = false;
                auto con_ptr = nd->find(node,true,&_created);
                if (con_ptr==nullptr) return true;
                if (_created) _changed = true;
                if (!con_ptr->alive()) {
                    nd->disconnect_node(node);
                    return false;
                }
                //processed/viewed node
                nodes.erase(node);
                return true;
            });
    }

    if (_changed) {
        nd->update_connection_set();
    }

    for ( auto & it : nodes ) {
        nd->disconnect_node(it);
    }

    return _changed;
}

static void __maintain_connections(__db_event_handle_t *nh) {
    __check_connections(nh);
    __resync_regs(nh);
}

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {
    std::unique_ptr<__db_event_handle_t> _h(new __db_event_handle_t);
    _h->_pending_events = cps_api_object_list_create();
    if (_h->_pending_events==nullptr) return cps_api_ret_code_ERR;
    *handle = _h.release();
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _register_one_object(cps_api_event_service_handle_t handle,
        cps_api_object_t object) {

    __db_event_handle_t *nh = handle_to_data(handle);
    std::lock_guard<std::recursive_mutex> lg(nh->_mutex);

    const char *_group = cps_api_key_get_group(object);

    std::vector<char> _key;
    cps_db::dbkey_from_instance_key(_key,object,true);
    if (_key.size()==0) {
        static const int CPS_OBJ_STR_LEN = 1000;
        char buff[CPS_OBJ_STR_LEN];
        EV_LOG(ERR,DSAPI,0,"CPS-EVNT-REG","Invalid object being registered for %s",cps_api_object_to_string(object,buff,sizeof(buff)-1));
        return cps_api_ret_code_ERR;
    }

    try {
        nh->_group_keys[_group].push_back(std::move(_key));    //add this key to the group
    } catch(std::exception &e) {
        return cps_api_ret_code_ERR;
    }

    //for each node in the group say need to be audited
    cps_api_node_set_iterate(_group,[&nh,_group](const std::string &node) {
        nh->_conn[node].set_synced_state(_group,false);
        return true;
    });

    bool *_is_filt_obj = (bool*)cps_api_object_get_data(object,CPS_OBJECT_GROUP_EXACT_MATCH);
    if (_is_filt_obj!=nullptr && *_is_filt_obj) {
        nh->object_add_filter(object);
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_register_objs_function_(cps_api_event_service_handle_t handle,
        cps_api_object_list_t objects) {

    __db_event_handle_t *nh = handle_to_data(handle);
    std::lock_guard<std::recursive_mutex> lg(nh->_mutex);

    for ( size_t ix = 0, mx = cps_api_object_list_size(objects); ix < mx ; ++ix ) {
        cps_api_return_code_t rc = cps_api_ret_code_OK;
        cps_api_object_t o = cps_api_object_list_get(objects,ix);
        rc = _register_one_object(handle,o);
        if (rc!=cps_api_ret_code_OK) return rc;
    }
    __maintain_connections(handle_to_data(handle));

    return cps_api_ret_code_OK;
}


static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    STD_ASSERT(msg!=NULL);
    STD_ASSERT(handle!=NULL);

    if (cps_api_key_get_len(cps_api_object_key(msg))==0) return cps_api_ret_code_ERR;

    const char *_group = cps_api_key_get_group(msg);

    bool sent = true;
    cps_api_node_set_iterate(_group,[&msg,&sent](const std::string &name)->bool{
        cps_db::connection_request r(cps_db::ProcessDBEvents(),name.c_str());
        sent = sent && r.valid() && cps_db::publish(r.get(),msg);
        return true;
    });

    return sent? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

static cps_api_return_code_t _cps_api_event_service_client_deregister(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nh = handle_to_data(handle);

    //no point in locking the handle on close..
    //clients will be messed up anyway if they try to have multiple threads
    //using and destroying event channels
    cps_api_object_list_destroy(nh->_pending_events,true);

    delete nh;

    return cps_api_ret_code_OK;
}

static bool get_event(cps_db::connection *conn, cps_api_object_t obj) {
    cps_db::response_set set;
    if (!conn->get_event(set)) {
        return false;
    }
    cps_db::response r = set.get_response(0);
    cps_db::response type (r.element_at(0));

    if (strcasecmp(type.get_str(),"pmessage")==0) {
        cps_db::response data(r.element_at(3));
        if (cps_api_array_to_object(data.get_str(),data.get_str_len(),obj)) {
            return true;
        }
    }
    return false;
}

static cps_api_return_code_t _cps_api_wait_for_event(
        cps_api_event_service_handle_t handle,
        cps_api_object_t msg,
        ssize_t timeout_ms) {

    /* 10 seconds is too long for timeout, if there are no events in that case group update takes
     * longer time so reducing it to 3
     */
    const static int DEF_SELECT_TIMEOUT_SEC = (3);
    __db_event_handle_t *nh = handle_to_data(handle);

    uint64_t __started_time = std_get_uptime(nullptr);
    size_t _max_wait_time = 0;
    bool _waiting_for_event = true;

    auto _fetch_event = [&](const std::string &node,cps_db::connection *conn) -> bool {
        if (get_event(conn,msg)) {
            nh->_conn[node].communicated();
            if (!nh->object_matches_filter(msg)) return false;        //throw out if doesn't match
            std::string node_name;
            if(cps_api_db_get_node_from_ip(conn->addr(),node_name)) {
               cps_api_object_attr_add(msg,CPS_OBJECT_GROUP_NODE,node_name.c_str(),node_name.size()+1);
            }
            return true;
        }
        return false;
    };

    cps_api_return_code_t __rc = cps_api_ret_code_TIMEOUT;
    while (_waiting_for_event) {
        ++nh->_wait_loop_count;
        std::lock_guard<std::recursive_mutex> lock (nh->_mutex);
        if (std_time_is_expired(nh->_last_connection_validation_time,MILLI_TO_MICRO(1000*5))) {    //wait for 3 seconds before scanning again
            nh->_last_connection_validation_time = std_get_uptime(nullptr);
            __maintain_connections(nh);
        }

        //allow insertion of node loss messages
        if (cps_api_object_list_size(nh->_pending_events)>0) {
            cps_api_object_guard og(cps_api_object_list_get(nh->_pending_events,0));
            cps_api_object_list_remove(nh->_pending_events,0);
            cps_api_object_swap(msg,og.get());
            return cps_api_ret_code_OK;
        }

        if (timeout_ms==CPS_API_TIMEDWAIT_NO_TIMEOUT) {
            _max_wait_time = DEF_SELECT_TIMEOUT_SEC*1000;
        } else {
            _max_wait_time = timeout_ms - ((std_get_uptime(nullptr) - __started_time)/1000);
             if (std_time_is_expired(__started_time,MILLI_TO_MICRO(timeout_ms))) {
                 _waiting_for_event = false;
                 _max_wait_time = 0;
             }
        }

        if (nh->_conn.size()==0) {
            const static size_t RETRY_TIME_US = 1000*1000;
            std_usleep(RETRY_TIME_US);
            continue;
        }

        for (auto &it : nh->_conn) {
            if (it.second._connection->has_event()) {
                nh->_mutex.unlock();
                bool _rc =_fetch_event(it.first,it.second._connection.get());
                nh->_mutex.lock();
                if (_rc) return cps_api_ret_code_OK;
            }
        }

        int _handle = -1;
        nh->_mutex.unlock();
        size_t _count = nh->_select_set.get_event(_max_wait_time,&_handle);
        nh->_mutex.lock();

        if ( _count<0) {
            continue;
        }

        auto it = nh->_fd_to_conn.find(_handle);
        if (it==nh->_fd_to_conn.end()) {
            nh->update_connection_set();
            continue;
        }

        nh->_mutex.unlock();
        bool _rc = _fetch_event(it->second->_name,it->second->_connection.get());
        nh->_mutex.lock();
        if (_rc) return cps_api_ret_code_OK;
    }
    return __rc;
}

extern "C" {

static cps_api_event_methods_reg_t functions = {
        NULL,
        0,
        _cps_api_event_service_client_connect,
        nullptr,
        _cps_api_event_service_publish_msg,
        _cps_api_event_service_client_deregister,
        _cps_api_wait_for_event,
        _cps_api_event_service_register_objs_function_
};

cps_api_return_code_t cps_api_event_service_init(void) {
    cps_api_event_method_register(&functions);
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_services_start() {
    return cps_api_ret_code_OK;
}

}
