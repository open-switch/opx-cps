/*
 * filename: cps_api_object_category.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */


#ifndef cps_api_objECT_CATEGORY_H_
#define cps_api_objECT_CATEGORY_H_

#include "ds_common_types.h"
#include <stdint.h>

/**
 * The object categories
 */
typedef enum {
    cps_api_obj_cat_RESERVED    = 1,//! reserved for internal use
    cps_api_obj_cat_KEY,      //!< cps_api_obj_cat_KEY
    cps_api_obj_cat_INTERFACE,//!< cps_api_obj_cat_INTERFACE
    cps_api_obj_cat_ROUTE,    //!< cps_api_obj_cat_ROUTE
    cps_api_obj_cat_QOS,       //!< cps_api_obj_cat_QOS
    cps_api_obj_cat_PHY,
    cps_api_obj_cat_MAX,
}cps_api_object_category_types_t;

typedef uint32_t cps_api_object_subcategory_types_t;


#endif /* cps_api_objECT_CATEGORY_H_ */
