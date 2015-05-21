/** OPENSOURCELICENSE */
/*
 * cps_api_object_key.h
 *
 *  Created on: May 13, 2015
 */

#ifndef CPS_API_INC_CPS_API_OBJECT_KEY_H_
#define CPS_API_INC_CPS_API_OBJECT_KEY_H_

#include "cps_api_object.h"
#include "cps_api_object_attr.h"
#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CPSAPI The CPS API
 *
    This file consists of the utilities to create, and manage and update data associated with an object's
    key.  This file provides mechanisms to get key data and set key data

@{
*/

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
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* CPS_API_INC_CPS_API_OBJECT_KEY_H_ */
