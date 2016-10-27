/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * cps_api_object_internal.h
 */

#ifndef CPS_API_OBJECT_INTERNAL_H_
#define CPS_API_OBJECT_INTERNAL_H_

/** @addtogroup CPSAPI
 *  @{
 *
 *  @addtogroup Internal Internal Headers
 *  @warning this is an internal API.  Do not use directly
 *  @{
*/

#include "cps_api_key.h"

#include <stdbool.h>
#include <stddef.h>
#include <string>

/**
 * The internal format of the CPS API's object array.  the main part is the key followed
 * by an array of bytes
 */
typedef struct {
    cps_api_key_t key;
}cps_api_object_data_t;

/**
 * This is the internal structure of the CPS API to manage an object.
 */
typedef struct {
    size_t len;    //the total length of the buffer pointed after the cps_api_object_data_t
                //pointed to by data
    size_t remain;    //the remaining space not used
    bool allocated; //if this buffer is allocated via malloc
    cps_api_object_data_t * data; // the pointer to the TLV array, etc..

} cps_api_object_internal_t;

#define CPS_API_OBJECT_INTERNAL_SPACE_REQ (sizeof(cps_api_object_internal_t) + \
        sizeof(cps_api_object_data_t))

/**
 * @}
 * @}
 */
#endif /* CPS_API_OBJECT_INTERNAL_H_ */
