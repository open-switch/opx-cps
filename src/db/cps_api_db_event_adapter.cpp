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

/*
 * filename: hal_event_service.c
 */


#include "cps_api_node_private.h"
#include "cps_api_db.h"
#include "cps_api_db_response.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "cps_api_node_set.h"

#include "dell-cps.h"
#include "cps_class_map.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"
#include "private/cps_ns.h"
#include "cps_api_service.h"
#include "cps_string_utils.h"
#include "cps_api_vector_utils.h"

#include "std_mutex_lock.h"
#include "event_log.h"

#include "std_time_tools.h"
#include "std_select_tools.h"

#include <thread>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <memory>

struct db_connection_details {
    static const uint64_t timeout = 60*1000*1000 ; //60 seconds

    using cv_hash = cps_utils::vector_hash<char>;
    std::unordered_set<std::vector<char>,cv_hash> _keys;

    std::unordered_map<std::string,bool> _group_reg;

    uint64_t _last_msg;

    void reset() {
        _group_reg.clear();
        _keys.clear();
        _last_msg = 0;
    }
    void communicated() {
        _last_msg = std_get_uptime(nullptr);
    }
    bool expired(bool update =true) {
        if (std_time_is_expired(_last_msg,timeout)) {
            if (update) communicated();
            return true;
        }
        return false;
    }
};

struct __db_event_handle_t {
    std::recursive_mutex _mutex;

    std::unordered_map<std::string,std::vector<std::vector<char>>> _group_keys;

    std::unordered_map<std::string,std::unique_ptr<cps_db::connection>> _connections;
    std::unordered_map<std::string,db_connection_details> _connection_mon;

    fd_set _connection_set;
    ssize_t _max_fd;

    cps_api_object_list_t _pending_events=nullptr;

    void update_connection_set() ;
    void add_connection_state_event(const char *node, const char *group, bool state) ;

    void disconnect_node(const std::string &node, bool update_fd_set=true) ;
};

void __db_event_handle_t::disconnect_node(const std::string &node, bool update_fd_set) {
    _connections.erase(node);
    _connection_mon.erase(node);
    if (update_fd_set) update_connection_set();
    add_connection_state_event(node.c_str(),node.c_str(),false);
}

void __db_event_handle_t::add_connection_state_event(const char *node, const char *group, bool state) {
    cps_api_object_t o = cps_api_object_list_create_obj_and_append(_pending_events);
    if (o!=nullptr) {
        cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY, cps_api_qualifier_TARGET);
        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP, node,strlen(node)+1);
        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME, node,strlen(node)+1);
        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_GROUP, group,strlen(group)+1);
        cps_api_object_attr_add_u32(o,CPS_CONNECTION_ENTRY_CONNECTION_STATE, state);
    }
}

void __db_event_handle_t::update_connection_set() {
    _max_fd = -1;
    FD_ZERO(&_connection_set);
    for (auto &it : _connections) {
        FD_SET(it.second->get_fd(), &_connection_set);
        if (_max_fd< it.second->get_fd())  _max_fd = it.second->get_fd();
    }
}

inline __db_event_handle_t* handle_to_data(cps_api_event_service_handle_t handle) {
    return (__db_event_handle_t*) handle;
}

static void __resync_regs(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nd = handle_to_data(handle);

    bool _connections_changed = false;

    for (auto it : nd->_group_keys) {

        cps_api_node_set_iterate(it.first,[nd,&it,&handle,&_connections_changed](const std::string &node,void *context) {
            auto con_it = nd->_connections.find(node);
            if (con_it==nd->_connections.end()) {
                return;
            }
            if (nd->_connection_mon[node]._group_reg[it.first]==true) return ;

            //for each client key associated with the group
            for ( auto reg_it : it.second) {
                if (nd->_connection_mon[node]._keys.find(reg_it)!=nd->_connection_mon[node]._keys.end()) {
                    continue;
                }
                if (!cps_db::subscribe(*con_it->second,reg_it)) {
                    nd->disconnect_node(node.c_str());
                    _connections_changed = true;
                    return;
                }
                nd->_connection_mon[node]._keys.insert(reg_it);
                nd->_connection_mon[node].communicated();
            }
            nd->_connection_mon[node]._group_reg[it.first] = true;

        },handle);
    }
    if (_connections_changed) nd->update_connection_set();
}

static bool __check_connections(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nd = handle_to_data(handle);

    bool changed = false;

    for (auto it : nd->_group_keys) {
        cps_api_node_set_iterate(it.first,[&nd,&changed,it,&handle](const std::string &node,void *context) {
                auto con_it = nd->_connections.find(node);
                if (con_it==nd->_connections.end()) {
                    std::unique_ptr<cps_db::connection> c(new cps_db::connection);
                    if (!c->connect(node)) {
                        EV_LOG(TRACE,DSAPI,0,"CPS-EVT-CONN","Failed to connect to the remote node %s",node.c_str());
                        return;
                    }
                    if (!cps_db::ping(*c)) return;
                    nd->_connection_mon[node].reset();
                    nd->_connection_mon[node].communicated();
                    nd->_connections[node] = std::move(c);
                    nd->add_connection_state_event(node.c_str(),it.first.c_str(),true);

                    changed = true;
                }
                if (nd->_connection_mon[node].expired()) {
                    if (!cps_db::ping(*nd->_connections[node])) {
                        nd->disconnect_node(node.c_str());
                        return;
                    }
                }
                if (nd->_connection_mon[node]._group_reg.find(it.first)==nd->_connection_mon[node]._group_reg.end()) {
                    nd->_connection_mon[node]._group_reg[it.first] = false;
                }
            },handle);

    }
    if (changed) {
        nd->update_connection_set();
    }
    return changed;
}

static bool __maintain_connections(__db_event_handle_t *nh) {
    bool new_conn = __check_connections(nh);
    __resync_regs(nh);
    return new_conn;
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
    cps_db::dbkey_from_instance_key(_key,object);
    if (_key.size()==0) {
        static const int CPS_OBJ_STR_LEN = 1000;
        char buff[CPS_OBJ_STR_LEN];
        EV_LOG(ERR,DSAPI,0,"CPS-EVNT-REG","Invalid object being registered for %s",cps_api_object_to_string(object,buff,sizeof(buff)-1));
        return cps_api_ret_code_ERR;
    }
    nh->_group_keys[_group].push_back(std::move(_key));

    cps_api_node_set_iterate(_group,[&nh,_group](const std::string &node,void *context) {
        nh->_connection_mon[node]._group_reg[_group] = false;
    },nullptr);

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_register_objs_function_(cps_api_event_service_handle_t handle,
        cps_api_object_list_t objects) {

    for ( size_t ix = 0, mx = cps_api_object_list_size(objects); ix < mx ; ++ix ) {
        cps_api_return_code_t rc = cps_api_ret_code_OK;
        cps_api_object_t o = cps_api_object_list_get(objects,ix);
        rc = _register_one_object(handle,o);
        if (rc!=cps_api_ret_code_OK) return rc;
    }

    std::lock_guard<std::recursive_mutex> lg(handle_to_data(handle)->_mutex);
    __maintain_connections(handle_to_data(handle));

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    STD_ASSERT(msg!=NULL);
    STD_ASSERT(handle!=NULL);

    cps_api_key_t *_okey = cps_api_object_key(msg);

    if (cps_api_key_get_len(_okey) < CPS_OBJ_KEY_SUBCAT_POS) {
        return cps_api_ret_code_ERR;
    }

    const char *_group = cps_api_key_get_group(msg);

    bool sent = true;
    cps_api_node_set_iterate(_group,[&msg,&sent](const std::string &name,void *context){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        sent &= cps_db::publish(r.get(),msg);
    },nullptr);

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
        cps_api_object_t msg) {

    __db_event_handle_t *nh = handle_to_data(handle);

    uint64_t last_checked = 0;

    int fd_max = -1;
    fd_set _r_set ;
    ssize_t rc = 0;

    while (true) {

        //allow insertion of node loss messages
        std::lock_guard<std::recursive_mutex> lock (nh->_mutex);
        if (cps_api_object_list_size(nh->_pending_events)>0) {
            cps_api_object_guard og(cps_api_object_list_get(nh->_pending_events,0));

            cps_api_object_list_remove(nh->_pending_events,0);
            cps_api_object_clone(msg,og.get());
            return cps_api_ret_code_OK;
        }
        if (std_time_is_expired(last_checked,1000)) {
            last_checked = std_get_uptime(nullptr);

            __maintain_connections(nh);
        }

        if (nh->_max_fd==-1) {
            const static size_t RETRY_TIME_US = 1000*500;
            std_usleep(RETRY_TIME_US);
            continue;
        }


        bool has_event = false;
        for (auto &it : nh->_connections) {
            if (it.second->has_event()) {
                has_event = true;
            }
        }

        if (!has_event) {
            _r_set = nh->_connection_set;
            fd_max = nh->_max_fd+1;
            timeval tv={0,0};
            nh->_mutex.unlock();
            rc = std_select_ignore_intr(fd_max,&_r_set,nullptr,nullptr,&tv,nullptr);
            nh->_mutex.lock();
            if (rc==-1) {
                last_checked = 0;
                continue;
            }
            if (rc==0) continue;
        }

        for (auto &it : nh->_connections) {
            if (it.second->get_fd() > nh->_max_fd) {
                EV_LOG(ERR,DSAPI,0,"CPS-EVT-WAIT","Invalid Max FD value %d vs current fd %d",nh->_max_fd,it.second->get_fd());
                continue;
            }
            bool has_data = !has_event ? FD_ISSET(it.second->get_fd(),&_r_set) : 0;

            if (has_data || it.second->has_event()) {
                if (get_event(it.second.get(),msg)) {
                    cps_api_object_attr_add(msg,CPS_OBJECT_GROUP_NODE,it.first.c_str(),it.first.size()+1);
                    return cps_api_ret_code_OK;
                }
            }
        }
    }
    return cps_api_ret_code_OK;

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
