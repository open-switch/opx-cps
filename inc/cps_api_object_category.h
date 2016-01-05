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
 * @addtogroup CPSAPI The CPS API
 * @{
 * @addtogroup Misc
 * This file describes all of the categories that are available.
 * This file also has the general definition for all sub-categories which must fit into
 * a uint32_t cps_api_object_subcategory_types_t
 * @{
*/

/**
 * The object categories definition is not depreciated - please use object categories within the appliciable CPS api generated files
 */

typedef enum {
    cps_api_obj_cat_RESERVED    = 1,//! reserved for internal use
    cps_api_obj_cat_KEY=2,      //!< @depreciated
    cps_api_obj_cat_INTERFACE=3,//!< @depreciated
    cps_api_obj_cat_ROUTE=4,    //!< @depreciated
    cps_api_obj_cat_QOS=5,      //!< @depreciated
    cps_api_obj_cat_PHY=6,      //!< @depreciated
    cps_api_obj_cat_L2=7,       //!< @depreciated this is only temporary use - do not use for any other component
    cps_api_obj_cat_CPS_OBJ=8,  //!< @depreciated this object handles CPS to object
    cps_api_obj_cat_CPS_INFA=9, //!< this object handles CPS infrastructure objs
    cps_api_obj_cat_MAX,
}cps_api_object_category_type_internal_t;

/*
 * All CPS Object categories fit within a uint32_t which also is the basic type of the CPS key elements
 * @depreciated
 */
typedef uint32_t cps_api_object_category_types_t;

/*
 * The object sub category also fits within a uint32_t wich is also the basic type of the CPS key elements
 * * @depreciated
 */
typedef uint32_t cps_api_object_subcategory_types_t;

/**
 * @}
 * @}
 */
#endif /* cps_api_objECT_CATEGORY_H_ */
