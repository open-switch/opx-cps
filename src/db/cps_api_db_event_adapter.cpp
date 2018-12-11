/*
 * Copyright (c) 2018 Dell Inc.
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

#include "cps_api_db_connection_tools.h"
#include "cps_class_map.h"
#include "cps_api_key_cache.h"
#include "cps_string_utils.h"
#include "cps_api_vector_utils.h"
#include "cps_api_object_tools.h"

#include "std_mutex_lock.h"
#include "event_log.h"

#include "std_time_tools.h"
#include "std_select_tools.h"

#include <thread>
#include <stdlib.h>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <set>

static cps_api_key_t __grp_key;
static pthread_once_t __one_time_only;//need to statically init

struct db_connection_details {
    std::string _name;
    static const uint64_t timeout = 60*1000*1000 ; //60 seconds

    using cv_hash = cps_utils::vector_hash<char>;
    std::unordered_set<std::vector<char>,cv_hash> _keys;

    std::unordered_map<std::string,bool> _group_reg;

    void reset() {
        _group_reg.clear();
        _keys.clear();
    }
};

struct __db_event_handle_t {
    std::recursive_mutex _mutex;

    std::unordered_map<std::vector<char>,std::string,cps_utils::vector_hash<char>> _key_translation;

    std::unordered_map<std::string,std::vector<std::vector<char>>> _group_keys;

    std::unordered_map<std::string,std::unique_ptr<cps_db::connection>> _connections;
    std::unordered_map<std::string,db_connection_details> _connection_mon;

    std::unordered_map<size_t,size_t> _sequence_tracker;

    cps_api_key_cache<std::vector<cps_api_object_t>> _filters;

    fd_set _connection_set;
    ssize_t _max_fd=0;
    size_t _last_checked = 0;

    cps_api_object_list_t _pending_events=nullptr;

    __db_event_handle_t() {
        FD_ZERO(&_connection_set);
    }

    bool object_matches_filter(cps_api_object_t obj) ;
    bool object_add_filter(cps_api_object_t obj) ;
    bool object_filter_exists(cps_api_object_t obj);

    void update_connection_set() ;

    void add_connection_state_event(const char *node, const char *group, bool state) ;

    void disconnect_node(std::string node, bool update_fd_set=true) ;

    ~__db_event_handle_t() ;
};

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

void __db_event_handle_t::disconnect_node(std::string node, bool update_fd_set) {
    add_connection_state_event(node.c_str(),"N/A",false);
    _connections.erase(node);
    _connection_mon.erase(node);
    if (update_fd_set) update_connection_set();
}

void __db_event_handle_t::add_connection_state_event(const char *node, const char *group, bool state) {
    cps_api_object_t o = cps_api_object_list_create_obj_and_append(_pending_events);
    if (o!=nullptr) {

        EV_LOGGING(CPS-DB-EV-CONN,DEBUG,state? "CONN": "DISCON","Connection event occured for %s", node!=nullptr?node:"");

        auto _it = _connection_mon.find(node);
        if (_it!=_connection_mon.end()) {
            cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME, _it->second._name.c_str(),_it->second._name.length()+1);
        }
        cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY, cps_api_qualifier_TARGET);
        cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP, node,strlen(node)+1);

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
        cps_api_node_set_iterate(it.first,
                [nd,&it,&handle,&_connections_changed](const std::string &node) ->bool {
            //occurs one for each node in the group
            auto con_it = nd->_connections.find(node);
            if (con_it==nd->_connections.end()) {    //if the connection is not so good
                return true;
            }

            if (nd->_connection_mon[node]._group_reg[it.first]==true) return true;

            //for each client key associated with the group
            for ( auto reg_it : it.second) {
                if (nd->_connection_mon[node]._keys.find(reg_it)!=nd->_connection_mon[node]._keys.end()) {
                    continue;
                }
                if (!cps_db::subscribe(*con_it->second,reg_it)) {
                    EV_LOGGING(CPS-DB-EV-CONN,ERR,"REG","Not able to subscript for event from node %s",node.c_str());
                    nd->disconnect_node(node.c_str());
                    _connections_changed = true;
                    return true;
                }
                nd->_connection_mon[node]._keys.insert(reg_it);

                //
                auto _k_it = nd->_key_translation.find(reg_it);
                if (_k_it!=nd->_key_translation.end()) {
                    EV_LOGGING(CPS-DB-CONN,INFO,"EVT-REG","Syncing registration for %s to the backend %s",
                            _k_it->second.c_str(),node.c_str());
                }

            }
            nd->_connection_mon[node]._group_reg[it.first] = true;
            return true;
        });
    }
    if (_connections_changed) nd->update_connection_set();
}

static bool __check_connections(cps_api_event_service_handle_t handle) {
    __db_event_handle_t *nd = handle_to_data(handle);

    bool changed = false;

    std::set<std::string> nodes;

    for ( auto &it : nd->_connections ) {
        nodes.insert(it.first);
    }

    for (auto it : nd->_group_keys) {
        cps_api_node_set_iterate(it.first,
                [&nd,&changed,it,&handle,&nodes](const std::string &node) -> bool {
                auto con_it = nd->_connections.find(node);

                if (con_it==nd->_connections.end()) {
                    std::unique_ptr<cps_db::connection> c(cps_db::cps_api_db_create_validated_connection(node.c_str()));
                    if (c.get()==nullptr) return true;    //if invalid connection nothing else to do

                    std::string node_name;
                    if(cps_api_db_get_node_from_ip(std::string(node),node_name)){
                        nd->_connection_mon[node]._name = node_name;
                    }
                    nd->_connection_mon[node].reset();
                    nd->_connections[node] = std::move(c);
                    nd->add_connection_state_event(node.c_str(),it.first.c_str(),true);

                    changed = true;
                }
                if (nd->_connections[node]->used_within(db_connection_details::timeout)) {
                    if (!cps_api_db_validate_connection(nd->_connections[node].get())) {
                        EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","Connection not validated %s",node.c_str());
                        nd->disconnect_node(node.c_str());
                        return true;
                    }
                }
                if (nd->_connection_mon[node]._group_reg.find(it.first)==nd->_connection_mon[node]._group_reg.end()) {
                    nd->_connection_mon[node]._group_reg[it.first] = false;
                }
                nodes.erase(node);
                return true;
            });
    }

    changed = changed || (nodes.size()>0);

    for ( auto & it : nodes ) {
        EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","removing node from connection set %s",it.c_str());
        nd->disconnect_node(it.c_str());
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
        nh->_key_translation[_key] = cps_api_object_to_c_string(object);
        nh->_group_keys[_group].push_back(std::move(_key));    //add this key to the group
    } catch(std::exception &e) {
        return cps_api_ret_code_ERR;
    }

    //for each node in the group say need to be audited
    cps_api_node_set_iterate(_group,[&nh,_group](const std::string &node) {
        nh->_connection_mon[node]._group_reg[_group] = false;
        return true;
    });

    bool *_is_filt_obj = (bool*)cps_api_object_get_data(object,CPS_OBJECT_GROUP_EXACT_MATCH);
    if (_is_filt_obj!=nullptr && *_is_filt_obj) {
        nh->object_add_filter(object);
    }

    return cps_api_ret_code_OK;
}

static void __init_event_data(void) {
    cps_api_key_from_attr_with_qual(&__grp_key,CPS_CONNECTIVITY_GROUP,cps_api_qualifier_OBSERVED);
}

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {
    std::unique_ptr<__db_event_handle_t> _h(new __db_event_handle_t);
    _h->_pending_events = cps_api_object_list_create();
    if (_h->_pending_events==nullptr) return cps_api_ret_code_ERR;
    *handle = _h.release();
    
    //add pthread once
	pthread_once(&__one_time_only,__init_event_data);

    // Register for connectivity group events
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_copy(cps_api_object_key(og.get()),&__grp_key);
    //log error
    if(_register_one_object(*handle,og.get()) != cps_api_ret_code_OK) EV_LOG(ERR,DSAPI,0,"CPS-EVNT-SERVICE","Connectivity group object subscription failed");
     __maintain_connections(handle_to_data(*handle));

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
    cps_api_node_set_iterate(_group,[&msg,&sent](const std::string &name)->bool{
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
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

static bool get_event(cps_db::connection *conn, cps_api_object_t obj, bool &has_error) {
    cps_db::response_set set;

    if (!conn->get_event(set,has_error)) {
        if(has_error) {
            //error logged elsewhere
            EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","Error on get_event");
        }
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

    int fd_max = -1;
    fd_set _r_set ;
    ssize_t rc = 0;

    uint64_t __started_time = std_get_uptime(nullptr);
    size_t _max_wait_time = 0;
    bool _waiting_for_event = true;

    cps_api_return_code_t __rc = cps_api_ret_code_TIMEOUT;
    bool _has_error=false;
    while (_waiting_for_event) {
        _has_error=false;
        if (timeout_ms==CPS_API_TIMEDWAIT_NO_TIMEOUT) {
            _max_wait_time = DEF_SELECT_TIMEOUT_SEC*1000;
        } else {
            _max_wait_time = timeout_ms - ((std_get_uptime(nullptr) - __started_time)/1000);
             if (std_time_is_expired(__started_time,MILLI_TO_MICRO(timeout_ms))) {
                 _waiting_for_event = false;
                 _max_wait_time = 0;
             }
        }

        //allow insertion of node loss messages
        std::lock_guard<std::recursive_mutex> lock (nh->_mutex);
        if (cps_api_object_list_size(nh->_pending_events)>0) {
            cps_api_object_guard og(cps_api_object_list_get(nh->_pending_events,0));

            cps_api_object_list_remove(nh->_pending_events,0);
            cps_api_object_clone(msg,og.get());
            return cps_api_ret_code_OK;
        }
        if (std_time_is_expired(nh->_last_checked,MILLI_TO_MICRO(1000*3))) {    //wait for 3 seconds before scanning again
            nh->_last_checked = std_get_uptime(nullptr);
            __maintain_connections(nh);
        }

        if (nh->_max_fd==-1) {
            const static size_t RETRY_TIME_US = 1000*1000;
            std_usleep(RETRY_TIME_US);
            continue;
        }

        bool pending_event = false;
        for (auto &it : nh->_connections) {
            if (it.second->has_event(_has_error)) {
                pending_event = true;
            }
            if (_has_error) {
                //error logged elsewhere
                EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","Error on has_event");
                nh->disconnect_node(it.first,true);
                break;
            }
        }

        if (_has_error) continue;

        if (!pending_event) {
            _r_set = nh->_connection_set;
            fd_max = nh->_max_fd+1;
            timeval tv={ static_cast<long int>(_max_wait_time/1000), static_cast<long int>((_max_wait_time%1000)*1000) };
            nh->_mutex.unlock();
            rc = std_select_ignore_intr(fd_max,&_r_set,nullptr,nullptr,&tv,nullptr);
            nh->_mutex.lock();
            if (rc==-1) {
                //test all connections
                auto _it = nh->_connections.begin();
                auto _end = nh->_connections.end();
                for ( ; _it != _end ; ++_it) {
                    _it->second->reset_last_used();
                }
                nh->_last_checked = 0;    //trigger reconnect evaluation
                continue;
            }
            if (rc==0) continue;
        }

        for (auto &it : nh->_connections) {
            if (it.second->get_fd() > nh->_max_fd) {
                EV_LOG(ERR,DSAPI,0,"CPS-EVT-WAIT","Invalid Max FD value %d vs current fd %d",nh->_max_fd,it.second->get_fd());
                continue;
            }
            bool has_data = it.second->has_event(_has_error);

            if (_has_error) {
                //error logged elsewhere
                EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","Error on has_event");
                nh->disconnect_node(it.first,true);
                break;
            }

            has_data |= !pending_event && FD_ISSET(it.second->get_fd(),&_r_set) ;

            if (has_data) {
                if (get_event(it.second.get(),msg,_has_error)) {
                    // Check if its connectivity group object
                    if (cps_api_key_matches(&__grp_key, cps_api_object_key(msg), true) == 0) {
                        EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"EVT-WAIT","Received connectivity group object");
                        __maintain_connections(nh);
                        continue;
                    }
                    
                    if (!nh->object_matches_filter(msg)) continue;        //throw out if doesn't match
                    std::string node_name;
                    if(cps_api_db_get_node_from_ip(it.first,node_name)) {
                        cps_api_object_attr_add(msg,CPS_OBJECT_GROUP_NODE,node_name.c_str(),node_name.size()+1);
                    }
                    EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"EVT-WAIT","Waiting for event returned %s",
                            cps_api_object_to_c_string(msg).c_str());

                    //_sequence_tracker
                    uint64_t*_id = (uint64_t*)cps_api_object_get_data(msg,CPS_OBJECT_GROUP_THREAD_ID);
                    uint64_t*_seq = (uint64_t*)cps_api_object_get_data(msg,CPS_OBJECT_GROUP_SEQUENCE);
                    if (_id!=nullptr && _seq!=nullptr) {
                        if (nh->_sequence_tracker[*_id]>=(*_seq)) {
                            EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"EVT-RECV","Recieved a unusual sequence number %d:%d"
                                    , (int)*_id,(int)*_seq);
                        } else {
                            nh->_sequence_tracker[*_id]=*_seq;//don't increase in negative case
                        }
                    }
                    return cps_api_ret_code_OK;
                } else {
                    if (_has_error) {
                        //error logged elsewhere
                        EV_LOGGING(CPS-DB-EV-CONN,DEBUG,"DISCON","Error on the get_event");
                        nh->disconnect_node(it.first,true);
                        break;
                    }
                }
            }
        }
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
