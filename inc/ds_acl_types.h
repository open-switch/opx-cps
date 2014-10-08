/**
 * filename: ds_acl_types.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/* OPENSOURCELICENSE */

#ifndef __DS_ACL_TYPES_H
#define __DS_ACL_TYPES_H

#include <stdint.h>
#include "stddef.h"
#include "ds_common_types.h"
#include "ds_acl_qualifier.h"
#include "ds_acl_action.h"
#include "ds_acl_qualifier_masks.h"
#include "ds_acl_qualifier_mask_union.h"


#define ACL_ALL_PORTS 0xffff
#define ACL_ALL_UNITS 0xff

#define ACL_FEATURE_NAME_LEN_MAX 30
#define ACL_ROOT_PATH  "/etc/dn/acl"

/**
 * Struct holding ACL Stage 
 */
typedef enum {
    ds_acl_STAGE_INGRESS=0,
    ds_acl_STAGE_EGRESS,
    ds_acl_STAGE_PRE_INGRESS,
} ds_acl_stage_t;

/**
 * Struct holding ACL Entry Flags
 */
typedef enum {
    ds_acl_DELETE_ENTRY = 1,
    ds_acl_ADD_STATS = 2,
} ds_acl_entry_flags_t;

/**
 * Struct holding ACL Feature info
 * 
 */
typedef struct  {
    unsigned int feature_id;
    size_t size;
} ds_acl_feature_info_t;

/**
* qual_enum identifies the qualifier
* acl_qualmask is union of all possible qualifiers
*/
typedef struct {
    ds_acl_qualifier_enum_t qual_enum;
    ds_acl_qualifier_mask_t acl_qualmask;
} ds_acl_qualmask_detail_t;

/**
* qual_enum identifies the qualifier
* acl_qualmask is union of all possible qualifiers
*/
typedef struct
{
    ds_acl_action_enum_t action_enum;
    int action_arg1;
    int action_arg2;
}ds_acl_action_detail_t;

/**
* details of the entry to be installed in TCAM
*/

typedef struct {
    ds_acl_stage_t acl_stage;
    unsigned int num_qualifiers;
    ds_acl_qualmask_detail_t *qual_array; /* array of num_qualifiers elements */
    unsigned int num_actions;
    ds_acl_action_detail_t *action_array; /* array of num_actions elements */
    port_t num_ports;
    port_t *port_array; /* array of num_ports elements */
    unsigned int entry_index;
    unsigned int entry_virtual_index; /* priority of entry, if different from index */
    ds_acl_entry_flags_t  entry_flags;
} ds_acl_entry_metadata_t;

#endif
