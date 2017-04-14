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

/*
 */

#ifndef CPS_API_INC_CPS_API_OBJECT_TOOLS_H_
#define CPS_API_INC_CPS_API_OBJECT_TOOLS_H_

/** @addtogroup CPSAPI
 *  @{
 *  @addtogroup ObjectAndAttributes Object and Object Attribute Handling
 *  @{
*/

#include "cps_api_object_attr.h"
#include "cps_api_object.h"
#include "cps_api_operation.h"
#include "std_error_codes.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
/**
 *  @addtogroup ObjectAttributesDetails Object Attribute Details
 *  These functions are used to manipulate and retrieve object attribute details.
    <p>In order to access the functionality provided by this module, applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_object_tools.h>
 @endverbatim

 *  @{
 */

/**
 * Create an object and set the key of the object to the key of the specified attribute ID
 * @param qual the qualifier of the key (target, observed, etc..
 * @param id the object that contains the key that will be set on the object
 * @param use_create_defaults - this attribute is reserved and must be set to false
 *
 * @return a cps_api_object_t with the key and attributes set appropriately or a NULL on error
 */
cps_api_object_t cps_api_obj_tool_create(cps_api_qualifier_t qual, cps_api_attr_id_t id, bool use_create_defaults);


/**
 * Compare the filter and the specified object.  Return true if all of the attributes in the filter
 *    matches the corresponding attributes in the object.
 * @param filter the filter object containing the attributes/values to look for
 * @param obj the object that as the attributes to compare
 * @param require_all_attribs if this is true, all attributes in the filter must be present in the object and they must
 *  match otherwise only compare found attribute
 *
 * @return true if the filter and object match based on the require_all_attrib setting
 */
bool cps_api_obj_tool_matches_filter(cps_api_object_t filter, cps_api_object_t obj, bool require_all_attribs);

/**
 * Take the contents of the main and copy the attributes from the overlay into main removing any duplicates discovered in main
 * @param current the object that will be updated to have all of the attributes of the overlay
 * @param changes the object attributes that will be added to the "main"
 * @return true if successful otherwise false
 */
bool cps_api_obj_tool_merge(cps_api_object_t current, cps_api_object_t changes);


/**
 * Scan through the object looking for attributes and ensure that the contents match the request items exactly.
 * For instance...
@code{.cpp}
cps_api_attr_id_t ids = { NAME, IFINDEX };
int _ifix = 1;
void *values[] = { "Cliff", &_ifix };
size_t lens[] = { strlen("Cliff"),sizeof(int) };

cps_api_obj_tool_attr_matches(obj,ids,values,lens,sizeof(ids)/sizeof(*ids));
@endcode
 *
 * @param obj the object to check for the values
 * @param ids the list of attribute IDs
 * @param values the list of values
 * @param len the list of the length of the content in the values array
 * @param attr_id_list_len the total number of ids to scan
 *     NOTE: the ids, values and len are assumed to be the same size
 * @return
 */
bool cps_api_obj_tool_attr_matches(cps_api_object_t obj, cps_api_attr_id_t *ids, void ** values, size_t *len, size_t attr_id_list_len);


/**
 * This is the callback function for an attribute.  Will be provided
 * @param context the context that the user passed
 * @param data the actual attribute's data
 * @param len the length of the data
 * @param stop a boolean pointer that if set to false will stop the looking for more attributes
 */
typedef void (*cps_api_obj_tool_attr_callback_t)(void * contect, void *attrs[], size_t sizes[],
        size_t number_of_attrs, bool *stop);

typedef struct {
    cps_api_attr_id_t id;
    cps_api_obj_tool_attr_callback_t callback;
    void * context;
} cps_api_obj_tool_attr_cb_list_t;

/**
 * This will call the specified callback each time the attribute is discovered in the object.
 * This API will provide a list of all instances of the attribute being searched for at one time
 *
 * @param obj the object to search for attributes
 * @param id the attribute ID that when found, the CB will be called
 * @param cb the callback
 * @param context the "context" parameter passed to the callback
 * @return true if successful otherwise false.
 */
void cps_api_obj_tool_attr_callback(cps_api_object_t obj, cps_api_attr_id_t id, cps_api_obj_tool_attr_callback_t cb,
        void *context);

/**
 * Provide a way for a user to specifiy one or more callback handlers for attributes discovered in an object.
 * This can be thoguht of as repeatedly calling @cps_api_obj_tool_attr_callback
 * @param obj the object to search
 * @param lst the list of callback entries
 * @param len the length of the list of callbacks
 */
void cps_api_obj_tool_attr_callback_list(cps_api_object_t obj, cps_api_obj_tool_attr_cb_list_t *lst, size_t len);

/**
 * This API will take a error code and error string and store it into the object.  An application can then call
 * cps_api_object_return_string and cps_api_object_return_code to get the string and return code respectively.
 * @param obj the object that will have the attributes added
 * @param error the error code
 * @param fmt the string formatted error
 * @param ... variable length parameters
 * @return bool if successful otherwise an error
 */
bool cps_api_set_object_return_attrs(cps_api_object_t obj, t_std_error error, const char *fmt, ...);

/**
 * This API will take a error code and store it into the object.  An application can then call
 * cps_api_object_return_code to get the return code.
 * @param obj the object that will have the attributes added
 * @param error the error code
 * @return bool if successful otherwise an error
 */
bool cps_api_object_set_return_code(cps_api_object_t obj, t_std_error error);

/**
 * This function will get return error string in the object if it exists
 * @param obj the object containing the return string
 * @return returns a valid pointer in the case that there is a return string, otherwise a null
 */
const char *cps_api_object_return_string(cps_api_object_t obj);


/**
 * This API will return a pointer to the object's return code if it exists
 * @param obj the object with the return code
 * @return a pointer to the return code within the object or null
 */
const t_std_error *cps_api_object_return_code(cps_api_object_t obj);

/**
 * Check the object for the exact match flag.  If present return its value otherwise return false (no exact match by default)
 * @param obj the object in question
 * @param have_exact_match true if want an exact match against all attributes in the provided object otherwise false
 * @return true if successfully set the attribute
 */
bool cps_api_object_exact_match(cps_api_object_t obj, bool have_exact_match);

/**
 * Check the object for the exact match flag.  If present return its value otherwise return false (no exact match by default)
 * @param obj the object in question
 * @return true if exact match is desired otherwise false
 */
bool cps_api_object_get_exact_match_flag(cps_api_object_t obj);


/**
 * Compact the object by removing any temporary CPS attributes (eg. return code, status, transaction id, etc..) of an object
 * @param obj the object to operate on
 * @return true if successful otherwise false
 */
bool cps_api_object_compact(cps_api_object_t obj);

#ifdef __cplusplus
}
#endif
/**
 * @}
 * @}
 * @}
 */

#endif /* CPS_API_INC_CPS_API_OBJECT_TOOLS_H_ */
