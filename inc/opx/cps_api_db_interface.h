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

#ifndef CPS_API_INC_CPS_API_DB_INTERFACE_
#define CPS_API_INC_CPS_API_DB_INTERFACE_

#include "cps_api_object.h"
#include "cps_api_operation.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cps_api_db_commit_bulk_s {
    cps_api_operation_types_t op;    //the operation type (set,delete,create)
    cps_api_object_list_t objects;     //the list of objects to be modified or deleted/stored
    bool publish;    //true if the API will be required to publish change via cps event service
    const char *node_group;
}cps_api_db_commit_bulk_t;

/**
 * Initialize the bulk commit structure
 * @param the structure to initialize
 * @return true if successful otherwise false if out of memory
 */
bool cps_api_db_commit_bulk_init(cps_api_db_commit_bulk_t*);

/**
 * Clean up the objects contained within the bulk operation including all objects in the object list
 * @param the strucutre to clean up
 */
void cps_api_db_commit_bulk_close(cps_api_db_commit_bulk_t*);

/**
 * This function will take a object and attempt to perform the requested operation.
 * The function will attempt to update all objects.  If the communication to the DB fails or
 * some objects are not committed successfully, a error code will be returned and the objects will have an
 * error status that can be queried
 *
 * @param param the db commit params
 * @return a return code that will enther indicate failure in performing the operation or
 *             cps_api_ret_code_OK on success
 */
cps_api_return_code_t cps_api_db_commit_bulk(cps_api_db_commit_bulk_t *param);

/**
 * This API will take the existing object, look for the group ID and operation type and then optionally publish
 * the response.
 *
 * @param obj to update/create/delete
 * @param publish true if the object change should be published
 * @param prev the previous object that was in the DB before this operation (pass NULL for slightly more optimized)
 * @return cps_api_ret_code_OK on success otherwise an error codition.
 */
cps_api_return_code_t cps_api_db_commit_one(cps_api_operation_types_t op,cps_api_object_t obj, cps_api_object_t prev, bool publish);


/**
 * On the provided object, set the connection flag on the filter.
 * @param obj the object that will have the cps/connection-entry/connection-state flag set
 * @return true if successful otherwise false
 */
bool cps_api_db_get_filter_enable_connection(cps_api_object_t obj) ;

/**
 * This API will attempt to get the specified patterns from the database.  The API will return a list of
 * items in the found API.  If the database has no items matching a filter, no objects will be returned.
 *
 * It is possible that there may be a connection issue to one of the DBs specifieid by the filter's group
 * and in that case, if the object has the cps/connection-entry/connection-state flag enabled
 * then a connection object will be returned for each DB that wasn't acceptiong requests.
 *
 * @param filters this is a list of objects
 * @param found this is the returned list of objects.  The API will append the result to this list
 * @return either a cps_api_ret_code_ERR due to a parameter issue or the fact that the DB was not accessable
 *             or cps_api_ret_code_OK and a series of objects.
 */
cps_api_return_code_t cps_api_db_get(cps_api_object_t obj,cps_api_object_list_t found);


/**
 * Given the list of objects (containing instance keys) query the database for the exact matches.  This API
 * doesn't support wildcard queries and therefore is able to save some overhead.
 * @param objs the object list containing elements to query
 * @param node_group can be left NULL - contains the node group for the get
 * @return true on success otherwise false
 */
cps_api_return_code_t cps_api_db_get_bulk(cps_api_object_list_t objs, const char * node_group);


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
    cps_api_db_invalid_address,
}cps_api_db_errorcode_t;

typedef struct {
    cps_api_operation_types_t opcode;    // the operation type (set,delete,create)
    cps_api_object_t object_dest;
    cps_api_object_t object_src;
    const char *src_node;
    const char *dest_node;
}cps_api_db_sync_cb_param_t;

typedef struct {
    cps_api_dbchange_t change;            // enumerator that indicates whether to make changes to DB or not
    cps_api_dbchange_notify_t change_notify;  // enumerator that indicates whether to publish events in case of DB changes or not
}cps_api_db_sync_cb_response_t;


typedef struct {
    cps_api_db_errorcode_t err_code;
}cps_api_db_sync_cb_error_t;

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
* This API goes through the destination DB indicated by the contents of CPS object and merges them with the source objects.
* @param context Application specific context to pass to the reconcile API
* @param src_objs list of CPS objects that needs to be compared with destination object
* @param dest_obj destination CPS object that will be compared with the source objs
* @param cb sync callback function. This is a single threaded function and will be called in the context of the application. The callback function will be invoked in 3 cases.
         CREATE - where the source object exist and the dest object doesn't exist
         SET - where there are differences in attribute values between the source and dest objects
         DELETE - where the specific instance of the source object doesn't exist and the dest object exists
* @param err_cb error callback function. Will be invoked if a successful connection to the destination DB can't be established
* @return returns code cps_api_ret_code_OK if successful otherwise an error
*/
cps_api_return_code_t cps_api_reconcile(void *context, cps_api_object_list_t src_objs,  cps_api_object_t dest_obj, cps_api_sync_callback_t cb, cps_api_sync_error_callback_t err_cb);



#ifdef __cplusplus
} //end extern "C"

class cps_api_db_commit_bulk_guard {
    cps_api_db_commit_bulk_t *_entry;
public:
    cps_api_db_commit_bulk_guard(cps_api_db_commit_bulk_t *r) : _entry(r){}
    ~cps_api_db_commit_bulk_guard() { cps_api_db_commit_bulk_close(_entry); }
};
#endif

#endif /* CPS_API_INC_CPS_API_DB_INTERFACE_ */
