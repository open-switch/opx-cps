/* OPENSOURCELICENSE */
/*
 * cps_api_attributes.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_ATTRIBUTES_H_
#define CPS_API_ATTRIBUTES_H_

#include <stdint.h>

typedef enum {
	cps_api_ATTR_TYPE_BIN,
	cps_api_ATTR_TYPE_U16,
	cps_api_ATTR_TYPE_U32,
	cps_api_ATTR_TYPE_U64,
	cps_api_ATTR_TYPE_MAX
}cps_api_attr_types_t;

#define CPS_ATTR_TYPE_POS 56

uint64_t cps_api_add_attr_type(uint64_t attr,cps_api_attr_types_t t) {
	return (attr) | (((uint64_t)t) << CPS_ATTR_TYPE_POS);
}

inline cps_api_attr_types_t cps_api_attr_type(uint64_t attrid) {
	return (cps_api_attr_types_t)(((~((uint64_t)0)) << CPS_ATTR_TYPE_POS & attrid) >> CPS_ATTR_TYPE_POS);
}

#endif /* CPS_API_ATTRIBUTES_H_ */
