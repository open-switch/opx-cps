/*
 * filename: hal_event_service.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */

#include "cps_api_event_init.h"
#include "std_event_service.h"
#include "cps_api_operation.h"

#include <stdlib.h>

static std_event_server_handle _handle=NULL;

static inline std_event_client_handle handle_to_std_handle(cps_api_event_service_handle_t handle) {
    return *(std_event_client_handle*)(&handle);
}

/**
 * The HAL event service path passed to the standard event service API
 */
#define HAL_EVENT_SERVICE_PATH "/tmp/hal_event_service"

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {
    return std_server_client_connect((std_event_client_handle*)handle,HAL_EVENT_SERVICE_PATH) == STD_ERR_OK ?
            cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

static cps_api_return_code_t _cps_api_event_service_client_register(cps_api_event_service_handle_t handle,
        cps_api_event_reg_t * req) {
    size_t ix = 0;
    size_t mx = req->number_of_objects;

    std_event_srv_reg_msg_t m;
    memset(&m,0,sizeof(m));
    for ( ; ix < mx ; ++ix ) {
        if (!cps_api_key_valid_offset(&req->objects[ix],CPS_OBJ_KEY_CAT_POS)) {
            return cps_api_ret_code_ERR;
        }
        std_event_enable_class(&m.classes,
                cps_api_key_element_at(&req->objects[ix],CPS_OBJ_KEY_CAT_POS));
    }
    if (std_client_register_interest(handle_to_std_handle(handle),&m)!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {
    std_event_msg_t m;
    if (!cps_api_key_valid_offset(cps_api_object_key(msg),CPS_OBJ_KEY_SUBCAT_POS)) {
        return cps_api_ret_code_ERR;
    }

    m.event_class = cps_api_key_element_at(cps_api_object_key(msg),CPS_OBJ_KEY_CAT_POS);
    m.sub_class = cps_api_key_element_at(cps_api_object_key(msg),CPS_OBJ_KEY_SUBCAT_POS);
    m.data_len = cps_api_object_to_array_len(msg);
    m.data = cps_api_object_array(msg);
    return std_client_publish_msg(handle_to_std_handle(handle),&m) == STD_ERR_OK ?
            cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

static cps_api_return_code_t _cps_api_event_service_client_deregister(cps_api_event_service_handle_t handle) {
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_wait_for_event(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    std_event_msg_t m;
    m.max_data_len = cps_api_object_get_reserve_len(msg);
    m.data = cps_api_object_array(msg);
    cps_api_return_code_t cps_rc = cps_api_ret_code_ERR;
    t_std_error rc = std_client_wait_for_event(handle_to_std_handle(handle),&m);
    if (rc==STD_ERR_OK) {
        if (cps_api_object_received(msg,m.data_len)) {
            cps_rc = cps_api_ret_code_OK;
        }
    }
    return cps_rc;
}



static cps_api_event_methods_reg_t functions = {
    .connect_function =_cps_api_event_service_client_connect,
    .register_function =_cps_api_event_service_client_register,
    .publish_function = _cps_api_event_service_publish_msg,
    .deregister_function = _cps_api_event_service_client_deregister,
    .wait_for_event_function = _cps_api_wait_for_event
};


cps_api_return_code_t cps_api_event_service_init(void) {
    if (std_event_server_init(&_handle,HAL_EVENT_SERVICE_PATH)==STD_ERR_OK) {
        cps_api_event_method_register(&functions);
        return cps_api_ret_code_OK;
    }
    return cps_api_ret_code_ERR;
}
