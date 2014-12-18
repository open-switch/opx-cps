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

#define CPS_API_INT_IF_OBJ_KEY_IFIX (CPS_OBJ_KEY_APP_INST_POS+1)

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
