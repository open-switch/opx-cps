/**
 * filename: db_operation_common.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */

/*
 * db_operation_common.cpp
 */

#include "cps_api_operation.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "std_rw_lock.h"
#include "event_log.h"

#include <algorithm>
#include <vector>
#include <string.h>

typedef std::vector<cps_api_registration_functions_t> reg_functions_t;

static reg_functions_t db_functions;
static std_rw_lock_t db_list_lock;

static std_mutex_lock_create_static_init_fast(db_init_lock);

typedef std::vector<cps_api_object_category_types_t> processed_objs_t;

template  <typename T>
inline bool push_back(std::vector<T> &list, const T &elem) {
    try {
        list.push_back(elem);
    } catch (...) {
        return false;
    }
    return true;
}

static void db_operation_init() {
    std_mutex_simple_lock_guard g(&db_init_lock);
    static bool inited = false;
    if (!inited) {
        if (std_rw_lock_create_default(&db_list_lock)!=STD_ERR_OK) {
            EV_LOG(ERR,DSAPI,0,"DB-INIT-FAILED","Failed to create rw lock");
        }
    }
}

extern "C" {

void cps_api_key_init(cps_api_key_t * key, size_t len_of_inst_comps,
        cps_api_instance_t inst,
        cps_api_object_category_types_t cat,
        cps_api_object_subcategory_types_t subcat) {

    cps_api_key_set_attr(key,0);
    size_t key_len = 0;

    if (inst!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_INST_POS,inst);
        ++key_len;
    }

    if ((key_len > 0) && cat!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_CAT_POS,cat);
        ++key_len;
    }
    if ((key_len > 0) && subcat!=0) {
        cps_api_key_set(key,CPS_OBJ_KEY_SUBCAT_POS,subcat);
        ++key_len;
    }
    if (key_len == CPS_OBJ_KEY_APP_INST_POS) {
        key_len += len_of_inst_comps;
    }
    cps_api_key_set_len(key,key_len);
}

cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg) {
    db_operation_init();
    std_rw_lock_write_guard g(&db_list_lock);
    db_functions.push_back(*reg);
    return cps_api_ret_code_OK;
}
cps_api_return_code_t cps_api_get(cps_api_get_params_t * param) {
    db_operation_init();
    std_rw_lock_read_guard g(&db_list_lock);
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    size_t ix = 0;
    size_t mx = param->key_count;

    for ( ; ix < mx ; ++ix ) {
        size_t func_ix = 0;
        size_t func_mx = db_functions.size();
        for ( ; func_ix < func_mx ; ++func_ix ) {
            cps_api_registration_functions_t *p = &(db_functions[func_ix]);
            if ((p->_read_function!=NULL) &&
                    (cps_api_key_matches(&param->keys[ix],&p->key,false)==0)) {
                rc = p->_read_function(p->context,param,ix);
                if (rc!=cps_api_ret_code_OK) break;
            }
        }
    }
    return rc;
}

cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param) {
    db_operation_init();
    std_rw_lock_read_guard g(&db_list_lock);
    cps_api_return_code_t rc =cps_api_ret_code_OK;

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(param->list);
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t l = cps_api_object_list_get(param->list,ix);

        size_t func_ix = 0;
        size_t func_mx = db_functions.size();
        for ( ; func_ix < func_mx ; ++func_ix ) {
            cps_api_registration_functions_t *p = &(db_functions[func_ix]);
            if ((p->_write_function!=NULL) &&
                    (cps_api_key_matches(cps_api_object_key(l),&p->key,false)==0)) {
                rc = p->_write_function(p->context,param,ix);
                if (rc!=cps_api_ret_code_OK) break;
            }
        }
    }
    if (rc!=cps_api_ret_code_OK) {
        for ( ; ix < mx ; ++ix ) {
            cps_api_object_t l = cps_api_object_list_get(param->prev,ix);

            size_t func_ix = 0;
            size_t func_mx = db_functions.size();
            for ( ; func_ix < func_mx ; ++func_ix ) {
                cps_api_registration_functions_t *p = &(db_functions[func_ix]);
                if ((p->_rollback_function!=NULL) &&
                        (cps_api_key_matches(cps_api_object_key(l),
                        &p->key,false)==0)) {
                    rc = p->_rollback_function(p->context,param,ix);
                    if (rc!=cps_api_ret_code_OK) break;
                }
            }
        }

    }

    return rc;
}

cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req) {
    memset(req,0,sizeof(*req));
    req->list = cps_api_object_list_create();
    if (req->list==NULL) return cps_api_ret_code_ERR;
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_get_request_close(cps_api_get_params_t *req) {
    if (req->list!=NULL) cps_api_object_list_destroy(req->list,true);
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req) {
    memset(req,0,sizeof(*req));
    req->list = cps_api_object_list_create();
    if (req->list==NULL) return cps_api_ret_code_ERR;

    req->prev = cps_api_object_list_create();
    if (req->prev==NULL) {
        cps_api_object_list_destroy(req->list,true);
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_transaction_close(cps_api_transaction_params_t *req) {
    if (req->list!=NULL) cps_api_object_list_destroy(req->list,true);
    if (req->prev!=NULL) cps_api_object_list_destroy(req->prev,true);
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t ds_tran_op_append(cps_api_transaction_params_t * param,
        cps_api_object_t object) {
    if (!cps_api_object_list_append(param->list,object)) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
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

}
