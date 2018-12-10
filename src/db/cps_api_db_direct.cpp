
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


#include "cps_api_db_interface.h"

#include "cps_api_node_set.h"
#include "cps_api_node.h"

#include "cps_api_db.h"

#include "cps_api_db_connection_tools.h"
#include "cps_api_key_utils.h"
#include "cps_api_object_tools.h"
#include "cps_class_map.h"
#include "cps_api_operation.h"
#include "dell-cps.h"
#include "cps_api_object_tools_internal.h"
#include "cps_api_node_private.h"

#include "cps_class_map_query.h"
#include "cps_api_core_utils.h"

#include "event_log.h"

#include <vector>
#include <string>
#include <string.h>

namespace {

size_t _ignore_communication_errors = 0;

size_t _communication_error_retry_timeout = 200;    //200ms
size_t _communication_error_retry = (1000/200)*60*20;    //20 minutes

pthread_once_t  onceControl = PTHREAD_ONCE_INIT;

void __init(void) {
    cps_api_update_ssize_on_param_change("cps.db.ignore-comm-failure",(ssize_t*)&_ignore_communication_errors);
    cps_api_update_ssize_on_param_change("cps.db.comm-failure-retry",(ssize_t*)&_communication_error_retry);
    cps_api_update_ssize_on_param_change("cps.db.comm-failure-retry-delay",(ssize_t*)&_communication_error_retry_timeout);
}

static inline bool _handle_retry_request(cps_api_return_code_t rc, size_t retry_count) {
    if (rc==cps_api_ret_code_COMMUNICATION_ERROR &&
            _ignore_communication_errors!=0) {
        if (retry_count>=_communication_error_retry) return false;
        EV_LOGGING(CPS-DB-CONN,WARNING,"COMM-RETRY","Operation being retried - #%d in %dms - %s",
                (int)retry_count,(int)_communication_error_retry_timeout,cps_api_stacktrace().c_str());

        std_usleep((size_t)_communication_error_retry_timeout*1000);
        return true;
    }
    return false;
}

std::string _get_node_name(const std::string &str) {
    std::string s;
    if(!cps_api_db_get_node_from_ip(str,s)) {
        s = str;
    }
    return s;
}

bool _conn_event_enabled(cps_api_object_t filter) {
    return cps_api_obj_attr_get_bool(filter,CPS_CONNECTION_ENTRY_CONNECTION_STATE);
}

bool _continue_on_failure(cps_api_object_t filter) {
    return cps_api_obj_attr_get_bool(filter,CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE);
}

cps_api_return_code_t __cps_api_db_operation_get(cps_api_object_t obj, cps_api_object_list_t results) {

    cps_api_return_code_t rc=_continue_on_failure(obj) ? cps_api_ret_code_OK :cps_api_ret_code_ERR;

    const char * _group = cps_api_key_get_group(obj);

    cps_api_node_set_iterate(_group,[&obj,&results,&rc,_group](const std::string &name) -> bool{
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        if (!r.valid()) { rc = cps_api_ret_code_COMMUNICATION_ERROR; return true; }

        size_t _prev_obj_ptr = cps_api_object_list_size(results);

        if (cps_db::get_objects(r.get(),obj,results,&rc)) {
            rc = cps_api_ret_code_OK;
        }
        std::string node_name = _get_node_name(name);

        if (rc==cps_api_ret_code_OK && !cps_api_key_is_local_group(name.c_str())) {
            size_t _mx = cps_api_object_list_size(results);
            for ( ; _prev_obj_ptr < _mx ; ++_prev_obj_ptr ) {
                cps_api_object_t _o = cps_api_object_list_get(results,_prev_obj_ptr);
                STD_ASSERT(_o!=nullptr);
                cps_api_key_set_node(_o,node_name.c_str());
            }
        }
        if (rc!=cps_api_ret_code_OK) {
            bool _con_state_needed = _conn_event_enabled(obj);

            if (_con_state_needed) {
                cps_api_object_t o = cps_api_object_list_create_obj_and_append(results);
                cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY,cps_api_qualifier_OBSERVED);
                cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME,node_name.c_str(),node_name.size()+1);
                cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP,name.c_str(),name.size()+1);
                cps_api_key_set_group(o,_group);
                cps_api_key_set_node(o,node_name.c_str());
            }
        }
        return true;
    });

    return rc;
}

}

bool cps_api_db_get_filter_enable_connection(cps_api_object_t obj) {
    bool _enabled=true;
    return cps_api_object_attr_add(obj,CPS_CONNECTION_ENTRY_CONNECTION_STATE,&_enabled,sizeof(_enabled))
            ==cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_db_get(cps_api_object_t obj,cps_api_object_list_t found) {
    pthread_once(&onceControl,__init);
    cps_api_return_code_t _rc=cps_api_ret_code_OK;
    size_t _count = 0;
    do {
        _rc = __cps_api_db_operation_get(obj,found);
        if (_handle_retry_request(_rc,_count++)) continue;
        break;
    } while (true);

    return _rc;
}

static cps_api_return_code_t __cps_api_db_get_bulk(cps_api_object_list_t objs, const char * node_group) {
    pthread_once(&onceControl,__init);

    if (node_group==nullptr) {
        node_group = DEFAULT_REDIS_ADDR;
    }
    cps_api_return_code_t _rc = cps_api_ret_code_ERR;

    if (!cps_api_node_set_iterate(node_group,[&](const std::string &name) -> bool{
        if (_rc!=cps_api_ret_code_OK) _rc = cps_api_ret_code_COMMUNICATION_ERROR;

        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        if (!r.valid()) { return true; }
        if (cps_db::get_object_list(r.get(),objs,&_rc)) {
            _rc=cps_api_ret_code_OK;
        }
        return true;

    })) {
        return _rc;
    }
    return _rc;
}

cps_api_return_code_t cps_api_db_get_bulk(cps_api_object_list_t objs, const char * node_group) {
    cps_api_return_code_t _rc=cps_api_ret_code_OK;
    size_t _count = 0;
    do {
        _rc = __cps_api_db_get_bulk(objs,node_group);
        if (_handle_retry_request(_rc,_count++)) continue;
        break;
    } while (true);

    return _rc;
}

namespace {
    cps_api_return_code_t __one_pre_load_into_prev(std::vector<std::string> &service_addrs, cps_api_object_t obj,
            cps_api_object_t prev) {
        cps_api_return_code_t _rc = cps_api_ret_code_ERR;
        std::vector<char> key;
        if (!cps_db::dbkey_from_instance_key(key,obj,false)) return cps_api_ret_code_ERR;

        for (auto &it : service_addrs) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) { _rc = cps_api_ret_code_COMMUNICATION_ERROR; continue; }
            if (cps_db::get_object(r.get(),key,prev,&_rc)) {
                return cps_api_ret_code_OK;
            }
            if (_rc==cps_api_ret_code_NO_EXIST) break;    //no sense in checking
        }
        return _rc;
    }
    cps_api_return_code_t __one_pre_load_into_prev_delete(std::vector<std::string> &service_addrs, cps_api_object_t obj,
            cps_api_object_t prev) {
        (void)__one_pre_load_into_prev(service_addrs,obj,prev);
        EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-PRE-DEL","Prev objects is %s",cps_api_object_to_c_string(prev).c_str());
        return cps_api_ret_code_OK;
    }

    cps_api_return_code_t __one_pre_load_into_prev_for_set(std::vector<std::string> &service_addrs, cps_api_object_t obj,cps_api_object_t prev) {

        (void)__one_pre_load_into_prev(service_addrs,obj,prev);
        EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-PRE-SET","Prev objects is %s",cps_api_object_to_c_string(prev).c_str());


        //in this case, if there is no valid key in the prev object - it doesn't exist or wasn't retrievable.
        if (cps_api_key_matches(cps_api_object_key(obj),cps_api_object_key(prev),true)!=0) {
            EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-PRE-SET","Initiing object to match %s",cps_api_object_to_c_string(obj).c_str());
            //if the object key was invalid create a new blank object and return it into the prev
            cps_api_object_guard og(cps_api_object_create());
            cps_api_key_copy(cps_api_object_key(og.get()),cps_api_object_key(obj));
            cps_api_object_swap(og.get(),prev);
        }
        return cps_api_ret_code_OK;
    }

    cps_api_return_code_t __one_handle_delete(std::vector<std::string> &l, cps_api_object_t obj,cps_api_object_t prev) {
        cps_api_return_code_t rc = cps_api_ret_code_OK;
        for (auto &it : l) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) { rc = cps_api_ret_code_COMMUNICATION_ERROR; continue; }
            (void)cps_db::delete_object(r.get(),obj,&rc);
            EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-DELETE","Deleting object %s",cps_api_object_to_c_string(obj).c_str());
            if (rc!=cps_api_ret_code_COMMUNICATION_ERROR) rc=cps_api_ret_code_OK;
        }
        return rc;    //ignore merge issue
    }

    cps_api_return_code_t __one_handle_create(std::vector<std::string> &l, cps_api_object_t obj,cps_api_object_t prev) {
        cps_api_return_code_t rc = cps_api_ret_code_ERR;

        for (auto &it : l) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) {
                if (rc!=cps_api_ret_code_OK)
                    rc = cps_api_ret_code_COMMUNICATION_ERROR;
                continue;
            }
            EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-CREATE","Storing %s",cps_api_object_to_c_string(obj).c_str());
            if (!cps_db::store_object(r.get(),obj)) {
                if (!_continue_on_failure(obj)) return cps_api_ret_code_COMMUNICATION_ERROR;
                continue;
            }
            rc=cps_api_ret_code_OK;
        }
        return rc;    //ignore merge issue
    }

    cps_api_return_code_t __one_handle_set(std::vector<std::string> &l, cps_api_object_t obj,cps_api_object_t prev) {

        cps_api_object_guard merged(cps_api_object_create_clone(prev));

        if (!cps_api_object_attr_merge(merged.get(),obj,true)) return cps_api_ret_code_ERR;

        cps_api_return_code_t rc = cps_api_ret_code_ERR;

        for (auto &it : l) {
            if (rc!=cps_api_ret_code_OK) rc = cps_api_ret_code_COMMUNICATION_ERROR;

            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) continue;
            EV_LOGGING(CPS-DB-COMM-ONE,DEBUG,"COMMIT-SET","Storing %s",cps_api_object_to_c_string(merged.get()).c_str());
            if (!cps_db::store_object(r.get(),merged.get())) {
                if (!_continue_on_failure(obj)) return cps_api_ret_code_COMMUNICATION_ERROR;
                continue;
            }
            rc=cps_api_ret_code_OK;
        }
        return rc;    //ignore merge issue
    }
}

static cps_api_return_code_t __cps_api_db_commit_one(cps_api_operation_types_t op,cps_api_object_t obj,cps_api_object_t prev, bool publish) {
    pthread_once(&onceControl,__init);

    if (op>cps_api_oper_SET || op<=cps_api_oper_NULL)
        return cps_api_ret_code_ERR;

    cps_api_object_guard og(prev==nullptr ? cps_api_object_create() : nullptr);
    prev = prev==nullptr ? og.get() : prev;

    std::vector<std::string> lst;
    if (!cps_api_db_get_node_group(cps_api_key_get_group(obj),lst)) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Failed to get node details.");
        return cps_api_ret_code_ERR;
    }

    struct {
        cps_api_return_code_t (*handle)(std::vector<std::string> &, cps_api_object_t,cps_api_object_t);
    } pre_hook [cps_api_oper_SET+1] = {
            nullptr/*cps_api_oper_NULL*/,
            __one_pre_load_into_prev_delete,     /*Delete*/
            nullptr,                    /*Create*/
            __one_pre_load_into_prev_for_set,    /*set*/
    };
    cps_api_return_code_t rc = cps_api_ret_code_OK;

    if(pre_hook[op].handle!=nullptr) {
        rc = pre_hook[op].handle(lst,obj,prev);
        EV_LOGGING(CPS-DB-COMM-ONE,INFO,"COMMIT","Pre-commit handler for op %d result is %d",
                (int)op,(int)rc);
        EV_LOGGING(CPS-DB-COMM-ONE,INFO,"COMMIT","Committed objects are %s and %s",
                cps_api_object_to_c_string(obj).c_str(),cps_api_object_to_c_string(prev).c_str());
    }

    if (rc!=cps_api_ret_code_OK) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Prehook failed for operation %d",(int)op);
        return rc;
    }
    struct {
        cps_api_return_code_t (*handle)(std::vector<std::string> &, cps_api_object_t, cps_api_object_t);
    } handlers [cps_api_oper_SET+1] = {
            nullptr,
            __one_handle_delete,
            __one_handle_create,
            __one_handle_set
    };

    if(handlers[op].handle!=nullptr) {
        rc = handlers[op].handle(lst,obj,prev);
        EV_LOGGING(CPS-DB-COMM-ONE,INFO,"COMMIT","Commit handler for op %d result is %d",
                (int)op,(int)rc);
    }

    if (rc!=cps_api_ret_code_OK) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Update failed for operation %d",(int)op);
        return rc;
    }

    if (publish) {
        bool _first_time = true;
        for ( auto &it : lst ) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it);
            if (!r.valid()) continue;
            if (_first_time) cps_api_object_set_type_operation(cps_api_object_key(obj),op);
            if (!cps_db::publish(r.get(),obj)) {
                EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","failed to publish event %d",(int)op);
            }
            _first_time = false;
        }
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_db_commit_one(cps_api_operation_types_t op,cps_api_object_t obj,cps_api_object_t prev, bool publish) {
    cps_api_return_code_t _rc=cps_api_ret_code_OK;
    size_t _count = 0;
    do {
        _rc = __cps_api_db_commit_one(op,obj,prev,publish);
        if (_handle_retry_request(_rc,_count++)) continue;
        break;
    } while (true);

    return _rc;
}

namespace {
    cps_api_return_code_t __pre_set(std::vector<std::string> &lst, cps_api_db_commit_bulk_t *param) {
        cps_api_return_code_t rc = cps_api_ret_code_ERR;
        for (auto &it : lst ) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) {
                rc = cps_api_ret_code_COMMUNICATION_ERROR;
                continue;
            }
            if (cps_db::merge_objects(r.get(),param->objects,&rc)) {
                break;
            }
        }
        return rc;
    }

    cps_api_return_code_t __handle_delete(std::vector<std::string> &lst, cps_api_db_commit_bulk_t *param) {

        cps_api_return_code_t _rc = cps_api_ret_code_COMMUNICATION_ERROR;
        for (auto &it : lst ) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());
            if (!r.valid()) continue;
            if (cps_db::delete_object_list(r.get(),param->objects)) {
                _rc = cps_api_ret_code_OK;
            }
        }
        return _rc;
    }

    cps_api_return_code_t __handle_create(std::vector<std::string> &lst, cps_api_db_commit_bulk_t *param) {

        cps_api_return_code_t _rc = cps_api_ret_code_ERR;

        for (auto &it : lst ) {
            //default to communication error as this would be the only failure below (assuming syntax is correct)
            if (_rc==cps_api_ret_code_ERR) _rc = cps_api_ret_code_COMMUNICATION_ERROR;
            cps_db::connection_request r(cps_db::ProcessDBCache(),it);
            if (!r.valid()) {
                continue;
            }

            if (cps_db::store_objects(r.get(),param->objects)) {
                _rc = cps_api_ret_code_OK;
                continue;
            }
        }
        return _rc;
    }
}

bool cps_api_db_commit_bulk_init(cps_api_db_commit_bulk_t*p) {
    memset(p,0,sizeof(*p));
    p->objects = cps_api_object_list_create();
    return p->objects!=nullptr;
}

void cps_api_db_commit_bulk_close(cps_api_db_commit_bulk_t*p) {
    cps_api_object_list_destroy(p->objects,true);
}

static cps_api_return_code_t __cps_api_db_commit_bulk(cps_api_db_commit_bulk_t *param) {
    pthread_once(&onceControl,__init);

    cps_api_operation_types_t op = param->op;
    if (op>cps_api_oper_SET || op<=cps_api_oper_NULL)
        return cps_api_ret_code_PARAM_INVALID;

    if (param->node_group==nullptr) param->node_group = DEFAULT_REDIS_ADDR;
    std::vector<std::string> lst;
    if (!cps_api_db_get_node_group(param->node_group,lst)) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Failed to get node details for %s.",param->node_group);
        return cps_api_ret_code_PARAM_INVALID;
    }

    struct {
        cps_api_return_code_t (*handle)(std::vector<std::string> &, cps_api_db_commit_bulk_t *);
    } pre_hook [cps_api_oper_SET+1] = {
            nullptr/*cps_api_oper_NULL*/,
            nullptr,
            nullptr,
            __pre_set,
    };
    cps_api_return_code_t rc = cps_api_ret_code_OK;

    if(pre_hook[op].handle!=nullptr) rc = pre_hook[op].handle(lst,param);

    if (rc!=cps_api_ret_code_OK) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Prehook failed for operation %d",(int)op);
        return rc;
    }
    struct {
        cps_api_return_code_t (*handle)(std::vector<std::string> &, cps_api_db_commit_bulk_t *);
    } handlers [cps_api_oper_SET+1] = {
            nullptr,
            __handle_delete,
            __handle_create,
            __handle_create
    };

    if(handlers[op].handle!=nullptr) rc = handlers[op].handle(lst,param);

    if (rc!=cps_api_ret_code_OK) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Update failed for operation %d",(int)op);
        return rc;
    }

    if (param->publish) {
        bool _first_time = true;
        for ( auto &it : lst ) {

            cps_db::connection_request r(cps_db::ProcessDBCache(),it);
            if (!r.valid()) continue;
            for (size_t ix = 0, mx = cps_api_object_list_size(param->objects); ix < mx ; ++ix ) {
                cps_api_object_t o = cps_api_object_list_get(param->objects,ix);
                if (_first_time) cps_api_object_set_type_operation(cps_api_object_key(o),op);
                if (!cps_db::publish(r.get(),o)) {
                    EV_LOGGING(CPS-DB-EV,ERR,"EVT-SEND","Failed to send event (contents not displayed)");
                }
            }
            _first_time = false;
        }
    }

    return rc;
}

cps_api_return_code_t cps_api_db_commit_bulk(cps_api_db_commit_bulk_t *param) {
    cps_api_return_code_t _rc = cps_api_ret_code_OK;
    size_t _count = 0;
    do {
        _rc = __cps_api_db_commit_bulk(param);
        if (_handle_retry_request(_rc,_count++)) continue;
        break;
    } while (true);
    return _rc;
}
