/**
 * filename: db_object_category.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */
/*
 * db_object_catagory.h
 */

#ifndef DB_OBJECT_CATEGORY_H_
#define DB_OBJECT_CATEGORY_H_

#include "db_common_types.h"
#include <stdint.h>

/**
 * The object categories
 */
typedef enum {
    db_obj_cat_KEY,      //!< db_obj_cat_KEY
    db_obj_cat_INTERFACE,//!< db_obj_cat_INTERFACE
    db_obj_cat_ROUTE,    //!< db_obj_cat_ROUTE
    db_obj_cat_QOS       //!< db_obj_cat_QOS
}db_object_category_types_t;

/* The following can change without notice to applications.  Ensrue use
 * of the object category APIs
 * ******************************************************************
 * Object is made up of three pieces containing 64 bits
 *    6         5         4         3         2         1         0
 * 3210987654321098765432109876543210987654321098765432109876543210
 * RRCCCCCCCCCCCCCCOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
 ******************************************************************
 * R - Reserved for DB API (2 bits long)
 * C - Category field (14 bits long) - from category types
 * O - Category subfield (48 bits long)
 ******************************************************************/

/**
 * Macros to create/manage category/object ids
 */
#define DB_CAT_RES_BITS    (2)
#define DB_CAT_CAT_WID (14)
#define DB_CAT_OBJ_WID (48)
#define DB_CAT_RES_SHIFT (DB_CAT_CAT_WID+DB_CAT_OBJ_WID)

#define DB_CAT_BIT_SHIFT (((sizeof(uint64_t)*8) - (DB_CAT_RES_BITS + DB_CAT_CAT_WID)))

#define DB_CAT_MASK (((((db_object_type_t)(~0))<< DB_CAT_BIT_SHIFT) & (((db_object_type_t)(~0))>>DB_CAT_RES_BITS)))
#define DB_CAT_OBJ_MASK  (((db_object_type_t)(~0))>>(DB_CAT_RES_BITS+DB_CAT_CAT_WID))
#define DB_CAT_RES_MASK (~(DB_CAT_OBJ_MASK|DB_CAT_MASK))

/**
 * Make an object ID from a category and object type
 * @param cat is the category containing the object
 * @param subtype the category specific object
 * @return the object id
 */
static inline db_object_type_t DB_OBJ_MAKE(db_object_category_types_t cat,db_object_sub_type_t subtype ) {
    return ((((db_object_type_t)cat)<<(DB_CAT_BIT_SHIFT))&DB_CAT_MASK) | ((db_object_type_t)subtype);
}

/**
 * Remove any parts of the object type that is not the category or sub category
 * @param t the input object type
 * @return the filtered object type
 */
static inline db_object_sub_type_t DB_OBJ(db_object_type_t t) {
    return t & (~DB_CAT_RES_MASK);
}

/**
 * Get the object subtype from an object ID
 * @param t the object id
 * @return the object sub type
 */
static inline db_object_sub_type_t DB_OBJ_SUBT(db_object_type_t t) {
    return t & (DB_CAT_OBJ_MASK);
}

/**
 * Get a object category from an object ID
 * @param t is the object ID
 * @return object category
 */
static inline db_object_category_types_t DB_OBJ_CAT(db_object_type_t t) {
    return (db_object_category_types_t) ((t & (DB_CAT_MASK)) >> DB_CAT_BIT_SHIFT);
}


#endif /* DB_OBJECT_CATEGORY_H_ */
