/*
 * Copyright (c) 2019 Dell Inc.
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

#include "cps_api_db_interface.h"
#include "cps_api_db_interface_tools.h"
#include "cps_api_errors.h"
#include "cps_api_event_init.h"
#include "cps_api_events.h"
#include "cps_api_key.h"
#include "cps_api_node.h"
#include "cps_api_object_attr.h"
#include "cps_api_object_category.h"
#include "cps_api_object.h"
#include "cps_api_object_key.h"
#include "cps_api_object_tools.h"
#include "cps_api_operation_debug.h"
#include "cps_api_operation.h"
#include "cps_api_operation_stats.h"
#include "cps_api_operation_tools.h"
#include "cps_api_service.h"
#include "cps_class_map.h"
#include "cps_key_internals.h"
#include "dell-cps.h"

/*Validate that the C/C++ headers is working properly*/

int main() {
    return 0;
}

