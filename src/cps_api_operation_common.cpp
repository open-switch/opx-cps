/**
 * filename: cps_api_operation_common.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/* OPENSOURCELICENSE */


#include "cps_api_operation.h"
#include "std_mutex_lock.h"
#include "std_assert.h"
#include "std_rw_lock.h"
#include "event_log.h"
#include "cps_api_event_init.h"

#include "private/cps_api_client_utils.h"
#include "private/cps_ns.h"

#include <algorithm>
#include <vector>
#include <string.h>
#include <stdarg.h>
#include <memory>



typedef std::vector<cps_api_object_category_types_t> processed_objs_t;

extern "C" {

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

cps_api_return_code_t cps_api_get(cps_api_get_params_t * param) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;

    cps_api_get_params_t new_req;
    if (cps_api_get_request_init(&new_req)!=cps_api_ret_code_OK) {
        return rc;
    }
    cps_api_get_request_guard rg(&new_req);

    if (param->filters!=NULL) {
        cps_api_object_list_swap(param->filters,new_req.filters);
    }

    size_t filter_len = cps_api_object_list_size(new_req.filters);


    std::unique_ptr<cps_api_key_t[]> keys(new cps_api_key_t[param->key_count + filter_len]);
    if (keys.get()==NULL) return cps_api_ret_code_ERR;

    size_t ix = 0;

    for ( ; ix < filter_len ; ++ix ) {
        cps_api_key_copy(&keys.get()[ix],cps_api_object_key(cps_api_object_list_get(new_req.filters,0)));
    }
    ix=0;
    size_t mx = param->key_count;
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_create();
        if (o==NULL) return cps_api_ret_code_ERR;

        if (!cps_api_object_list_append(new_req.filters,o)){
            cps_api_object_delete(o);
            return cps_api_ret_code_ERR;
        }

        cps_api_key_copy(cps_api_object_key(o),param->keys+ix);
        cps_api_key_copy(&keys.get()[ix+filter_len],cps_api_object_key(o));

    }
    new_req.key_count = param->key_count + filter_len;
    new_req.keys = keys.get();

    ix = 0;
    mx = new_req.key_count;

    for ( ; ix < mx ; ++ix ) {
        if ((rc=cps_api_process_get_request(&new_req,ix))!=cps_api_ret_code_OK) break;
    }
    cps_api_object_list_swap(param->list,new_req.list);

    return rc;
}

cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param) {
    cps_api_return_code_t rc =cps_api_ret_code_OK;

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(param->change_list);
    for ( ; ix < mx ; ++ix ) {
        if ((rc=cps_api_process_commit_request(param,ix))!=cps_api_ret_code_OK) {
            break;
        }
    }
    if (rc!=cps_api_ret_code_OK) {
        mx = ix;
        for (ix = 0 ; ix < mx ; ++ix ) {
            if (cps_api_process_rollback_request(param,ix)!=cps_api_ret_code_OK) {
                EV_LOG(ERR,DSAPI,0,"ROLLBACK","Failed to rollback request at %d",ix);
            }
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

}
