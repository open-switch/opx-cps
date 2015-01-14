/*
 * filename: hal_event_service.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */

#include "cps_api_event_init.h"
#include "std_event_service.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"

#include <stdlib.h>

#define DEFAULT_DATA_LEN (1000)


typedef struct {
    std_event_client_handle handle;
    std_event_msg_buff_t buff;
} cps_api_to_std_event_map_t;

static inline std_event_client_handle handle_to_std_handle(cps_api_event_service_handle_t handle) {
    cps_api_to_std_event_map_t *p = (cps_api_to_std_event_map_t*) handle;
    return p->handle;
}
static inline std_event_msg_buff_t handle_to_buff(cps_api_event_service_handle_t handle) {
    cps_api_to_std_event_map_t *p = (cps_api_to_std_event_map_t*) handle;
    return p->buff;
}

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {

    cps_api_to_std_event_map_t *p = calloc(1,sizeof(cps_api_to_std_event_map_t));
    if (p==NULL) return cps_api_ret_code_ERR;

    if (std_server_client_connect(&p->handle,
            CPS_API_EVENT_CHANNEL_NAME) == STD_ERR_OK) {
        p->buff = std_client_allocate_msg_buff(DEFAULT_DATA_LEN,false);
        if (p->buff!=NULL) {
            *handle = p;
            return cps_api_ret_code_OK;
        }
        std_server_client_disconnect(p->handle);
    }
    free(p);
    return cps_api_ret_code_ERR;
}

static void cps_api_to_std_key(std_event_key_t *key,cps_api_key_t *cps_key) {
    memcpy(key->event_key,cps_api_key_elem_start(cps_key),
            cps_api_key_get_len(cps_key)* sizeof (key->event_key[0]));
    key->len = cps_api_key_get_len(cps_key);
}

static cps_api_return_code_t _cps_api_event_service_client_register(cps_api_event_service_handle_t handle,
        cps_api_event_reg_t * req) {
    size_t ix = 0;
    size_t mx = req->number_of_objects;
    std_event_key_t key;
    for ( ; ix < mx ; ++ix ) {
        cps_api_to_std_key(&key,&req->objects[ix]);
        if (std_client_register_interest(handle_to_std_handle(handle),
                &key,1)!=STD_ERR_OK) return cps_api_ret_code_ERR;
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    if (!cps_api_key_valid_offset(cps_api_object_key(msg),CPS_OBJ_KEY_SUBCAT_POS)) {
        return cps_api_ret_code_ERR;
    }
    std_event_key_t key;

    cps_api_to_std_key(&key,cps_api_object_key(msg));

    return std_client_publish_msg_data(handle_to_std_handle(handle), &key,
            cps_api_object_array(msg),
            cps_api_object_to_array_len(msg)) == STD_ERR_OK ?
                    cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

static cps_api_return_code_t _cps_api_event_service_client_deregister(cps_api_event_service_handle_t handle) {
    std_event_client_handle h = handle_to_std_handle(handle);
    std_client_free_msg_buff(handle_to_buff(handle));
    free(handle);
    return std_server_client_disconnect(h);

}

static cps_api_return_code_t _cps_api_wait_for_event(
        cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    std_event_msg_t m;
    if (std_client_wait_for_event_data( handle_to_std_handle(handle),
            &m, cps_api_object_array(msg),
            cps_api_object_get_reserve_len(msg))!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

static cps_api_event_methods_reg_t functions = {
    .connect_function =_cps_api_event_service_client_connect,
    .register_function =_cps_api_event_service_client_register,
    .publish_function = _cps_api_event_service_publish_msg,
    .deregister_function = _cps_api_event_service_client_deregister,
    .wait_for_event_function = _cps_api_wait_for_event
};

cps_api_return_code_t cps_api_event_service_init(void) {
    cps_api_event_method_register(&functions);
    return cps_api_ret_code_OK;
}
