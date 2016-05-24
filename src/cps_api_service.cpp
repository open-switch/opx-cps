/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * cps_api_service.cpp
 */

#include "cps_api_service.h"
#include "cps_class_map.h"
#include "std_event_service.h"
#include "event_log.h"
#include "private/cps_ns.h"

#include <string>
#include <unistd.h>

int main(int argc, char**argv) {
    //Preload the class meta data before the startup of the nameserver process
    cps_api_class_map_init();

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
