/*
 * filename: ds_object_category.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */


#ifndef DS_OBJECT_CATEGORY_H_
#define DS_OBJECT_CATEGORY_H_

#include "ds_common_types.h"
#include <stdint.h>

/**
 * The object categories
 */
typedef enum {
    ds_obj_cat_RESERVED    = 1,//! reserved for internal use
    ds_obj_cat_KEY,      //!< db_obj_cat_KEY
    ds_obj_cat_INTERFACE,//!< db_obj_cat_INTERFACE
    ds_obj_cat_ROUTE,    //!< db_obj_cat_ROUTE
    ds_obj_cat_QOS,       //!< db_obj_cat_QOS
    ds_obj_cat_PHY,
    ds_obj_cat_MAX,
}ds_object_category_types_t;


#endif /* DB_OBJECT_CATEGORY_H_ */
