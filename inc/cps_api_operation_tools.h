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



#ifndef CPS_API_INC_CPS_API_OPERATION_TOOLS_H_
#define CPS_API_INC_CPS_API_OPERATION_TOOLS_H_

/**
 * @addtogroup CPSAPI
 * @{
 * @addtogroup Operation Operation (Get/Set) and Operation Wrapper Functions
 *
 * @{
*/

#include "cps_api_operation.h"
#include "cps_api_object.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    cps_api_make_change,
    cps_api_no_change,
}cps_api_dbchange_t;


typedef enum {
    cps_api_raise_event,
    cps_api_raise_no_event,
}cps_api_dbchange_notify_t;

typedef enum {
    cps_api_db_no_connection,
}cps_api_db_errorcode_t;

struct cps_api_db_sync_cb_param_t {
    cps_api_operation_types_t opcode;    // the operation type (set,delete,create)
    cps_api_object_t object_dest;
    cps_api_object_t object_src;
    const char *src_node;
    const char *dest_node;
};

struct cps_api_db_sync_cb_response_t {
    cps_api_dbchange_t change;            // enumerator that indicates whether to make changes to DB or not
    cps_api_dbchange_notify_t change_notify;  // enumerator that indicates whether to publish events in case of DB changes or not
};


struct cps_api_db_sync_cb_error_t {
    cps_api_db_errorcode_t err_code;
};

/**
* The callback function used with CPS DB Sync operation
* @param context Application specific context from the sync API
* @param params the structure to initialize
* @param res the response from application
* @return must return true otherwise processing will be stopped
*/

typedef bool (*cps_api_sync_callback_t)(void *context, cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_response_t *res);

/**
* The error callback function used with CPS DB Sync operation
* @param context Application specific context from the sync API
* @param params the structure to initialize
* @param err structure with error information
* @return true on success otherwise false (processing will be stopped)
*/
typedef bool (*cps_api_sync_error_callback_t)(void *context, cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_error_t *err);


/**
* Sync CPS Object between nodes. This API goes through the destination DB indicated by the contents of the dest object and merges them with the source object fetched from the source DB.
* @param context Application specific context to pass to the sync API
* @param dest cps object that contains CPS_OBJECT_GROUP_NODE attribute indicating destination node name
* @param src cps object that needs to be synced with dest object. Contains CPS_OBJECT_GROUP_NODE attribute indicating source node name and CPS class key
* @param cb sync callback function. This is a single threaded function and will be called in the context of the application. The callback function will be invoked in 3 cases. 
         CREATE - where the source object exist and the dest object doesn't exist
         SET - where there are differences in attribute values between the source and dest objects
         DELETE - where the specific instance of the source object doesn't exist and the dest object exists
* @param err_cb error callback function. Will be invoked if a successful connection to the source DB can't be established
* @return returns code cps_api_ret_code_OK if successful otherwise an error
*/

cps_api_return_code_t cps_api_sync(void *context, cps_api_object_t dest, cps_api_object_t src,  cps_api_sync_callback_t cb, cps_api_sync_error_callback_t err_cb);

/**
 * Update, Delete, Create a single object.  This API hides all of the transaction details and also allows
 * the user to specify a retry count.
 *
 * @param type type of operation - see cps_api_operation.h
 * @param obj the object to update/create/delete/rpc call - it is up to the caller to free the object after use
 * @param retry_count the number of attempts to try before giving up - 0 means try for ever
 * @param ms_delay_between the amount of time in ms to wait before retrying - a value of 0 will use the default retry timeout
 * @return cps_api_ret_code_OK on success otherwise a failure condition
 */
cps_api_return_code_t cps_api_commit_one(cps_api_operation_types_t type, cps_api_object_t obj, size_t retry_count, size_t ms_delay_between);

/**
 * Get a one type of object and store it in the provided object list - it is up to the caller of the function to
 * dealloc the list with the list cleanup function.
 *
 * @param filt - the object that contains the attributes for which to filter the obj_list response
 * @param obj_list the returned list of objects - this list will only be appended to so it will not clear out any
 *     existing object before returning
 * @param retry_count the number of retry attempts before giving up - a value of 0 will wait for ever
 * @param ms_delay_between the amount of time in ms to wait before retrying - a value of 0 will use the default retry timeout
 * @return cps_api_ret_code_OK on success otherwise a failure
 */
cps_api_return_code_t cps_api_get_objs(cps_api_object_t filt, cps_api_object_list_t obj_list, size_t retry_count, size_t ms_delay_between);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 */

#endif /* CPS_API_INC_CPS_API_OPERATION_TOOLS_H_ */
