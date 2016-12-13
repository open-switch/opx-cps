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
 * cps_api_object_key.h
 */

#ifndef CPS_API_INC_CPS_API_OBJECT_KEY_H_
#define CPS_API_INC_CPS_API_OBJECT_KEY_H_

/** @addtogroup CPSAPI
 *  @{
 *  @addtogroup ObjectAndAttributes Object and Object Attribute Handling
 *
 *  Utilities to create, and manage and update data associated with an object's
    key. Provide mechanisms to get key data and set key data.
    <p>Applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_object_key.h>
 @endverbatim

 *  @{
*/

/**
 *  @addtogroup ObjectKey Object Key Handling
 *
 *  Utilities to create, and manage and update data associated with an object's
    key. Provide mechanisms to get key data and set key data.
    <p>Applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_object_key.h>
 @endverbatim

 *  @{
*/

#include "cps_api_object.h"
#include "cps_api_object_attr.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Query the object to get the key instance data.
 * @param obj the object that contains the key instance data
 * @param id the attribute ID of the key instance data
 * @return the pointer to the attribute or NULL if not found
 */
cps_api_object_attr_t cps_api_get_key_data(cps_api_object_t obj,cps_api_attr_id_t id);

/**
 * Set the object's key instance data with the specified value.
 * @param obj the object in question
 * @param id the the key who's data will be set
 * @param type the data type to set
 * @param data the data that will be set as the key instance
 * @param len the length of the data
 * @return true if successful
 */
bool cps_api_set_key_data(cps_api_object_t obj,cps_api_attr_id_t id,
        cps_api_object_ATTR_TYPE_t type, const void *data, size_t len);


/**
 * Set the key data based on the size of the uint passed in. If 1 byte add a char, 2 bytes add a short, etc
 * @param obj the object to add
 * @param id the attribute of the key data
 * @param data the data pointer to use
 * @param len the length of the int (1,2,4,8 supported)
 * @return true if successful otherwise an error
 */
static inline bool cps_api_set_key_data_uint(cps_api_object_t obj,cps_api_attr_id_t id,
        const void *data, size_t len) {
    return cps_api_set_key_data(obj,id,cps_api_object_int_type_for_len(len),data,len);
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 * @}
 */

#endif /* CPS_API_INC_CPS_API_OBJECT_KEY_H_ */
