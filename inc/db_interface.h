/** OPENSOURCELICENSE */
/*
 * db_interface.h
 */

#ifndef DB_INTERFACE_H_
#define DB_INTERFACE_H_

#include "db_interface_types.h"
#include "db_common_types.h"
#include "std_error_codes.h"
#include "db_common_list.h"

typedef enum {
    db_int_obj_INTERFACE,
    db_int_obj_ROUTE,
    db_int_obj_IP,
    db_int_obj_NEIGHBOUR,
    db_int_obj_MAC,
    db_int_obj_IF_OBJ,
    db_int_obj_IF_OSTATE,
    db_int_obj_IF_ASTATE
}db_interface_attr_types_t ;

typedef enum {
    db_interface_key_IFNAME,
    db_interface_key_INDEX,
} db_interface_key_type_t;

typedef struct {
    db_interface_key_type_t key;
    unsigned int vrfid;
    union {
        hal_ifindex_t ifindex;
        char if_name[HAL_IF_NAME_SZ+1];
    } un;
}db_interface_key_t;

#define DB_OBJ_IF_RECORD (DB_OBJ_MAKE(db_obj_cat_INTERFACE,db_int_obj_IF_OBJ))
#define DB_OBJ_IF_MAC (DB_OBJ_MAKE(db_obj_cat_INTERFACE,db_int_obj_MAC))
#define DB_OBJ_IF_ASTATE (DB_OBJ_MAKE(db_obj_cat_INTERFACE,db_int_obj_IF_ASTATE))

#define DB_OBJ_IF_RECORD_KEY DB_OBJ_MAKE(db_obj_cat_KEY,db_obj_cat_INTERFACE)
#define DB_OBJ_IF_RECORD_IFNAME_KEY DB_OBJ_MAKE(db_obj_cat_KEY,db_interface_key_IFNAME)


/**
 * Copy an interface name from in to out.  Handles non-terminated strings
 * @param out destination of ifname
 * @param in source of ifname
 */
void db_interface_ifname_copy(hal_ifname_t *out,const hal_ifname_t* const in);

void db_interface_print(db_common_list_t list);

#endif /* DB_INTERFACE_H_ */
