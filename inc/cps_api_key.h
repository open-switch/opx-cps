/* OPENSOURCELICENSE */
/*
 * cps_api_key.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_KEY_H_
#define CPS_API_KEY_H_


/** @addtogroup CPSAPI The CPS API
 *  @{
 *  @addtogroup Key Key Management
 *     This file consists of the utilities to create, and manage keys.
 *     In the CPS, a key is designed to identify a instance or a type of instances.
 *     The key can also be used to refer to a subtree of objects/instances.
 *  @{
*/


#include "cps_key_internals.h"
#include "std_assert.h"

#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <endian.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup typesandconsts
 * @{
 */

/**
 * Each element in the key will be a single uint32_t
 * and the size of the key is subtracted from the total number of available
 * element
 */
#define CPS_OBJ_MAX_KEY_LEN ((sizeof(cps_api_key_t)/CPS_OBJ_KEY_ELEM_SIZE)-CPS_OBJ_KEY_ELEM_START)
/**@}*/

/**
 * Get the length of elements in the key
 * @param elem
 * @return the number of elements in the key
 */
static inline size_t cps_api_key_get_len(cps_api_key_t * elem) {
    return cps_api_key_elem_raw_get(elem,CPS_OBJ_KEY_LEN_POS);
}

/**
 * Set the length of the key
 * @param elem the key to update
 * @param len the length of the key
 */
static inline void cps_api_key_set_len(cps_api_key_t *elem, size_t len) {
    STD_ASSERT(len < CPS_OBJ_MAX_KEY_LEN);
    cps_api_key_elem_raw_set(elem,CPS_OBJ_KEY_LEN_POS,(cps_api_key_element_t)len);
}

/**
 * Check the offset to ensure it is a valid key element position
 * @param elem the key to validate against
 * @param offset the position to validate
 * @return true if the index is valid
 */
static inline bool cps_api_key_valid_offset(cps_api_key_t *elem, size_t offset) {
    return offset < cps_api_key_get_len(elem);
}

/**
 * Get the attributes associated with a key
 * @param elem the key to get the attributes from
 */
static inline cps_api_key_element_t cps_api_key_get_attr(cps_api_key_t *elem) {
    return cps_api_key_elem_raw_get(elem,CPS_OBJ_KEY_ATTR_POS);
}

/**
 * Set the attributes associated with a kery
 * @param elem the key to set the attributes on
 * @param attr the attributes to set on the key
 */
static inline void cps_api_key_set_attr(cps_api_key_t *elem, uint32_t attr) {
    cps_api_key_elem_raw_set(elem,CPS_OBJ_KEY_ATTR_POS,(cps_api_key_element_t)attr);
}

/**
 * Get the pointer to the first element of the key - internal use only
 * @param elem the key
 * @return a pointer to the first element in the key
 */
static inline cps_api_key_element_t * cps_api_key_elem_start(cps_api_key_t *elem) {
    return cps_api_key_elem_raw(elem,CPS_OBJ_KEY_ELEM_START);
}

/**
 * Get a const pointer to the first element of the key
 * @param elem the key
 * @return a pointer to the first element in the key
 */
static inline const cps_api_key_element_t * cps_api_key_elem_start_const(const cps_api_key_t *elem) {
    return (const cps_api_key_element_t *) cps_api_key_elem_raw((cps_api_key_t *)elem,CPS_OBJ_KEY_ELEM_START);
}

/**
 * Set an element in the key to the specified value
 * @param elem the key to update
 * @param offset the offset of the field to update (must be less then CPS_OBJ_MAX_KEY_LEN)
 * @param field the uint32_t value to set
 */
static inline void cps_api_key_set(cps_api_key_t *elem, uint32_t offset, cps_api_key_element_t field) {
    STD_ASSERT(offset < CPS_OBJ_MAX_KEY_LEN);
    cps_api_key_elem_raw_set(elem,offset+CPS_OBJ_KEY_ELEM_START,field);
}

/**
 * Get the key element at the offset specified
 * @param elem the element to check
 * @param offset the offset to check at
 * @return the uint32_t element at the position requested
 */
static inline cps_api_key_element_t cps_api_key_element_at(cps_api_key_t *elem, uint32_t offset) {
    return cps_api_key_elem_raw_get(elem,CPS_OBJ_KEY_ELEM_START+offset);
}

/**
 * Insert a element into the key at the given spot
 * @param key the pointer to the key to modify
 * @param ix the index of the element
 * @param elem the actial item to insert
 * @return true if successful otherwise the buffer must have been too small
 */
bool cps_api_key_insert_element(cps_api_key_t *key, size_t ix, cps_api_key_element_t elem);

/**
 * Remove the item from the key at the specific index and compress the remaining key
 * @param key the key in question
 * @param ix the index of the element to remove
 */
void cps_api_key_remove_element(cps_api_key_t *key, size_t ix);

/**
 * Given a key return the hash that should be pretty unique
 * @param elem the key to hash
 * @return the hash value
 */
uint64_t cps_api_key_hash(cps_api_key_t *elem);

/**
 * Copy the key from src to destination.
 * @param dest the destination where to copy the key to
 * @param src of where to copy the key from
 */
static inline void cps_api_key_copy(cps_api_key_t *dest, cps_api_key_t *src) {
    size_t len = cps_api_key_get_len_in_bytes(src) + CPS_OBJ_KEY_HEADER_SIZE;
    memcpy(dest,src,len);
}

/**
 * Return a comparison of keys.
 *     if an exact match is required both keys must match exactly in size and contents
 *     if an exact match is not required
 *         the src must be larger then the prefix otherwise it is a failed match
 *
 * @param key the src key to compare against
 * @param prefix the destination key to compare against
 * @param exact true if want to search all of the key other wise limit to the size of the
 *                 comparison
 * @return  0 if there is a match
 *              1 if the key is > then the comparison
 *              -1 if the key is < then the comparison
 */

int cps_api_key_matches(cps_api_key_t * key, cps_api_key_t * prefix, bool exact) ;

/**
 * @addtogroup typesandconsts
 * @{
 */
#define CPS_API_KEY_STR_MAX (1024)
/**@}*/

/**
 * A debug API to print the key into a passed in buffer.
 * @param key to convert to string
 * @param buff the string buffer that will hold the text
 * @param len the length of the buffer
 * @return a pointer to buff passed in
 */
char * cps_api_key_print(cps_api_key_t *key, char *buff, size_t len);

/**
 * An API that takes a key as a string and returns the key object filled in
 * @param key the key that will be filled in with the contents of the string
 * @param buff NULL terminiated string containing the key "1.1.2.3.4"
 * @return true if parsing was successful
 */
bool cps_api_key_from_string(cps_api_key_t *key,const char *buff);

#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
*/

#endif /* CPS_API_KEY_H_ */
