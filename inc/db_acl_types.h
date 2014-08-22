#ifndef __DB_ACL_TYPES_H
#define __DB_ACL_TYPES_H

#include "db_acl_qualifiers.h"
#include "db_acl_actions.h"
#include "db_acl_qualifier_masks.h"
#include "db_acl_qualifier_mask_union.h"

#define ACL_ALL_PORTS 0xffff

typedef uint16_t port_no;
typedef struct {
    db_acl_qualifierinfo qual_enum;
    db_acl_qualifier_mask_t acl_qualmask;
} db_acl_qualmask_detail_t;

typedef struct {
    db_acl_actioninfo action_enum;
    db_acl_action_t action;
} db_acl_action_detail_t;

typedef struct {
    int db_acl_num_qualifiers;
    int db_acl_num_actions;
    uint16_t num_ports  ;
    int db_acl_entry_index;
    int db_acl_entry_virtual_index;
} db_acl_entry_metadata_t; 

#endif
