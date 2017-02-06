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

/**
 * @file cps_api_db_interface.h
 */

#ifndef CPS_API_INC_CPS_API_DB_INTERFACE_
#define CPS_API_INC_CPS_API_DB_INTERFACE_

#include "cps_api_object.h"
#include "cps_api_operation.h"

#ifdef __cplusplus
extern "C" {
#endif

struct cps_api_db_commit_bulk_t {
    cps_api_operation_types_t op;    //the operation type (set,delete,create)
    cps_api_object_list_t objects;     //the list of objects to be modified or deleted/stored
    bool publish;    //true if the API will be required to publish change via cps event service
    const char *node_group;
};

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
