/**
 * filename: db_acl_qualifier.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */


/* DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */

#ifndef __DB_ACL_QUALIFIER_H__
#define __DB_ACL_QUALIFIER_H__

typedef enum {

db_acl_SRC_MAC,
db_acl_DST_MAC,
db_acl_ETHER_TYPE,
db_acl_COLOR,
db_acl_OUTER_VLAN,
db_acl_INTERFACE_CLASS_L2,
db_acl_L2_STATION_MOVE,
db_acl_ING_STP_STATE,
db_acl_FORWARDING_VLAN_VALID,
db_acl_IP_SRC,
db_acl_IP_DST,
db_acl_IP_TYPE,
db_acl_IP_PROTOCOL,
db_acl_IP_FRAG,
db_acl_IP_FLAGS,
db_acl_IP_TTL,
db_acl_DSCP,
db_acl_ICMP_TYPE_CODE,
db_acl_IN_PORT,
db_acl_IN_PORTS,
db_acl_OUT_PORT,
db_acl_SRC_L4_PORT,
db_acl_DST_L4_PORT,
db_acl_TCP_CONTROL,
db_acl_TUNNEL_TYPE,
db_acl_PACKET_RES, 
db_acl_QUALIFIER_COUNT /* Always Last. Not a usable value.*/

}db_acl_qualifier_enum_t;

#endif

