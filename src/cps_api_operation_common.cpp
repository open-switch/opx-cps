/*
 * Copyright (c) 2019 Dell Inc.
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

/**
 * filename: cps_api_operation_common.cpp
 **/



#include "cps_api_operation.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "std_rw_lock.h"
#include "event_log.h"
#include "cps_api_event_init.h"
#include "cps_dictionary.h"
#include "cps_api_core_utils.h"
#include "dell-cps.h"
#include "cps_api_object_tools.h"

#include "cps_api_object_tools_internal.h"

#include "cps_api_db_operations.h"
#include "private/cps_api_client_utils.h"
#include "private/cps_ns.h"


#include <algorithm>
#include <vector>
#include <string.h>
#include <stdarg.h>
#include <memory>

typedef std::vector<cps_api_object_category_types_t> processed_objs_t;

bool cps_api_filter_set_count(cps_api_object_t obj, size_t obj_count) {
    return cps_api_object_attr_add_u64(obj,CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES,(uint64_t)obj_count);
}

bool cps_api_filter_get_count(cps_api_object_t obj, size_t *obj_count) {
    uint64_t *p = (uint64_t*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_NUMBER_OF_ENTRIES);
    if (p==nullptr) return false;
    *obj_count = *p;
    return true;
}

bool cps_api_filter_set_getnext(cps_api_object_t obj) {
    return cps_api_object_attr_add_u32(obj,CPS_OBJECT_GROUP_GET_NEXT,true);
}


bool cps_api_filter_is_getnext(cps_api_object_t obj) {
    uint32_t *p = (uint32_t*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_GET_NEXT);
    if (p==nullptr) return false;
    return *p;

}

bool cps_api_object_set_timestamp(cps_api_object_t obj, uint64_t timestamp)
{
    return cps_api_object_attr_add_u64(obj, CPS_OBJECT_GROUP_USER_TIMESTAMP_NSEC, timestamp);
}

uint64_t cps_api_object_get_timestamp(cps_api_object_t obj)
{
    cps_api_object_attr_t attr = cps_api_object_attr_get(obj, CPS_OBJECT_GROUP_USER_TIMESTAMP_NSEC);
    if (attr != NULL) {
            return (uint64_t)cps_api_object_attr_data_u64(attr);
    }
    return (uint64_t)0;
}

void cps_api_key_set_qualifier(cps_api_key_t *key, cps_api_qualifier_t qual) {
    cps_api_key_set(key,CPS_OBJ_KEY_INST_POS,qual);
}

CPS_CONFIG_TYPE_t cps_api_object_get_config_type(cps_api_object_t obj) {
    uint32_t *p = (uint32_t*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_CONFIG_TYPE);
    if (p==nullptr) return CPS_CONFIG_TYPE_RUNNING_CONFIG;
    return *(CPS_CONFIG_TYPE_t*)p;
}

bool cps_api_object_set_config_type(cps_api_object_t obj, CPS_CONFIG_TYPE_t type) {
    return cps_api_object_attr_add_u32(obj,CPS_OBJECT_GROUP_CONFIG_TYPE,type);
}

void cps_api_key_init(cps_api_key_t * key,
        cps_api_qualifier_t qual,
        cps_api_object_category_types_t cat,
        cps_api_object_subcategory_types_t subcat, size_t number_of_inst, ...) {

    va_list v;

    cps_api_key_set_attr(key,0);
    size_t key_len = 0;

    if (qual!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_INST_POS,qual);
        ++key_len;
    }

    if ((key_len > CPS_OBJ_KEY_INST_POS) && cat!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_CAT_POS,cat);
        ++key_len;
    }
    if ((key_len > CPS_OBJ_KEY_CAT_POS) && subcat!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_SUBCAT_POS,subcat);
        ++key_len;
    }
    if (key_len >CPS_OBJ_KEY_SUBCAT_POS) {
        size_t ix = 0;
        size_t mx = number_of_inst;
        va_start(v,number_of_inst);
        for ( ; ix < mx ; ++ix ) {
            int val = va_arg(v,int);

            cps_api_key_set(key,CPS_OBJ_KEY_APP_INST_POS+ix,val);
            ++key_len;
        }
        va_end(v);
    }

    cps_api_key_set_len(key,key_len);
}

static void cps_api_object_list_swap(cps_api_object_list_t &a, cps_api_object_list_t &b) {
    cps_api_object_list_t t = a;
    a = b;
    b = t;
}


static bool _cps_api_get_clone(cps_api_get_params_t * dest, cps_api_get_params_t * src) {
    if (src==nullptr || dest==nullptr) return false;

    cps_api_object_list_destroy(dest->filters,true);
    dest->filters = cps_api_object_list_clone(src->filters,false);

    size_t ix = 0;

    size_t mx = src->key_count;
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t obj = cps_api_object_list_create_obj_and_append(dest->filters);
        if (obj==NULL) return false;
        cps_api_key_copy(cps_api_object_key(obj),src->keys+ix);
    }

    dest->timeout = src->timeout;
    dest->keys = nullptr;
    dest->key_count = 0;
    return true;
}


void _filter_list_as_needed(cps_api_object_t obj, cps_api_object_list_t results, size_t from, size_t to) {
    if (!cps_api_object_get_exact_match_flag(obj)) return;

    size_t _list_walker = from;

    for ( ; from < to ; ++from ) {
        cps_api_object_t o = cps_api_object_list_get(results,from);
        if (o!=nullptr) {
            if (!cps_api_obj_tool_matches_filter(obj,o,true)) {
                cps_api_object_list_set(results,from,nullptr,true);
            }
        }
    }
    size_t _lst_mx = cps_api_object_list_size(results);
    while (_list_walker< _lst_mx ) {
        if (cps_api_object_list_get(results,_list_walker)==nullptr) {
            cps_api_object_list_remove(results,_list_walker);
            --_lst_mx;
            continue;
        }
        ++_list_walker;
    }

}

cps_api_return_code_t cps_api_get(cps_api_get_params_t * param) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    cps_api_get_params_t new_req;

    bool _copy_param = param->key_count!=0;
    cps_api_get_request_guard rg(nullptr);

    if (_copy_param) {    //copy the get request and add the keys if.. the number of just keys > 0
        if (cps_api_get_request_init(&new_req)!=cps_api_ret_code_OK) {
            return cps_api_ret_code_INTERNAL_FAILURE;
        }
        rg.set(&new_req);

        if (!_cps_api_get_clone(&new_req,param)) {
            return cps_api_ret_code_INTERNAL_FAILURE;
        }

        cps_api_object_list_swap(param->list,new_req.list);
    }

    cps_api_get_params_t * get_req = _copy_param ? &new_req : param;

    size_t mx = cps_api_object_list_size(get_req->filters);

    for ( size_t ix = 0 ; ix < mx ; ++ix ) {
        size_t _cur_lst_len = cps_api_object_list_size(param->list);
        cps_api_object_t o = cps_api_object_list_get(get_req->filters,ix);

        STD_ASSERT(o!=nullptr);
        if (cps_api_obj_get_ownership_type(o)!=CPS_API_OBJECT_SERVICE) {
            rc = cps_api_db_operation_get(get_req,ix);
        } else {
            rc=cps_api_process_get_request(get_req,ix);
        }
        if (rc!=cps_api_ret_code_OK) {
        	if (!cps_api_obj_attr_get_bool(o,CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE)) {
        		break;
        	}
        }
        _filter_list_as_needed(o,param->list,_cur_lst_len,cps_api_object_list_size(param->list));
    }

    if (_copy_param) {
        //based on the return - get the response list
        cps_api_object_list_swap(param->list,new_req.list);
    }

    return rc;
}

cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param) {
    cps_api_return_code_t rc =cps_api_ret_code_OK;
    ///TODO remove any previous operation status from the object

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(param->change_list);


    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
        cps_api_object_attr_delete(o,CPS_OBJECT_GROUP_FAILED_NODES);
        STD_ASSERT(o!=nullptr);

        ///TODO until the performance fix is updated - made the temporary fix
        bool _is_db_handled = cps_api_obj_get_ownership_type(o)!=CPS_API_OBJECT_SERVICE;

        if (_is_db_handled) {
            rc = cps_api_db_operation_commit(param,ix);
        } else {
            rc = cps_api_process_commit_request(param,ix);
        }

        //trickly logic start... if the operation failed... and if want to continue - reset the return code to OK
        if (rc!=cps_api_ret_code_OK) {
            bool continue_on_fail = cps_api_obj_attr_get_bool(
                    o,CPS_OBJECT_GROUP_CONTINUE_ON_FAILURE);
            char obj_str[1024];
            EV_LOG(ERR,DSAPI,0,"COMMIT",
                    "Error %d: commit %zu/%zu failed, type %d, %s (%s)",
                    rc, ix+1, mx, cps_api_obj_get_ownership_type(o),
                    continue_on_fail ? "continue" : "break",
                    cps_api_object_to_string(o, obj_str, sizeof(obj_str)));
            if (!continue_on_fail) {
                break;
            }
            // override fail rc, reset to OK
            rc=cps_api_ret_code_OK;
        } else {
        	//if return was ok, publish event
        	if (!_is_db_handled && cps_api_obj_has_auto_events(o) && cps_api_object_type_operation(cps_api_object_key(o))!=cps_api_oper_ACTION) {
        		cps_api_core_publish(o);
        	}
        }
    }
    if (rc!=cps_api_ret_code_OK) {
        cps_api_return_code_t _revert_rc = cps_api_ret_code_OK;
        mx = ix;

        while (mx > 0) {    //from the last to the first
            ix = mx - 1;
            cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
            STD_ASSERT(o!=nullptr);
            if (cps_api_obj_get_ownership_type(o)!=CPS_API_OBJECT_SERVICE) {
                _revert_rc = cps_api_db_operation_rollback(param,ix);
            } else {
                _revert_rc=cps_api_process_rollback_request(param,ix);
            }

            if (_revert_rc!=cps_api_ret_code_OK) {
                EV_LOG(ERR,DSAPI,0,"ROLLBACK","Failed to rollback request at %d (rc:%d)",ix,_revert_rc);
            }
            cps_api_operation_types_t op_type = cps_api_object_type_operation(cps_api_object_key(o));

            if (rc==cps_api_ret_code_OK && op_type!=cps_api_oper_ACTION) {
                cps_api_operation_types_t tmp_op_type = cps_api_oper_SET;
                if (op_type==cps_api_oper_CREATE) tmp_op_type=cps_api_oper_DELETE;
                if (op_type==cps_api_oper_DELETE) tmp_op_type = cps_api_oper_CREATE;
                cps_api_object_set_type_operation(cps_api_object_key(o),tmp_op_type);
                cps_api_core_publish(o);
                cps_api_object_set_type_operation(cps_api_object_key(o),op_type);
            }
            --mx;
        }
    }
    return rc;
}

cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req) {
    memset(req,0,sizeof(*req));
    req->list = cps_api_object_list_create();
    if (req->list==NULL) return cps_api_ret_code_ERR;
    req->filters = cps_api_object_list_create();
    if (req->filters==NULL) {
        cps_api_object_list_destroy(req->list,true);
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_get_request_close(cps_api_get_params_t *req) {
    if (req==nullptr) return cps_api_ret_code_ERR;
    if (req->list!=NULL) cps_api_object_list_destroy(req->list,true);
    req->list = NULL;
    if (req->filters!=NULL) {
        cps_api_object_list_destroy(req->filters,true);
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req) {
    memset(req,0,sizeof(*req));
    req->change_list = cps_api_object_list_create();
    if (req->change_list==NULL) return cps_api_ret_code_ERR;

    req->prev = cps_api_object_list_create();
    if (req->prev==NULL) {
        cps_api_object_list_destroy(req->change_list,true);
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_transaction_close(cps_api_transaction_params_t *req) {
    if (req->change_list!=NULL) cps_api_object_list_destroy(req->change_list,true);
    if (req->prev!=NULL) cps_api_object_list_destroy(req->prev,true);
    req->change_list = NULL;
    req->prev = NULL;
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t ds_tran_op_append(cps_api_transaction_params_t * param,
        cps_api_object_t object) {
    if (!cps_api_object_list_append(param->change_list,object)) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

void cps_api_object_set_type_operation(cps_api_key_t *key,cps_api_operation_types_t op)  {
    cps_api_key_set_attr(key,op);
}

cps_api_operation_types_t cps_api_object_type_operation(cps_api_key_t *key)  {
    return  (cps_api_operation_types_t) cps_api_key_get_attr(key);
}

cps_api_return_code_t cps_api_set(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),cps_api_oper_SET);
    return ds_tran_op_append(trans,object);
}

cps_api_return_code_t cps_api_create(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),cps_api_oper_CREATE);
    return ds_tran_op_append(trans,object);
}

cps_api_return_code_t cps_api_delete(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),cps_api_oper_DELETE);
    return ds_tran_op_append(trans,object);
}

cps_api_return_code_t cps_api_action(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),cps_api_oper_ACTION);
    return ds_tran_op_append(trans,object);
}

bool cps_api_unittest_init(void) {
    return cps_api_event_service_init()==cps_api_ret_code_OK &&
            cps_api_ns_startup()==cps_api_ret_code_OK;
}

bool cps_api_is_registered(cps_api_key_t *key, cps_api_return_code_t *rc) {
    cps_api_object_owner_reg_t owner;
    if (rc!=nullptr) *rc = cps_api_ret_code_OK;
    return cps_api_find_owners(key,owner);
}

bool cps_api_filter_wildcard_attrs(cps_api_object_t obj, bool has_wildcard_attributes) {
    while (cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_WILDCARD_SEARCH)) ;
    return cps_api_object_attr_add_u32(obj,CPS_OBJECT_GROUP_WILDCARD_SEARCH,has_wildcard_attributes);
}

bool cps_api_filter_has_wildcard_attrs(cps_api_object_t obj) {
    uint32_t *p = (uint32_t*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_WILDCARD_SEARCH);
    if (p==nullptr) return false;
    return *p;
}

bool cps_api_attr_create_escaped(cps_api_object_ATTR_TYPE_t type, void *buff, size_t len, const void *attr, size_t *attr_len) {
    if(*attr_len < 2*len)
      return false;

    union {
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
    } un;

    switch(type) {
        case cps_api_object_ATTR_T_U16:
            if(len != sizeof(uint16_t))
                return false;
            un.u16 = htole16(*(uint16_t*)buff); *((uint16_t *)buff) = un.u16; break;
        case cps_api_object_ATTR_T_U32:
            if(len != sizeof(uint32_t))
                return false;
            un.u32 = htole32(*(uint32_t*)buff); *((uint32_t *)buff) = un.u32; break;
        case cps_api_object_ATTR_T_U64:
            if(len != sizeof(uint64_t))
                return false;
            un.u64 = htole64(*(uint64_t*)buff); *((uint64_t *)buff) = un.u64; break;
        default:
            break;
    }
    size_t count = 0;
    for(size_t ix = 0; ix < len; ++ix, ++count) {
      if(((char *)buff)[ix] == '*' || ((char *)buff)[ix] == '?' || ((char *)buff)[ix] == '[' || ((char *)buff)[ix] == ']' || ((char *)buff)[ix] == '\\') {
        ((char *)attr)[count] = '\\';
        ++count;
      }
      ((char *)attr)[count] = ((char *)buff)[ix];
    }
    *attr_len = count;

    return true;
}

