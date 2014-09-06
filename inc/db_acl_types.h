#ifndef __DB_ACL_TYPES_H
#define __DB_ACL_TYPES_H

#include <stdint.h>
#include "db_acl_qualifier.h"
#include "db_acl_action.h"
#include "db_acl_qualifier_masks.h"
#include "db_acl_qualifier_mask_union.h"

#define ACL_ALL_PORTS 0xffff
#define ACL_ALL_UNITS 0xff
#define ACL_FEATURE_NAME_LEN_MAX 30
#define ACL_ROOT_PATH  "/etc/dn/acl"

typedef uint16_t port_no;

typedef uint16_t unit_no;

typedef enum {
    db_acl_STAGE_INGRESS=0,
    db_acl_STAGE_EGRESS,
    db_acl_STAGE_PRE_INGRESS,
} db_acl_stage_t;

typedef enum {
    db_acl_DELETE_ENTRY = 1,
    db_acl_ADD_STATS = 2,
} db_acl_entry_flags_t;

typedef struct  {
    int feature_id;
    int size;
    void *params;
} db_acl_feature_info_t;

typedef struct {
    db_acl_qualifier_enum_t qual_enum;
    db_acl_qualifier_mask_t acl_qualmask;
} db_acl_qualmask_detail_t;

typedef struct
{
    db_acl_action_enum_t action_enum;
    int param1;
    int param2;
}db_acl_action_detail_t;

typedef struct {
    db_acl_stage_t acl_stage;
    int num_qualifiers;
    db_acl_qualmask_detail_t *qual_array;
    int num_actions;
    db_acl_action_action_t *action_array;
    uint16_t num_ports;
    uint16_t *port_array;
    int entry_index;
    int entry_virtual_index;
    db_acl_entry_flags_t  entry_flags;
} db_acl_entry_metadata_t; 

#endif
