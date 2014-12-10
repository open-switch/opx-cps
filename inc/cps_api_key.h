/* OPENSOURCELICENSE */
/*
 * cps_api_key.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_KEY_H_
#define CPS_API_KEY_H_

#include "cps_key_internals.h"


#include "std_assert.h"


#include <stdbool.h>
#include <string.h>

#include "endian.h"

#define CPS_OBJ_KEY_INST_POS (0)
#define CPS_OBJ_KEY_CAT_POS (1)
#define CPS_OBJ_KEY_SUBCAT_POS (2)

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Each element in the key will be a single uint32_t
 * and the size of the key is subtracted from the total number of available
 * element
 */
#define CPS_OBJ_MAX_KEY_LEN ((sizeof(cps_obj_key_t)/CPS_OBJ_KEY_ELEM_SIZE)-CPS_OBJ_KEY_ELEM_START)

/**
 * Get the length of elements in the key
 * @param elem
 * @return the number of elements in the key
 */
static inline uint32_t cps_api_key_get_len(cps_obj_key_t *elem) {
	return le32toh(((uint32_t*)elem)[CPS_OBJ_KEY_LEN_POS]);
}

/**
 * Set the length of the key
 * @param elem the key to update
 * @param len the length of the key
 */
static inline void cps_api_key_set_len(cps_obj_key_t *elem, uint32_t len) {
	STD_ASSERT(len < CPS_OBJ_MAX_KEY_LEN);
	((uint32_t*)elem)[CPS_OBJ_KEY_LEN_POS] = htole32(len);
}

/**
 * Get the attributes associated with a key
 * @param elem the key to get the attributes from
 */
static inline uint32_t cps_api_key_get_attr(cps_obj_key_t *elem) {
	return le32toh(((uint32_t*)elem)[CPS_OBJ_KEY_ATTR_POS] );
}

/**
 * Set the attributes associated with a kery
 * @param elem the key to set the attributes on
 * @param attr the attributes to set on the key
 */
static inline void cps_api_key_set_attr(cps_obj_key_t *elem, uint32_t attr) {
	((uint32_t*)elem)[CPS_OBJ_KEY_ATTR_POS] = htole32(attr);
}

/**
 * Get the pointer to the first element of the key
 * @param elem the key
 * @return a pointer to the first element in the key
 */
static inline uint32_t * cps_api_key_elem_start(cps_obj_key_t *elem) {
	return ((uint32_t*)elem)+CPS_OBJ_KEY_ELEM_START;
}

/**
 * Set an element in the key to the specified value
 * @param elem the key to update
 * @param offset the offset of the field to update (must be less then CPS_OBJ_MAX_KEY_LEN)
 * @param field the uint32_t value to set
 */
static inline void cps_api_key_set(cps_obj_key_t *elem, uint32_t offset, uint32_t field) {
	STD_ASSERT(offset < CPS_OBJ_MAX_KEY_LEN);
	cps_api_key_elem_start(elem)[offset] = htole32(field);
}

/**
 * Check the offset to ensure it is a valid key element position
 * @param elem the key to validate against
 * @param offset the position to validate
 * @return true if the index is valid
 */
static inline bool cps_api_key_valid_offset(cps_obj_key_t *elem, uint32_t offset) {
	return offset < cps_api_key_get_len(elem);
}

/**
 * Get the key element at the offset specified
 * @param elem the element to check
 * @param offset the offset to check at
 * @return the uint32_t element at the position requested
 */
static inline uint32_t cps_api_key_element_at(cps_obj_key_t *elem, uint32_t offset) {
	return le32toh(cps_api_key_elem_start(elem)[offset]);
}

/**
 * Copy the key from src to destination.
 * @param dest the destination where to copy the key to
 * @param src of where to copy the key from
 */
static inline void cps_api_key_copy(cps_obj_key_t *dest, cps_obj_key_t *src) {
	size_t len = cps_api_key_get_len_in_bytes(src) + CPS_OBJ_KEY_HEADER_SIZE;
	memcpy(dest,src,len);
}

/**
 * Return a comparison of keys.
 * 	if an exact match is required both keys must match exactly in size and contents
 * 	if an exact match is not required, the src must be larger then the comparison otherwise it is a failed match
 *
 * @param key the src key to compare against
 * @param comparison the destination key to compare against
 * @param exact true if want to search all of the key other wise limit to the size of the
 * 				comparison
 * @return  0 if there is a match
 * 		 	1 if the key is > then the comparison
 * 		 	-1 if the key is < then the comparison
 */
int cps_api_key_matches(cps_obj_key_t *key, cps_obj_key_t *comparison, bool exact) ;

/**
 * Print the key into a passed in buffer
 * @param key
 * @param buff the string buffer that will hold the text
 * @param len the length of the buffer
 * @return a pointer to buff
 */
char * cps_api_key_print(cps_obj_key_t *key, char *buff, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* CPS_API_KEY_H_ */
