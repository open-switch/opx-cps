/*
 * Copyright (c) 2018 Dell Inc.
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


#ifndef CPS_OPERATION_DEBUG_H_
#define CPS_OPERATION_DEBUG_H_


#include "cps_api_operation.h"

/**
 * Log the get request to the default logging mechanism
 * @param req the request structure
 */
void cps_api_get_request_log(cps_api_get_params_t *req);

/**
 * Log the commit request to the default logging mechanism
 * @param req the request structure
 */
void cps_api_commit_request_log(cps_api_transaction_params_t *req);

/**
 * Take the object and log it - the object line will be prefixed with the "prefix" string
 * @param obj the object to log
 * @param log_prefix a string prefix a log entry with
 */
void cps_api_object_log(cps_api_object_t obj,const char *log_prefix);

/**
 * Take the object list and log it - the object line will be prefixed with the "prefix" string
 * @param obj the object to log
 * @param log_prefix a string prefix a log entry with
 */
void cps_api_object_list_log(cps_api_object_list_t lst,const char *log_prefix);

#endif /* CPS_OPERATION_DEBUG_H_ */
