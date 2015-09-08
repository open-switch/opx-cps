/** OPENSOURCELICENSE */
/*
 * cps_api_operation_tools.h
 *
 *  Created on: Sep 3, 2015
 */

#ifndef CPS_API_INC_CPS_API_OPERATION_TOOLS_H_
#define CPS_API_INC_CPS_API_OPERATION_TOOLS_H_

#include "cps_api_operation.h"
#include "cps_api_object.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Update, Delete, Create a single object.  This API hides all of the transaction details and also allows
 * the user to specify a retry count.
 *
 * @param type the type of operation - see cps_api_operation.h
 * @param obj the object to update/create/delete/rpc call - it is up to the caller to free the object after use
 * @param retry_count the number of attempts to try before giving up - 0 means try for ever
 * @param ms_delay_between the amount of time in ms to wait before retrying
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
 * @param ms_delay_between the amount of time in ms to wait before retrying
 * @return cps_api_ret_code_OK on success otherwise a failure
 */
cps_api_return_code_t cps_api_get_objs(cps_api_object_t filt, cps_api_object_list_t obj_list, size_t retry_count, size_t ms_delay_between);

#ifdef __cplusplus
}
#endif


#endif /* CPS_API_INC_CPS_API_OPERATION_TOOLS_H_ */
