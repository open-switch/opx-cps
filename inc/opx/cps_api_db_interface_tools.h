/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */
#ifndef __CPS_API_DB_INTERFACE_TOOLS_H
#define __CPS_API_DB_INTERFACE_TOOLS_H

#include "cps_api_object.h"
#include "cps_api_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The following function call will take the object and store the object into the DB in either the running, startup or running + startup areas.
 * The DB Config Type (running cfg, running + startup, startup only) present in the object is used to control the destination.
 *
 * If the DB Config Type CPS_CONFIG_TYPE_RUNNING_CONFIG, CPS_CONFIG_TYPE_STARTUP_AND_RUNNING or CPS_CONFIG_TYPE_RUNNING_CONFIG_ONLY the object will
 * be stored under the cps_api_qualifier_RUNNING_CONFIG qualifer in the DB
 *
 * If the DB Config Type CPS_CONFIG_TYPE_STARTUP_CONFIG or CPS_CONFIG_TYPE_STARTUP_AND_RUNNING the object will
 * be stored under the cps_api_qualifier_STARTUP_CONFIG qualifer in the DB
 *
 * @param obj the object is the data to store with the correct DB Config Type set already ( see cps_api_object_set_config_type )
 * @return
 */
cps_api_return_code_t cps_api_db_config_write(cps_api_object_t obj);

#ifdef __cplusplus
}
#endif

#endif

