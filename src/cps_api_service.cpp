/* OPENSOURCELICENSE */
/*
 * cps_api_service.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "cps_api_service.h"
#include "std_event_service.h"

#include <unistd.h>

static std_event_server_handle_t _handle=NULL;

cps_api_return_code_t cps_api_services_start() {
    if (std_event_server_init(&_handle,CPS_API_EVENT_CHANNEL_NAME,CPS_API_EVENT_THREADS )!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

int main(int argc, char**argv) {

    while (true) sleep(1);

    return 0;
}
