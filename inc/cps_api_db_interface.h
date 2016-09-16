/*
 * cps_api_db_interface.h
 */

#ifndef CPS_API_INC_CPS_API_DB_INTERFACE_
#define CPS_API_INC_CPS_API_DB_INTERFACE_

#include "cps_api_object.h"
#include "cps_api_operation.h"

/**
 * This function will take a object and attempt to perform the requested operation.
 * The function will attempt to update all objects.  If the communication to the DB fails or
 * some objects are not committed successfully, a error code will be returned and the objects will have an
 * error status that can be queried
 *
 * @param object the object to be modified or deleted/stored
 * @param previous the object that existed before this operation
 * @param publish true if the API will be required to publish change via cps event service
 * @return a return code that will enther indicate failure in performing the operation or
 *             cps_api_ret_code_OK on success
 */
cps_api_return_code_t cps_api_db_commit(cps_api_object_t object, cps_api_object_t previous, bool publish);


/**
 * This function will take a series of objects and attempt to perform the requested operation on them.
 * This API will attempt the operation specified on all object regardless of the failure of the previous attempt.
 * The return code for the operation will be contained within the returned object.
 * All *attempts* will be made to make this occur symulatniously but no guarentees
 *
 * @param objects the object to be modified or stored
 * @param publish true if an event should be sent after the update otherwise false
 * @return the return code will either indicate that all is well, and all operations passed (cps_api_ret_code_OK)
 *  or an error returned and each object will need to be checked to see which operation failed.
 */
cps_api_return_code_t cps_api_db_commit_list(cps_api_object_list_t objects, bool publish);

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



#endif /* CPS_API_INC_CPS_API_DB_INTERFACE_ */
