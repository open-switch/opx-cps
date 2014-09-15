/**
 * filename: db_acl_qualifiers.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/ 
     
/** OPENSOURCELICENSE */
/*
 * db_acl_qualifiers.h
 */

#ifndef DB_ACL_QUALIFIERS_H_
#define DB_ACL_QUALIFIERS_H_

#include "db_common_types.h"
#include "std_error_codes.h"
#include "db_common_list.h"

typedef enum
{
 db_acl_SRC_MAC = 0,
 db_acl_ETHER_TYPE,
 db_acl_COLOR,
 db_acl_INT_CLASS_L2,
 db_acl_ING_PORT,
 db_acl_EGR_PORT,
 db_acl_IP_PROTO,
 db_acl_IP_TTL,
 db_acl_IP_FLAGS,
 db_acl_IP_SRC,
 db_acl_IP_DST,
 db_acl_IP6_SRC,
 db_acl_IP6_DST,
 db_acl_DST_L2_PORT,
 db_acl_DST_MAC,
 db_acl_SRC_MAC,
 db_acl_OUTER_VLAN,
 db_acl_SRC_L4_PORT,
 db_acl_DST_L4_PORT,
 db_acl_DSCP,
 db_acl_ING_STP_STATE,
 db_acl_FWD_VLAN_VALID,
 db_acl_IP_FRAG,
 db_acl_SRC_CLASS,
 db_acl_ICMP_TYPE,
 db_acl_SRC_TRUNK,
 db_acl_LPBK_TYPE,
 db_acl_DST_CLASS,
 db_acl_INT_CLASS_PORT,
 db_acl_TUNNEL_TYPE,
 db_acl_L2_STATION_MOVE,
 db_acl_TCP_CONTROL,
 db_acl_LAST_QUALIFIER
}db_acl_qualifierinfo;

typedef struct _action
{
    db_acl_qualifierinfo qualifier;
    int param1;
    int param2;
}db_acl_qualifier_t;

#endif /* DB_ACL_QUALIFIERS_H_ */
