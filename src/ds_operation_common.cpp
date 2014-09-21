/**
 * filename: db_operation_common.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */
/** OPENSOURCELICENSE */
/*
 * db_operation_common.cpp
 */

#include "ds_operation.h"
#include "std_mutex_lock.h"

#include <vector>
#include <string.h>



typedef std::vector<ds_registration_functions_t> reg_functions_t;
typedef std::vector<reg_functions_t> db_instances_t;

static db_instances_t db_instance_handlers;
//static std_mutex_lock_create_static_init_rec(db_handle_lock);
static std_mutex_lock_create_static_init_fast(db_init_lock);

static db_instances_t &get() {
    std_mutex_simple_lock_guard g(&db_init_lock);
    static bool inited = false;
    if (!inited) {
        db_instance_handlers.resize(ds_inst_MAX);
    }
    return db_instance_handlers;
}
ds_object_type_t db_object_type_operation_add(ds_object_type_t obj, ds_operation_types_t type) {
    return obj | ((((ds_object_type_t)(type)) << DB_CAT_RES_SHIFT) & DB_CAT_RES_MASK);
}

extern "C" {

ds_return_code_t ds_register(ds_registration_functions_t * reg) {
    get()[reg->instance_type].push_back(*reg);
    return ds_ret_code_OK;
}

//XXX todo add read/write lock to db list function calls - for now assume after init
//no registration change
ds_return_code_t ds_get(ds_get_params_t * param) {
    reg_functions_t & r = get()[param->instance];
    size_t ix = 0;
    size_t mx = r.size();
    ds_object_category_types_t cat = DB_OBJ_CAT(param->type);
    ds_return_code_t rc = ds_ret_code_OK;
    for ( ; ix < mx ; ++ix ) {

        if (r[ix].object_category == cat) {
            rc = r[ix].db_read_function(r[ix].context,param);
            if (rc!=ds_ret_code_OK) break;
        }
    }
    return rc;
}

ds_return_code_t ds_commit(ds_transaction_params_t * param) {
    reg_functions_t & r = get()[param->instance];
    ds_return_code_t rc = ds_ret_code_OK;
    size_t len  = ds_list_get_len(param->list);
    size_t s_ix = 0;
    for ( ; s_ix < len ;++s_ix ) {
        ds_list_entry_t *l = ds_list_elem_get(param->list,s_ix);
        size_t ix = 0;
        size_t mx = r.size();
        ds_object_category_types_t oc = DB_OBJ_CAT(l->type);
        for ( ; ix < mx ; ++ix ) {
            if (r[ix].object_category == oc) {
                rc = r[ix].db_write_function(r[ix].context,l);
                if (rc!=ds_ret_code_OK) {
                    /** @TODO implement rollback if any db write fails ... */
                    break;
                }
            }
        }
    }
    return rc;
}

ds_return_code_t ds_get_request_init(ds_get_params_t *req, ds_instance_t db_type, ds_object_type_t objtype) {
    memset(req,0,sizeof(*req));
    req->instance = db_type;
    req->type = objtype;
    req->list = DS_LIST_ALLOC;
    if (req->list==NULL) return ds_ret_code_ERR;
    req->keys = DS_LIST_ALLOC;
    if (req->keys==NULL) {
        ds_list_destroy(req->list);
        return ds_ret_code_ERR;
    }

    return ds_ret_code_OK;
}
ds_return_code_t ds_get_request_close(ds_get_params_t *req) {
    if (req->list!=NULL) ds_list_destroy(req->list);
    if (req->keys!=NULL) ds_list_destroy(req->keys);
    return ds_ret_code_OK;
}

ds_return_code_t ds_transaction_init(ds_transaction_params_t *req, ds_instance_t db_type) {
    memset(req,0,sizeof(*req));
    req->list = DS_LIST_ALLOC;
    if (req->list==NULL) return ds_ret_code_ERR;
    req->instance = db_type;
    return ds_ret_code_OK;
}

ds_return_code_t ds_transaction_close(ds_transaction_params_t *req) {
    if (req->list!=NULL) ds_list_destroy(req->list);
    return ds_ret_code_OK;
}

ds_return_code_t ds_get_add_key(ds_get_params_t * param,ds_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    if (!ds_list_elem_add(param->keys,type,data,len,deep_copy)) {
        return ds_ret_code_ERR;
    }
    return ds_ret_code_OK;
}

static ds_return_code_t ds_tran_op_append(ds_transaction_params_t * param,ds_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    if (!ds_list_elem_add(param->list,type,data,len,deep_copy)) {
        return ds_ret_code_ERR;
    }
    return ds_ret_code_OK;
}


ds_operation_types_t ds_object_type_operation(ds_object_type_t obj)  {
    return  (ds_operation_types_t) ((obj & (DB_CAT_RES_MASK))>>(DB_CAT_CAT_WID+DB_CAT_OBJ_WID));
}

ds_return_code_t ds_set(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return ds_tran_op_append(trans,
            db_object_type_operation_add(type,ds_oper_SET),data,len,deep_copy);
}

ds_return_code_t ds_create(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return ds_tran_op_append(trans,
            db_object_type_operation_add(type,ds_oper_CREATE),data,len,deep_copy);
}

ds_return_code_t ds_delete(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return ds_tran_op_append(trans,
            db_object_type_operation_add(type,ds_oper_DELETE),data,len,deep_copy);
}

}
