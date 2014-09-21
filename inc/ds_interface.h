/*
 * filename: db_interface.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_interface.h
 */

#ifndef DB_INTERFACE_H_
#define DB_INTERFACE_H_

#include "ds_interface_types.h"
#include "ds_common_types.h"
#include "std_error_codes.h"
#include "ds_common_list.h"

/**
 * Interface Category subtypes
 */
typedef enum {
    ds_int_obj_INTERFACE,//!< db_int_obj_INTERFACE
    ds_int_obj_ROUTE,    //!< db_int_obj_ROUTE
    ds_int_obj_IP,       //!< db_int_obj_IP
    ds_int_obj_NEIGHBOUR,//!< db_int_obj_NEIGHBOUR
    ds_int_obj_MAC,      //!< db_int_obj_MAC
    ds_int_obj_IF_OBJ,   //!< db_int_obj_IF_OBJ
    ds_int_obj_IF_OSTATE,//!< db_int_obj_IF_OSTATE
    ds_int_obj_IF_ASTATE, //!< db_int_obj_IF_ASTATE
    ds_int_obj_VLAN_INTERFACE,
    ds_int_obj_INTERFACE_ADDR,
    ds_int_obj_HW_LINK_STATE
}ds_interface_sub_category_t ;

/**
 * The type of key to use
 */
typedef enum {
    db_interface_key_IFNAME,//!< db_interface_key_IFNAME
    db_interface_key_INDEX, //!< db_interface_key_INDEX
} db_interface_key_type_t;

/**
 * This is a key structure
 */
typedef struct {
    db_interface_key_type_t key;
    unsigned int vrfid;
    union {
        hal_ifindex_t ifindex;
        char if_name[HAL_IF_NAME_SZ+1];
    } un;
}db_interface_key_t;

/**
 * The data type used to set either the mac address or admin state specifically instead of the full
 * object
 */
typedef struct {
    db_interface_key_t key;
    union {
        hal_mac_addr_t mac;
        db_interface_state_t astate;
    }un;
} db_if_set_request_t;

/**
 * A few wrappers to create common IF object IDs
 */
#define DB_OBJ_IF_RECORD (DB_OBJ_MAKE(ds_obj_cat_INTERFACE,ds_int_obj_IF_OBJ))
#define DB_OBJ_IF_MAC (DB_OBJ_MAKE(ds_obj_cat_INTERFACE,ds_int_obj_MAC))
#define DB_OBJ_IF_ASTATE (DB_OBJ_MAKE(ds_obj_cat_INTERFACE,ds_int_obj_IF_ASTATE))

#define DB_OBJ_IF_RECORD_KEY DB_OBJ_MAKE(ds_obj_cat_KEY,ds_obj_cat_INTERFACE)
#define DB_OBJ_IF_RECORD_IFNAME_KEY DB_OBJ_MAKE(ds_obj_cat_KEY,ds_interface_key_IFNAME)


/**
 * Copy an interface name from in to out.  Handles non-terminated strings
 * @param out destination of ifname
 * @param in source of ifname
 */
void db_interface_ifname_copy(hal_ifname_t *out,const hal_ifname_t* const in);

/**
 * Print all interface structures in the list
 * @param list of objects (only the interface objects will be printed
 */
void db_interface_print(ds_common_list_t list);

#endif /* DB_INTERFACE_H_ */
