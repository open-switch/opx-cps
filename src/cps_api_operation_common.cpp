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

typedef std::vector<ds_object_category_types_t> processed_objs_t;

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
        db_functions.resize(ds_inst_MAX);
        if (std_rw_lock_create_default(&db_list_lock)!=STD_ERR_OK) {
            EV_LOG(ERR,DSAPI,0,"DB-INIT-FAILED","Failed to create rw lock");
        }
    }
}

static reg_functions_t &get() {
    db_operation_init();
    return db_functions;
}

extern "C" {

cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg) {
    db_operation_init();
    std_rw_lock_write_guard g(&db_list_lock);
    uint32_t inst = cps_api_key_element_at(&reg->key,CPS_OBJ_KEY_INST_POS);

    STD_ASSERT(inst<get().size());

    if (inst>=get().size()) return cps_api_ret_code_ERR;

    db_functions.push_back(*reg);
    return cps_api_ret_code_OK;
}
cps_api_return_code_t cps_api_get(cps_api_get_params_t * param) {
    db_operation_init();
    std_rw_lock_read_guard g(&db_list_lock);
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    size_t ix = 0;
    size_t mx = db_functions.size();
    for ( ; ix < mx ; ++ix ) {
        size_t k_ix = 0;
        size_t k_mx = param->key_count;
        for ( ; k_ix < k_mx ; ++k_ix ) {
            if (cps_api_key_matches(&param->keys[k_ix],&(db_functions[ix].key),false)==0) {
                rc = db_functions[ix].db_read_function(db_functions[ix].context,param,k_ix);
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
            if (cps_api_key_matches(cps_api_object_key(l),&(db_functions[ix].key),false)==0) {
                rc = db_functions[ix].db_write_function(db_functions[func_ix].context,param,ix);
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
                if (cps_api_key_matches(cps_api_object_key(l),
                        &(db_functions[ix].key),false)==0) {
                    rc = db_functions[ix].db_rollback_function(db_functions[func_ix].context,param,ix);
                    if (rc!=cps_api_ret_code_OK) break;
                }
            }
        }

    }

    return rc;
}

cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req) {
    memset(req,0,sizeof(*req));
    req->object = cps_api_object_list_create();
    if (req->object==NULL) return cps_api_ret_code_ERR;
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_get_request_close(cps_api_get_params_t *req) {
    if (req->object!=NULL) cps_api_object_list_destroy(req->object,true);
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req, cps_api_instance_t db_type) {
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


cps_api_operation_types_t cps_api_object_type_operation(cps_obj_key_t *key)  {
    return  (cps_api_operation_types_t) cps_api_key_get_attr(key);
}

cps_api_return_code_t cps_api_set(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),ds_oper_SET);
    return ds_tran_op_append(trans,object);
}

cps_api_return_code_t cps_api_create(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),ds_oper_CREATE);
    return ds_tran_op_append(trans,object);
}

cps_api_return_code_t cps_api_delete(cps_api_transaction_params_t * trans,
        cps_api_object_t object) {
    cps_api_key_set_attr(cps_api_object_key(object),ds_oper_DELETE);
    return ds_tran_op_append(trans,object);
}

}
