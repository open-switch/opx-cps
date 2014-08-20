/** OPENSOURCELICENSE */
/*
 * db_object_catagory.h
 */

#ifndef DB_OBJECT_CATEGORY_H_
#define DB_OBJECT_CATEGORY_H_

#include "db_common_types.h"
#include <stdint.h>

typedef enum {
    db_obj_cat_KEY,
    db_obj_cat_INTERFACE,
    db_obj_cat_ROUTE,
    db_obj_cat_QOS
}db_object_category_types_t;

#define DB_CAT_BIT_SHIFT ((sizeof(uint64_t)-(sizeof(uint16_t)))*8)
#define DB_CAT_MASK (((db_object_type_t)(~0))<< DB_CAT_BIT_SHIFT)


static inline db_object_type_t DB_OBJ_MAKE(db_object_category_types_t cat,db_object_sub_type_t subtype ) {
    return ((db_object_type_t)cat)<<(DB_CAT_BIT_SHIFT) | ((db_object_type_t)subtype);
}

static inline db_object_sub_type_t DB_OBJ_SUBT(db_object_type_t t) {
    return t & ~(DB_CAT_MASK);
}

static inline db_object_category_types_t DB_OBJ_CAT(db_object_type_t t) {
    return (db_object_category_types_t) ((t & (DB_CAT_MASK)) >> DB_CAT_BIT_SHIFT);
}

#endif /* DB_OBJECT_CATEGORY_H_ */
