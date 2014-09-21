/*
 * filename: hal_event_service.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */

#include "ds_event_api.h"
#include <stdlib.h>

static std_event_server_handle handle=NULL;

/**
 * The HAL event service path passed to the standard event service API
 */
#define HAL_EVENT_SERVICE_PATH "/tmp/hal_event_service"

t_std_error ds_event_service_init(void) {
    return std_event_server_init(&handle,HAL_EVENT_SERVICE_PATH);
}

t_std_error ds_events_register_cb(std_event_srv_reg_t *reg) {
    return std_server_events_register_cb(handle,reg);
}

t_std_error ds_event_service_client_connect(std_event_client_handle * handle) {
    return std_server_client_connect(handle,HAL_EVENT_SERVICE_PATH);
}

t_std_error ds_event_service_publish_direct_event( std_event_msg_t *msg ) {
    return std_server_publish_msg(handle,msg);
}
