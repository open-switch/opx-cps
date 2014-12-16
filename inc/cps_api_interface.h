/*
 * filename: cps_api_interface.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */


#ifndef cps_api_iNTERFACE_H_
#define cps_api_iNTERFACE_H_

#include "cps_api_interface_types.h"
#include "ds_common_types.h"
#include "std_error_codes.h"

#include "cps_api_object.h"
#include "cps_api_operation.h"

/**
 * Interface Category subtypes
 */
typedef enum {
    cps_api_int_obj_INTERFACE=1,//!< db_int_obj_INTERFACE
    cps_api_int_obj_ROUTE,    //!< db_int_obj_ROUTE
    cps_api_int_obj_IP,       //!< db_int_obj_IP
    cps_api_int_obj_NEIGHBOUR,//!< db_int_obj_NEIGHBOUR
    cps_api_int_obj_MAC,      //!< db_int_obj_MAC
    cps_api_int_obj_IF_OBJ,   //!< db_int_obj_IF_OBJ
    cps_api_int_obj_IF_OSTATE,//!< db_int_obj_IF_OSTATE
    cps_api_int_obj_IF_ASTATE, //!< db_int_obj_IF_ASTATE
    cps_api_int_obj_VLAN_INTERFACE,
    cps_api_int_obj_INTERFACE_ADDR,
    cps_api_int_obj_HW_LINK_STATE,
}cps_api_interface_sub_category_t ;

/**
 * Create a CPS API key pointing at a specific if instance
 * or point to all indexes
 * @param key the key to fill in
 * @param use_inst true if want to add the instance variables
 * @param vrf the VRF ID
 * @param ifix the IF index
 */
static inline void cps_api_int_if_key_create(
        cps_api_key_t *key, bool use_inst,
        uint32_t vrf, uint32_t ifix) {

    cps_api_key_init(key,
        cps_api_qualifier_TARGET,
        cps_api_obj_cat_INTERFACE,
        cps_api_int_obj_INTERFACE, use_inst ? 2 : 0,
                vrf, ifix);
}

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
#define DB_OBJ_IF_RECORD (DB_OBJ_MAKE(cps_api_obj_cat_INTERFACE,cps_api_int_obj_IF_OBJ))
#define DB_OBJ_IF_MAC (DB_OBJ_MAKE(cps_api_obj_cat_INTERFACE,cps_api_int_obj_MAC))
#define DB_OBJ_IF_ASTATE (DB_OBJ_MAKE(cps_api_obj_cat_INTERFACE,cps_api_int_obj_IF_ASTATE))

#define DB_OBJ_IF_RECORD_KEY DB_OBJ_MAKE(cps_api_obj_cat_KEY,cps_api_obj_cat_INTERFACE)
#define DB_OBJ_IF_RECORD_IFNAME_KEY DB_OBJ_MAKE(cps_api_obj_cat_KEY,cps_api_interface_key_IFNAME)


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
void db_interface_print(cps_api_object_t list);

#endif /* DB_INTERFACE_H_ */
