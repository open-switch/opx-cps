/* OPENSOURCELICENSE */
/*
 * cps_api_operation_service.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "private/cps_ns.h"
#include "private/cps_api_client_utils.h"

#include "cps_api_operation.h"
#include "cps_api_object.h"
#include "std_socket_service.h"
#include "std_rw_lock.h"
#include "std_mutex_lock.h"
#include "event_log.h"
#include "std_file_utils.h"

#include <unistd.h>
#include <vector>


typedef std::vector<cps_api_registration_functions_t> reg_functions_t;

struct cps_api_operation_data_t {
    std_socket_server_handle_t handle;
    std_socket_server_t service_data;
    std_rw_lock_t db_lock;
    std_mutex_type_t mutex;
    reg_functions_t db_functions;
    cps_api_channel_t ns_handle;
};

static bool  cps_api_handle_get(cps_api_operation_data_t *op, int fd,size_t len) {

    cps_api_key_t key;
    if (!cps_api_receive_key(fd,key)) return false;

    cps_api_get_params_t param;
    if (cps_api_get_request_init(&param)!=cps_api_ret_code_OK) return false;
    cps_api_get_request_guard grg(&param);

    std_rw_lock_read_guard g(&op->db_lock);

    param.key_count =1;
    param.keys = &key;

    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();

    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_read_function!=NULL) &&
                (cps_api_key_matches(&key,&p->key,false)==0)) {
            rc = p->_read_function(p->context,&param,0);
            if (rc!=cps_api_ret_code_OK) break;
        }
    }

    if (rc!=cps_api_ret_code_OK) {
        return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
    } else {
        //send all objects in the list
        size_t ix = 0;
        size_t mx = cps_api_object_list_size(param.list);
        for ( ; (ix < mx) ; ++ix ) {
            cps_api_object_t obj = cps_api_object_list_get(param.list,ix);
            if (obj==NULL) continue;
            if (!cps_api_send_one_object(fd,cps_api_msg_o_GET_RESP,obj)) return false;
        }
        if (!cps_api_send_header(fd,cps_api_msg_o_GET_DONE,0)) return false;
    }

    return true;
}


static bool cps_api_handle_commit(cps_api_operation_data_t *op, int fd, size_t len) {
    std_rw_lock_read_guard g(&op->db_lock);

    cps_api_return_code_t rc =cps_api_ret_code_OK;

    cps_api_object_guard o(cps_api_receive_object(fd,len));
    if (!o.valid()) return false;

    cps_api_transaction_params_t param;
    if (cps_api_transaction_init(&param)!=cps_api_ret_code_OK) return false;
    cps_api_transaction_guard tr(&param);

    if (!cps_api_object_list_append(param.change_list,o.get())) return false;

    cps_api_object_t l = o.release();

    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();
    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_write_function!=NULL) &&
                (cps_api_key_matches(cps_api_object_key(l),&p->key,false)==0)) {
            rc = p->_write_function(p->context,&param,0);
            if (rc!=cps_api_ret_code_OK) break;
        }
    }

    if (rc!=cps_api_ret_code_OK) {
        return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
    } else {
        cps_api_object_t obj = cps_api_object_list_get(param.prev,0);
        if (obj==NULL) return false;

        if (!cps_api_send_one_object(fd,cps_api_msg_o_COMMIT_OBJECT,obj)) return false;
    }

    return true;
}

static bool cps_api_handle_revert(cps_api_operation_data_t *op, int fd, size_t len) {
    std_rw_lock_read_guard g(&op->db_lock);

    cps_api_return_code_t rc =cps_api_ret_code_OK;

    cps_api_object_guard o(cps_api_receive_object(fd,len));
    if (!o.valid()) return false;

    cps_api_transaction_params_t param;
    if (cps_api_transaction_init(&param)!=cps_api_ret_code_OK) return false;
    cps_api_transaction_guard tr(&param);

    if (!cps_api_object_list_append(param.prev,o.get())) return false;

    cps_api_object_t l = o.release();

    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();
    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_rollback_function!=NULL) &&
                (cps_api_key_matches(cps_api_object_key(l),&p->key,false)==0)) {
            rc = p->_rollback_function(p->context,&param,0);
            if (rc!=cps_api_ret_code_OK) break;
        }
    }

    return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
}


static bool  _some_data_( void *context, int fd ) {
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)context;

    uint32_t op;
    size_t len;
    if(!cps_api_receive_header(fd,op,len)) return false;
    if (op==cps_api_msg_o_GET) return cps_api_handle_get(p,fd,len);
    if (op==cps_api_msg_o_COMMIT) return cps_api_handle_commit(p,fd,len);
    if (op==cps_api_msg_o_REVERT) return cps_api_handle_revert(p,fd,len);

    return true;
}

static bool register_one_key(cps_api_operation_data_t *p, cps_api_key_t &key) {
    cps_api_object_owner_reg_t r;
    r.addr = p->service_data.address;
    memcpy(&r.key,&key,sizeof(r.key));
    if (!cps_api_ns_register(p->ns_handle,r)) {
        close(p->ns_handle);
        return false;
    }
    return true;
}
static bool reconnect_with_ns(cps_api_operation_data_t *data) {
    if (!cps_api_ns_create_handle(&data->ns_handle)) {
        data->ns_handle = STD_INVALID_FD;
        return false;
    }
    if (std_socket_service_client_add(data->handle,data->ns_handle)!=STD_ERR_OK) {
        close(data->ns_handle);
        return false;
    }
    size_t ix = 0;
    size_t mx = data->db_functions.size();
    for ( ; ix < mx ; ++ix ) {
        if(!register_one_key(data,data->db_functions[ix].key)) {
            close(data->ns_handle); data->ns_handle=STD_INVALID_FD;
            return false;
        }
    }
    return true;
}

cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg) {
    STD_ASSERT(reg->handle!=NULL);
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)reg->handle;
    std_rw_lock_write_guard g(&p->db_lock);

    if (p->ns_handle==STD_INVALID_FD) {
        reconnect_with_ns(p);
    }
    p->db_functions.push_back(*reg);

    if (p->ns_handle!=STD_INVALID_FD) {
        if (!register_one_key(p,reg->key)) {
            close(p->ns_handle);
            p->ns_handle = STD_INVALID_FD;
        }
    }
    return cps_api_ret_code_OK;
}


static void _timedout(void * context) {
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)context;
    std_rw_lock_write_guard g(&p->db_lock);
    if (p->ns_handle==STD_INVALID_FD) {
        reconnect_with_ns(p);
    }
}

cps_api_return_code_t cps_api_operation_subsystem_init(
        cps_api_operation_handle_t *handle, size_t number_of_threads) {

    cps_api_operation_data_t *p = new cps_api_operation_data_t;
    if (p==NULL) return cps_api_ret_code_ERR;

    p->handle = NULL;
    memset(&p->service_data,0,sizeof(p->service_data));
    std_mutex_lock_init_recursive(&p->mutex);

    std_rw_lock_create_default(&p->db_lock);

    p->service_data.name = "CPS_API_instance";
    cps_api_create_process_address(&p->service_data.address);
    p->service_data.thread_pool_size = number_of_threads;
    p->service_data.some_data = _some_data_;
    p->service_data.timeout = _timedout;

    p->service_data.context = p;

    if (std_socket_service_init(&p->handle,&p->service_data)!=STD_ERR_OK) {
        delete p;
        return cps_api_ret_code_ERR;
    }

    if (std_socket_service_run(p->handle)!=STD_ERR_OK) {
        std_socket_service_destroy(p->handle);
        delete p;
        return cps_api_ret_code_ERR;
    }

    reconnect_with_ns(p);

    *handle = p;

    return cps_api_ret_code_OK;
}

