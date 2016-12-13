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
 * filename: cps_api_object_attr.h
 */

#ifndef CPS_API_OBJECT_ATTR_H_
#define CPS_API_OBJECT_ATTR_H_

/** @addtogroup CPSAPI
 *  @{
 *  @addtogroup ObjectAndAttributes Object and Object Attribute Handling
 *  @{
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "std_tlv.h"
#include "std_type_defs.h"

#include <stdint.h>
#include <stddef.h>
/**
 *  @addtogroup ObjectAttributes Object Attribute Handling
 *  <p>APIs for manipulating and retrieving object attribute details.</p>
    <p>Applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_object_attr.h>
 @endverbatim

 *  @{
*/

/**
 * @addtogroup typesandconstsObjectAttributes Types and Constants
 * @{
 */

/**
 * The type of each attribute in an object
 */
typedef uint64_t cps_api_attr_id_t;

/**
 * The type of an attribute that is stored within the CPS object
 */
typedef void * cps_api_object_attr_t;

/**
 * This stuct is a helper for walking through a list of attributes.
 * The iterator can be initialized and then keeps the total size of the scope in
 * the len field
 */
typedef struct cps_api_object_it_t {
    size_t len;
    cps_api_object_attr_t attr;
} cps_api_object_it_t;

/**
 * The value that matches a NULL (invalid) attribute
 */
#define CPS_API_ATTR_NULL NULL
/** @} */


/**
 * Check to see if the current iterator is valid
 * @param it the iterator that contains the attribute to check
 * @return true if the attribute contained by the current iterator is valid
 */
static inline bool cps_api_object_it_valid(cps_api_object_it_t *it) {
    return std_tlv_valid(it->attr,it->len);
}

/**
 * Search for the attributes in the match list and replace them with the corresponding
 *     ones in the replace list.
 *
 * @param it the iterator to use
 * @param match the list of matching attribute IDs
 * @param replace the list of attributes that will be replace the attribute
 * @param len the length of matches
 */
void cps_api_object_it_attr_replace(const cps_api_object_it_t *it, const cps_api_attr_id_t *match,
        const cps_api_attr_id_t *replace, size_t len);

/**
 * Start to iterate through all of the contained attributes by the current attribute
 * @param iter the current iterator that we will navigate inside
 */
static inline void cps_api_object_it_inside(cps_api_object_it_t *iter) {
    iter->len = std_tlv_len(iter->attr);
    iter->attr = std_tlv_data(iter->attr);
}

/**
 * Leaving the existing attribute alone, find the next attribute matching the tag and return the cps attribute
 * @param iter the current iterator to start from
 * @param tag the tag to locate
 * @return either NULL if not found or the cps attribute
 */
static inline cps_api_object_attr_t cps_api_object_it_find(const cps_api_object_it_t *iter,std_tlv_tag_t tag) {
    cps_api_object_it_t it = *iter;
    return std_tlv_find_next(it.attr,&it.len,tag);
}

/**
 * Get the next attribute or CPS_API_ATTR_NULL if there are no more attributes
 * @param obj the object to query
 * @param attr the current attribute
 * @return the next attribute or CPS_API_ATTR_NULL if there is no next
 */
static inline bool cps_api_object_it_next(cps_api_object_it_t *iter) {
    iter->attr = std_tlv_next(iter->attr,&iter->len);
    return iter->attr != NULL && iter->len!=0;
}

/**
 * Create a CPS API object attribute iterator from a attribute.  This can be useful if you want to query/walk through embedded attributes.
 *
 * @param attr the attribute to initialize the iterator from
 * @param iter the iterator to initialize
 */
void cps_api_object_it_from_attr(cps_api_object_attr_t attr, cps_api_object_it_t *iter);

/**
 *    Print the attribute into a human readable format - not the data just the header
 * @param attr the attribute to print
 * @param buff the buffer to use
 * @param len the length of the buffer
 * @return the pointer to the buffer passed in
 */
const char * cps_api_object_attr_to_string(cps_api_object_attr_t attr, char *buff, size_t len);

/**
 * Get the attribute id for the object.
 * @param attr is the attribute to query
 * @return the attribute ID of the attribute
 */
cps_api_attr_id_t cps_api_object_attr_id(cps_api_object_attr_t attr);

/**
 * Get the attribute id and return it into a specific enum value.
 * @note limited to uint32.
 *
 * @param attr is the TLV containing the attribute to query
 * @param enumptr is a pointer to the enum value (cast to a void*) - the size must be an int len
 */
void cps_api_object_attr_id_as_enum(cps_api_object_attr_t attr, void *enumptr);

/**
 * Get the length of the attribute.
 * @param attr the attribute to querty
 * @return the length of the current attribute's data
 */
size_t cps_api_object_attr_len(cps_api_object_attr_t attr);

/**
 * Get the data from the attribute as a uint16_t - this will ensure proper endianess if sent via
 * networking or between processes.
 * @param attr the attribute to query
 * @return the data as a uint16_t
 */
uint16_t cps_api_object_attr_data_u16(cps_api_object_attr_t attr);
/**
 * Get the data from the attribute as a uint32_t - this will ensure proper endianess if sent via
 * networking or between processes.
 * @param attr the attribute to query
 * @return the data as a uint32_t
 */
uint32_t cps_api_object_attr_data_u32(cps_api_object_attr_t attr);
/**
 * Get the data from the attribute as a uint64_t - this will ensure proper endianess if sent via
 * networking or between processes.
 * @param attr the attribute to query
 * @return the data as a uint64_t
 */
uint64_t cps_api_object_attr_data_u64(cps_api_object_attr_t attr);

/**
 * Get the data from the attribute as a uint_t ensuring proper endianess.
 * Supports 1, 2 and 4 byte integers only
 *
 * @param attr the attribute to query
 * @return the data as a uint_t (eg..data should be a 1,2,4 bytes in length otherwise will return 0
 */
uint_t cps_api_object_attr_data_uint(cps_api_object_attr_t attr);


/**
 * Get the data from the attribute as a binary blob - this will not ensure proper endianess
 * so the data should already be formatted in a way that is endian neutral for users
 * @param attr the attribute to query
 * @return the data as binary
 */
void *cps_api_object_attr_data_bin(cps_api_object_attr_t attr);


/**
 * Compare the two attributes and return a numerical value determining difference
 * @param lhs one of the attributes to compare
 * @param rhs the second attribute to compare
 * @return 0 if the attributes are the same
 *            1 if the lhs is greater then the rhs (after comparison)
 *            -1 if the lhs is lower then the rhs (after comparison)
 */
int cps_api_object_attrs_compare(cps_api_object_attr_t lhs, cps_api_object_attr_t rhs);


/**
 * This API will start at the current iterator and find the next location of an attribute.
 * The iterator will be updated with the new position. If there is no next, the iterator will be invalid.
 *
 * @param it the iterator to start to search for an attribute that matches the iterator
 * @param attr the attribute to search for
 * @return true if the iterator is valid still otherwise false
 */
bool cps_api_object_it_attr_walk(cps_api_object_it_t *it, cps_api_attr_id_t attr);


/**
 * @}
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif


#endif /* CPS_API_INC_CPS_API_OBJECT_ATTR_H_ */
