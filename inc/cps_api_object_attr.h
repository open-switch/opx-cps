/*
 * filename: cps_api_object.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */
/* OPENSOURCELICENSE */

#ifndef CPS_API_OBJECT_ATTR_H_
#define CPS_API_OBJECT_ATTR_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The type of each attribute in an object
 */
typedef uint64_t cps_api_attr_id_t;

/**
 * The type of an attribute that is stored within the CPS object
 */
typedef void * cps_api_object_attr_t;


typedef struct cps_api_object_it_t {
    size_t len;
    cps_api_object_attr_t attr;
} cps_api_object_it_t;

/**
 * The value that matches a NULL (invalid) attribute
 */
#define CPS_API_ATTR_NULL ((void*)0)

/**
 * Check to see if the current iterator is valid
 * @param it the iterator that contains the attribute to check
 * @return true if the attribute contained by the current iterator is valid
 */
static inline bool cps_api_object_it_valid(cps_api_object_it_t *it) {
    return std_tlv_valid(it->attr,it->len);
}

/**
 * Start to iterate through all of the contained attributes by the current attribute
 * @param iter the current iterator that we will navigate inside
 */
static inline void cps_api_object_it_inside(cps_api_object_it_t *iter) {
    iter->len = std_tlv_len(iter->attr);
    iter->attr = std_tlv_data(iter->attr);
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
 * Get the data from the attribute as a binary blob - this will not ensure proper endianess
 * so the data should already be formatted in a way that is endian neutral for users
 * @param attr the attribute to query
 * @return the data as binary
 */
void *cps_api_object_attr_data_bin(cps_api_object_attr_t attr);

#ifdef __cplusplus
}
#endif


#endif /* CPS_API_INC_CPS_API_OBJECT_ATTR_H_ */
