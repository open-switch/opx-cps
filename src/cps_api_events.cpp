/* OPENSOURCELICENSE */
/*
 * cps_api_event_channel.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "cps_api_event_init.h"
#include "std_mutex_lock.h"
#include "cps_api_events.h"

#include <stdio.h>
#include <stdlib.h>


static std_mutex_lock_create_static_init_rec(mutex);
static cps_api_event_methods_reg_t m_method;

extern "C" {

cps_api_return_code_t cps_api_event_method_register( cps_api_event_methods_reg_t * what )  {
    std_mutex_simple_lock_guard l(&mutex);
    m_method = *what;
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_event_init(void) {
    return cps_api_ret_code_OK;
}

cps_api_event_header_t * cps_api_event_allocate(unsigned int space) {
    cps_api_event_header_t *p = (cps_api_event_header_t*)malloc(sizeof(cps_api_event_header_t)+space);
    if (p==NULL) return NULL;
    memset(p,0,sizeof(*p));
    p->max_data_len = space;
    p->data_len = 0;
    return p;
}

void cps_api_event_free(cps_api_event_header_t *msg) {
    free(msg);
}

const char * cps_api_event_print(cps_api_event_header_t *msg, char *buf,size_t len) {
    char dbg_key[200];
    snprintf(buf,len,    "Max Size: %d\n"
            "Size :%d\n"
        "key : %s\n",
       msg->max_data_len,msg->data_len,
       cps_api_key_print(&msg->key,dbg_key,sizeof(dbg_key)));
    return buf;
}

cps_api_return_code_t cps_api_event_client_connect(cps_api_event_service_handle_t * handle) {
    return m_method.connect_function(handle);
}

cps_api_return_code_t cps_api_event_client_register(cps_api_event_service_handle_t *handle,
        cps_api_event_reg_t * req) {
    return m_method.register_function(handle,req);
}

cps_api_return_code_t cps_api_event_publish(cps_api_event_service_handle_t handle,
        cps_api_event_header_t *msg) {
    return m_method.publish_function(handle,msg);
}

cps_api_return_code_t cps_api_event_client_disconnect(cps_api_event_service_handle_t handle) {
    return m_method.deregister_function(handle);
}

}
