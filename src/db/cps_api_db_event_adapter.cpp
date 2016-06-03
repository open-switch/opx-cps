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

#include "cps_api_operation.h"
#include "cps_api_service.h"
#include "private/cps_ns.h"
#include "cps_api_service.h"
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

struct _reg_data {
    bool _sync = false;
    std::vector<char> _key;
};

struct _handle_data {
    std::mutex _mutex;

    cps_db::connection_cache _sub;
    cps_db::connection_cache _pub;
    std::unordered_map<std::string,std::vector<_reg_data>> _group_keys;

    std::unordered_map<std::string,std::unique_ptr<cps_db::connection>> _connections;

    std::unordered_map<std::string,bool> _group_updated;
};


inline _handle_data* handle_to_data(cps_api_event_service_handle_t handle) {
    return (_handle_data*) handle;
}

static std::mutex _mutex;
static std::unordered_set<_handle_data *> _handles;


static void __resync_regs(cps_api_event_service_handle_t handle) {
    _handle_data *nd = handle_to_data(handle);

    for (auto it : nd->_group_keys) {

        bool success = true;
        bool group_updated = nd->_group_updated[it.first];

        if (!cps_api_node_set_iterate(it.first,[nd,&it,&success,&group_updated](const std::string &node,void *context) {
            auto con_it = nd->_connections.find(node);
            if (con_it==nd->_connections.end()) {
                return;
            }

            //for each client key associated with the group
            for ( auto reg_it : it.second) {
                if (!reg_it._sync || group_updated) { //best we can do for now but come back and fix this condition
                    if (!cps_db::subscribe(*con_it->second,reg_it._key)) {
                        nd->_connections.erase(con_it);
                        break;
                    }
                }
            }

        },handle)) {
            success=false;
        }

        //nodes in group totally subscribed for
        if (success) {
            nd->_group_updated[it.first] = false;
            for ( auto reg_it : it.second) {
                reg_it._sync = true;
            }
        }
    }
}

static bool __check_connections(cps_api_event_service_handle_t handle) {
    _handle_data *nd = handle_to_data(handle);

    bool changed = false;

    for (auto it : nd->_group_keys) {
        if (cps_api_node_set_iterate(it.first,[&nd,&changed](const std::string &node,void *context) {
                auto con_it = nd->_connections.find(node);
                if (con_it==nd->_connections.end()) {
                    std::unique_ptr<cps_db::connection> c(new cps_db::connection);
                    if (!c->connect(node)) {
                        return;
                    }
                    nd->_connections[node] = std::move(c);
                    nd->_group_updated[node] = true;
                    changed = true;
                }
            },handle)) {

        }
    }

    return changed;
}

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {
    std::unique_ptr<_handle_data> _h(new _handle_data);
    *handle = _h.release();
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _register_one_object(cps_api_event_service_handle_t handle,
        cps_api_object_t object) {

    _handle_data *nh = handle_to_data(handle);
    std::lock_guard<std::mutex> lg(nh->_mutex);

    const char *_group = cps_api_key_get_group(object);

    _reg_data rd ;
    rd._sync = false;
    cps_db::dbkey_from_instance_key(rd._key,object);

    nh->_group_keys[_group].push_back(rd);
    nh->_group_updated[_group] = true;

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

    if (__check_connections(handle)) __resync_regs(handle);

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    _handle_data *nh = handle_to_data(handle);
    std::lock_guard<std::mutex> lg(nh->_mutex);

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
    _handle_data *nh = handle_to_data(handle);

    //no point in locking the handle on close..
    //clients will be messed up anyway if they try to have multiple threads
    //using and destroying event channels

    delete nh;

    return cps_api_ret_code_OK;
}

static ssize_t __setup_fd_set(_handle_data *nh, fd_set &set) {
    std::lock_guard<std::mutex> lg(nh->_mutex);
    ssize_t mx = -1;
    FD_ZERO(&set);
    for (auto &it : nh->_connections) {
        FD_SET(it.second->get_fd(), &set);
        if (mx < it.second->get_fd())  mx = it.second->get_fd();
    }
    return mx;
}

static cps_api_return_code_t _cps_api_wait_for_event(
        cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    _handle_data *nh = handle_to_data(handle);

    fd_set _template_set;
    ssize_t max_fd = __setup_fd_set(nh,_template_set);

    while (true) {

        ssize_t rc = 0;

        std::function<bool()> check_connections = [max_fd,&_template_set]() ->bool {
            struct timeval tv; tv.tv_sec=1; tv.tv_usec =0;
            fd_set _r_set = _template_set;
            return std_select_ignore_intr(max_fd+1,&_r_set,nullptr,nullptr,&tv,nullptr);
        };

        if (max_fd==-1 || (rc=check_connections())==-1) {
            std::lock_guard<std::mutex> lg(nh->_mutex);
            if (__check_connections(handle)) {
                __resync_regs(handle);
            }
            max_fd = __setup_fd_set(nh,_template_set);
            continue;
        }

        if (rc==0) continue;

        std::lock_guard<std::mutex> lg(nh->_mutex);
        for (auto &it : nh->_connections) {
            if (FD_ISSET(it.second->get_fd(),&_template_set)) {
                cps_db::response_set set;
                if (!it.second->get_event(set.get())) {
                    nh->_connections.erase(it.first);
                    break;
                }

                cps_db::response r = set.get_response(0);
                cps_db::response type (r.element_at(0));
                if (strcasecmp(type.get_str(),"pmessage")==0) {
                    cps_db::response data(r.element_at(3));
                    if (cps_api_array_to_object(data.get_str(),data.get_str_len(),msg)) {
                        return cps_api_ret_code_OK;
                    }
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
