/* OPENSOURCELICENSE */
/*
 * cps_api_object_internal.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_OBJECT_INTERNAL_H_
#define CPS_API_OBJECT_INTERNAL_H_

#include "cps_api_key.h"

/** @defgroup CPSAPI_Internal The CPS API Internal API
 * The internal portion of the NS resolution API
@warning this is an internal API.  Do not use directly
@{
*/

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
 */
#endif /* CPS_API_OBJECT_INTERNAL_H_ */
