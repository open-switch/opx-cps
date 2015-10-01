/* OPENSOURCELICENSE */
/*
 * cps_api_client_utils.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_CLIENT_UTILS_H_
#define CPS_API_CLIENT_UTILS_H_

#include "cps_api_errors.h"
#include "cps_api_operation.h"
#include "private/cps_ns.h"
#include <stddef.h>

typedef int cps_api_channel_t;

enum cps_api_msg_operation_t {
    cps_api_msg_o_GET,
    cps_api_msg_o_GET_RESP,
    cps_api_msg_o_GET_DONE,
    cps_api_msg_o_COMMIT,
    cps_api_msg_o_COMMIT_OBJECT,
    cps_api_msg_o_RETURN_CODE,
    cps_api_msg_o_REVERT,
    cps_api_msg_o_COMMIT_CHANGE,
    cps_api_msg_o_COMMIT_PREV,
    cps_api_msg_o_STATS,

    cps_api_ns_r_ADD,
    cps_api_ns_r_DEL,
    cps_api_ns_r_QUERY,
    cps_api_ns_r_QUERY_RESULTS,
    cps_api_ns_r_RETURN_CODE,
};


cps_api_return_code_t cps_api_process_get_request(cps_api_get_params_t *param, size_t ix);
cps_api_return_code_t cps_api_process_commit_request(cps_api_transaction_params_t *param, size_t ix);
cps_api_return_code_t cps_api_process_rollback_request(cps_api_transaction_params_t *param, size_t ix);


bool cps_api_send(cps_api_channel_t handle, uint32_t op,
        const struct iovec *iov,
        size_t count);

bool cps_api_send_header(cps_api_channel_t handle, uint32_t op,
        size_t len);

bool cps_api_send_data(cps_api_channel_t handle, void *data, size_t len);
bool cps_api_send_key(cps_api_channel_t handle, cps_api_key_t &key) ;
bool cps_api_send_object(cps_api_channel_t handle, cps_api_object_t obj);

bool cps_api_receive_header(cps_api_channel_t handle, uint32_t &op,
        size_t &len) ;
bool cps_api_receive_data(cps_api_channel_t handle, void *data, size_t len) ;
bool cps_api_receive_key(cps_api_channel_t handle, cps_api_key_t &key);
cps_api_object_t cps_api_receive_object(cps_api_channel_t handle,size_t len) ;

bool cps_api_find_owners(cps_api_key_t *key, cps_api_object_owner_reg_t &owner);
void cps_api_disconnect_owner(cps_api_channel_t handle);

bool cps_api_ns_register(cps_api_channel_t handle, cps_api_object_owner_reg_t &reg);
bool cps_api_ns_create_handle(cps_api_channel_t *handle);

static inline bool cps_api_send_key_request(cps_api_channel_t handle, uint32_t op,
        cps_api_key_t &key) {
    if (cps_api_send_header(handle,op,sizeof(key))) {
        return cps_api_send_data(handle,&key,sizeof(key));
    }
    return false;
}
static inline bool cps_api_send_return_code(cps_api_channel_t handle,uint32_t op,
        cps_api_return_code_t rc) {
    if (cps_api_send_header(handle,op,sizeof(rc))) {
        return cps_api_send_data(handle,&rc,sizeof(rc));
    }
    return false;
}

static inline bool cps_api_send_one_object(cps_api_channel_t handle, cps_api_msg_operation_t op,
        cps_api_object_t obj) {
    size_t len = obj!=NULL ? cps_api_object_to_array_len(obj) : 0;
    if (cps_api_send_header(handle,op,len)) {
        return len > 0 ? cps_api_send_object(handle,obj) : true;
    }
    return false;
}

class cps_api_channel_handle_guard {
    cps_api_channel_t handle;
public:
    cps_api_channel_handle_guard(cps_api_channel_t h) : handle(h) {}
    ~cps_api_channel_handle_guard() {
        if (handle!=-1) cps_api_disconnect_owner(handle);
    }
    cps_api_channel_t get() { return handle; }
};

#endif /* CPS_API_CLIENT_UTILS_H_ */
