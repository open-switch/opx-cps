/* OPENSOURCELICENSE */
/*
 * cps_key_internals.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_KEY_INTERNALS_H_
#define CPS_KEY_INTERNALS_H_

#include <stdint.h>
#include <endian.h>
#include <stddef.h>

/** @defgroup CPSAPI_Internal The CPS API Internal API
 *@warning this is an internal file.  Don't use the APIs directly.
 * this file is for internal manipulation of the key and the definition of the key
 * itself.
@{
*/

/**
 * The data type for each element in the key
 */
typedef uint32_t cps_api_key_element_t;

#define CPS_API_KEYE_TO_HOST(x) le32toh(x)
#define CPS_API_HOST_TO_KEYE(x) htole32(x)


#define CPS_OBJ_KEY_BYTE_MAX_LEN (256)
/**
 * opaque key structure.  Each element in the key is a uint32_t.
 */
typedef uint8_t cps_api_key_t[CPS_OBJ_KEY_BYTE_MAX_LEN];



#define CPS_OBJ_KEY_ELEM_SIZE (sizeof(cps_api_key_element_t))
#define CPS_OBJ_KEY_LEN_POS (0)
#define CPS_OBJ_KEY_ATTR_POS (1)

#define CPS_OBJ_KEY_ELEM_START (CPS_OBJ_KEY_ATTR_POS+1)
#define CPS_OBJ_KEY_HEADER_SIZE (CPS_OBJ_KEY_ELEM_START * CPS_OBJ_KEY_ELEM_SIZE)

/**
 * Get the pointer to the elements in the key using the key data type - internal use only
 * @param elem the key
 * @return a pointer to the first element in the key
 */
static inline cps_api_key_element_t * cps_api_key_elem_raw(cps_api_key_t *elem, size_t offset) {
    return ((cps_api_key_element_t*)elem) + offset;
}

/**
 * Get the start of the key elements in its native format
 * @param elem the key to get the elements of
 * @param offset the offset into the key
 * @return the host converted value of the key at the offset specified
 */
static inline cps_api_key_element_t cps_api_key_elem_raw_get(cps_api_key_t *elem,size_t offset) {
    return CPS_API_KEYE_TO_HOST(*cps_api_key_elem_raw(elem,offset));
}

/**
 * Set a value into the key at the specific location using the host format as input and storing it in the keys native
 *  format
 * @param elem the key to update
 * @param offset the offset into the key data
 * @param field the field to update
 */
static inline void cps_api_key_elem_raw_set(cps_api_key_t *elem,size_t offset, cps_api_key_element_t field) {
    *cps_api_key_elem_raw(elem,offset) = (cps_api_key_element_t)CPS_API_HOST_TO_KEYE(field);
}

/**
 * Get the length in bytes of the key
 */
static inline size_t cps_api_key_get_len_in_bytes(cps_api_key_t * elem) {
    return (cps_api_key_elem_raw_get(elem,CPS_OBJ_KEY_LEN_POS)) * CPS_OBJ_KEY_ELEM_SIZE;
}

/**
 * @}
 */
#endif /* CPS_KEY_INTERNALS_H_ */
