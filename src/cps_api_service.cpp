/* OPENSOURCELICENSE */
/*
 * cps_api_service.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "cps_api_service.h"
#include "std_event_service.h"
#include "event_log.h"
#include "private/cps_ns.h"
#include <unistd.h>

static std_event_server_handle_t _handle=NULL;

cps_api_return_code_t cps_api_services_start() {
    if (std_event_server_init(&_handle,CPS_API_EVENT_CHANNEL_NAME,CPS_API_EVENT_THREADS )!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

int main(int argc, char**argv) {
    if (cps_api_services_start()!=cps_api_ret_code_OK) {
        EV_LOG(ERR,DSAPI,0,"FLT","Failed to initialize the messaging service.");
        return cps_api_ret_code_ERR;
    }
    if (cps_api_ns_startup()!=cps_api_ret_code_OK) {
        return cps_api_ret_code_ERR;
    }
    while (true) sleep(1);

    return 0;
}
