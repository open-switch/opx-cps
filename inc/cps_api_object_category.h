/*
 * filename: cps_api_object_category.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */


#ifndef cps_api_objECT_CATEGORY_H_
#define cps_api_objECT_CATEGORY_H_

#include "ds_common_types.h"
#include <stdint.h>

/** @defgroup CPSAPI The CPS API
 * This file describes all of the categories that are available.
 * This file also has the general definition for all sub-categories which must fit into
 * a uint32_t cps_api_object_subcategory_types_t
@{
*/

/**
 * The object categories definition is not depreciated - please use object categories within the appliciable CPS api generated files
 */

typedef enum {
    cps_api_obj_cat_RESERVED    = 1,//! reserved for internal use
    cps_api_obj_cat_KEY,      //!< cps_api_obj_cat_KEY
    cps_api_obj_cat_INTERFACE,//!< cps_api_obj_cat_INTERFACE
    cps_api_obj_cat_ROUTE,    //!< cps_api_obj_cat_ROUTE
    cps_api_obj_cat_QOS,       //!< cps_api_obj_cat_QOS
    cps_api_obj_cat_PHY,
    cps_api_obj_cat_L2,        //!< cps_api_obj_cat_L2 @TODO this is only temporary use - do not use for any other component
    cps_api_obj_cat_MAX,
}cps_api_object_category_type_internal_t;

/*
 * All CPS Object categories fit within a uint32_t which also is the basic type of the CPS key elements
 */
typedef uint32_t cps_api_object_category_types_t;

/*
 * The object sub category also fits within a uint32_t wich is also the basic type of the CPS key elements
 */
typedef uint32_t cps_api_object_subcategory_types_t;

/**
 * @}
 */
#endif /* cps_api_objECT_CATEGORY_H_ */
