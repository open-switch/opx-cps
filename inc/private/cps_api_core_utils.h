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


#ifndef CPS_API_INC_PRIVATE_CPS_API_CORE_UTILS_H_
#define CPS_API_INC_PRIVATE_CPS_API_CORE_UTILS_H_

#include "cps_api_object.h"

bool cps_api_core_publish(cps_api_object_t obj);

void cps_api_event_stats();

#endif /* CPS_API_INC_PRIVATE_CPS_API_CORE_UTILS_H_ */
