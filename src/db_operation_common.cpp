/*
 * filename: db_operation_common.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_operation_common.cpp
 */

#include "db_operation.h"
#include "std_mutex_lock.h"

#include <vector>
#include <string.h>



typedef std::vector<db_registration_functions_t> reg_functions_t;
typedef std::vector<reg_functions_t> db_instances_t;

static db_instances_t db_instance_handlers;
//static std_mutex_lock_create_static_init_rec(db_handle_lock);
static std_mutex_lock_create_static_init_fast(db_init_lock);

static db_instances_t &get() {
    std_mutex_simple_lock_guard g(&db_init_lock);
    static bool inited = false;
    if (!inited) {
        db_instance_handlers.resize(db_inst_MAX);
    }
    return db_instance_handlers;
}
db_object_type_t db_object_type_operation_add(db_object_type_t obj, db_operation_types_t type) {
    return obj | ((((db_object_type_t)(type)) << DB_CAT_RES_SHIFT) & DB_CAT_RES_MASK);
}

extern "C" {

db_return_code_t db_register(db_registration_functions_t * reg) {
    get()[reg->instance_type].push_back(*reg);
    return db_ret_code_OK;
}

//XXX todo add read/write lock to db list function calls - for now assume after init
//no registration change
db_return_code_t db_get(db_get_params_t * param) {
    reg_functions_t & r = get()[param->instance];
    size_t ix = 0;
    size_t mx = r.size();
    db_object_category_types_t cat = DB_OBJ_CAT(param->type);
    db_return_code_t rc = db_ret_code_OK;
    for ( ; ix < mx ; ++ix ) {

        if (r[ix].object_category == cat) {
            rc = r[ix].db_read_function(r[ix].context,param);
            if (rc!=db_ret_code_OK) break;
        }
    }
    return rc;
}

db_return_code_t db_commit(db_transaction_params_t * param) {
    reg_functions_t & r = get()[param->instance];
    db_return_code_t rc = db_ret_code_OK;
    size_t len  = db_list_get_len(param->list);
    size_t s_ix = 0;
    for ( ; s_ix < len ;++s_ix ) {
        db_list_entry_t *l = db_list_elem_get(param->list,s_ix);
        size_t ix = 0;
        size_t mx = r.size();
        db_object_category_types_t oc = DB_OBJ_CAT(l->type);
        for ( ; ix < mx ; ++ix ) {
            if (r[ix].object_category == oc) {
                rc = r[ix].db_write_function(r[ix].context,l);
                if (rc!=db_ret_code_OK) {
                    /** @TODO implement rollback if any db write fails ... */
                    break;
                }
            }
        }
    }
    return rc;
}

db_return_code_t db_get_request_init(db_get_params_t *req, db_instance_t db_type, db_object_type_t objtype) {
    memset(req,0,sizeof(*req));
    req->instance = db_type;
    req->type = objtype;
    req->list = DB_LIST_ALLOC;
    if (req->list==NULL) return db_ret_code_ERR;
    req->keys = DB_LIST_ALLOC;
    if (req->keys==NULL) {
        db_list_destroy(req->list);
        return db_ret_code_ERR;
    }

    return db_ret_code_OK;
}
db_return_code_t db_get_request_close(db_get_params_t *req) {
    if (req->list!=NULL) db_list_destroy(req->list);
    if (req->keys!=NULL) db_list_destroy(req->keys);
    return db_ret_code_OK;
}

db_return_code_t db_transaction_init(db_transaction_params_t *req, db_instance_t db_type) {
    memset(req,0,sizeof(*req));
    req->list = DB_LIST_ALLOC;
    if (req->list==NULL) return db_ret_code_ERR;
    req->instance = db_type;
    return db_ret_code_OK;
}

db_return_code_t db_transaction_close(db_transaction_params_t *req) {
    if (req->list!=NULL) db_list_destroy(req->list);
    return db_ret_code_OK;
}

db_return_code_t db_get_add_key(db_get_params_t * param,db_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    if (!db_list_elem_add(param->keys,type,data,len,deep_copy)) {
        return db_ret_code_ERR;
    }
    return db_ret_code_OK;
}

static db_return_code_t db_tran_op_append(db_transaction_params_t * param,db_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    if (!db_list_elem_add(param->list,type,data,len,deep_copy)) {
        return db_ret_code_ERR;
    }
    return db_ret_code_OK;
}


db_operation_types_t db_object_type_operation(db_object_type_t obj)  {
    return  (db_operation_types_t) ((obj & (DB_CAT_RES_MASK))>>(DB_CAT_CAT_WID+DB_CAT_OBJ_WID));
}

db_return_code_t db_set(db_transaction_params_t * trans,db_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return db_tran_op_append(trans,
            db_object_type_operation_add(type,db_oper_SET),data,len,deep_copy);
}

db_return_code_t db_create(db_transaction_params_t * trans,db_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return db_tran_op_append(trans,
            db_object_type_operation_add(type,db_oper_CREATE),data,len,deep_copy);
}

db_return_code_t db_delete(db_transaction_params_t * trans,db_object_type_t type,
        void *data, size_t len, bool deep_copy) {
    return db_tran_op_append(trans,
            db_object_type_operation_add(type,db_oper_DELETE),data,len,deep_copy);
}

}
