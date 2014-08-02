/** OPENSOURCELICENSE */
/*
 * db_object_catagory.h
 */

#ifndef DB_OBJECT_CATAGORY_H_
#define DB_OBJECT_CATAGORY_H_

#include <stdint.h>

typedef enum {
    db_obj_cat_KEY,
    db_obj_cat_INTERFACE,
    db_obj_cat_ROUTE,
    db_obj_cat_QOS
}db_object_catagory_types_t;


static inline db_object_type_t DB_OBJ_MAKE(db_object_catagory_types_t cat,db_object_sub_type_t subtype );

#endif /* DB_OBJECT_CATAGORY_H_ */
