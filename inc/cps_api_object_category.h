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
 * filename: cps_api_object_category.h
 */



#ifndef cps_api_objECT_CATEGORY_H_
#define cps_api_objECT_CATEGORY_H_

#include "ds_common_types.h"
#include <stdint.h>

/**
 * @addtogroup CPSAPI
 * @{
 * @addtogroup Miscellaneous Utilities
 * This file describes all of the categories that are available.
 * This file also has the general definition for all sub-categories which must fit into
 * a uint32_t cps_api_object_subcategory_types_t
 * @{
*/

/**
 * The object categories definition is not deprecated - please use object categories within the appliciable CPS API generated files
 */

typedef enum {
    cps_api_obj_cat_RESERVED    = 1,//! reserved for internal use
    cps_api_obj_cat_KEY=2,      //!< @deprecated
    cps_api_obj_cat_INTERFACE=3,//!< @deprecated
    cps_api_obj_cat_ROUTE=4,    //!< @deprecated
    cps_api_obj_cat_QOS=5,      //!< @deprecated
    cps_api_obj_cat_PHY=6,      //!< @deprecated
    cps_api_obj_cat_L2=7,       //!< @deprecated this is only temporary use - do not use for any other component
    cps_api_obj_cat_CPS_OBJ=8,  //!< @deprecated this object handles CPS to object
    cps_api_obj_cat_CPS_INFA=9, //!< this object handles CPS infrastructure objs
    cps_api_obj_cat_MAX,
}cps_api_object_category_type_internal_t;

/*
 * All CPS Object categories fit within a uint32_t which also is the basic type of the CPS key elements
 * @deprecated
 */
typedef uint32_t cps_api_object_category_types_t;

/*
 * The object sub category also fits within a uint32_t wich is also the basic type of the CPS key elements
 * * @deprecated
 */
typedef uint32_t cps_api_object_subcategory_types_t;

/**
 * @}
 * @}
 */
#endif /* cps_api_objECT_CATEGORY_H_ */
