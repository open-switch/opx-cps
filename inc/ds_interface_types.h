/*
 * filename: ds_interface_types.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */


#ifndef DS_INTERFACE_TYPES_H_
#define DS_INTERFACE_TYPES_H_

#include "ds_common_types.h"

typedef enum  {
    ADMIN_DEF,
    DB_ADMIN_STATE_UP,
    DB_ADMIN_STATE_DN
}db_interface_state_t;

typedef enum  {
    DB_INTERFACE_OP_SET,
    DB_INTERFACE_OP_CREATE,
    DB_INTERFACE_OP_DELETE
}db_interface_operation_t;

typedef enum {
    OPER_DEF,
    DB_OPER_STATE_UP,
    DB_OPER_STATE_DN
}db_interface_operational_state_t ;


typedef enum {
    ADDR_DEFAULT,
    ADDR_ADD,
    ADDR_DEL
}db_if_addr_msg_type_t ;

typedef enum {
    ROUTE_DEFAULT,
    ROUTE_ADD,
    ROUTE_DEL,
    ROUTE_UPD
}db_route_msg_t;


typedef enum {
    QDISC_INVALID,
    QDISC_ADD,
    QDISC_DEL,
} db_qdisc_msg_type_t ;

typedef enum {
    TCA_KIND_INVALID = 0,
    TCA_KIND_PRIO,
    TCA_KIND_HTB,
    TCA_KIND_PFIFO_FAST,
    TCA_KIND_CBQ,
    TCA_KIND_RED,
    TCA_KIND_U32,
    TCA_KIND_INGRESS,
    TCA_KIND_TBF
} db_qos_tca_type_t;

typedef enum {
    NBR_DEFAULT,
    NBR_ADD,
    NBR_DEL,
    NBR_UPD
}db_nbr_event_type_t;

typedef enum {
    CLASS_INVALID,
    CLASS_ADD,
    CLASS_DEL,
} db_qos_class_msg_type_t;

typedef enum {
    DB_IF_TYPE_PHYS_INTER,
    DB_IF_TYPE_VLAN_INTER,
    DB_IF_TYPE_LAG_INTER
} db_if_type_t;

/**
 * The size of an interface name
 */
typedef char hal_ifname_t[HAL_IF_NAME_SZ];

/**
 * Database structure for an interface
 */
typedef struct  {
    char if_name[HAL_IF_NAME_SZ+1];
    hal_ifindex_t   if_index;
    unsigned long   if_flags;
    long            namelen;
    hal_ip_addr_t         if_addr;
    hal_ip_addr_t         if_mask;
    db_if_addr_msg_type_t        if_msgtype;
} db_if_addr_t;

typedef struct  {
    char               if_name[HAL_IF_NAME_SZ+1];
    hal_ifindex_t     if_index;
    unsigned short     if_flags;
    long               namelen;
    hal_mac_addr_t          if_hwaddr;
    unsigned long      vrfid;
    unsigned short     family;
    db_interface_state_t  if_astate;
    db_interface_operational_state_t   if_ostate;
    unsigned int               mtu;
    db_if_type_t         if_type;
    hal_vlan_id_t         if_vlan;
}db_if_t;

typedef struct {
    db_if_t interface;
    db_interface_operation_t operation;
} db_if_event_t;

typedef struct  {
    db_route_msg_t  msg_type;
    unsigned short  distance;
    unsigned short  protocol;
    unsigned long   vrfid;
    hal_ip_addr_t         prefix;
    unsigned short  prefix_masklen;
    hal_ifindex_t   nh_if_index;
    unsigned long   nh_vrfid;
    hal_ip_addr_t         nh_addr;
}db_route_t;


typedef struct  {
    unsigned short  family;
    db_nbr_event_type_t    msg_type;
    hal_ip_addr_t         nbr_addr;
    hal_mac_addr_t      nbr_hwaddr;
    hal_ifindex_t   if_index;
    hal_ifindex_t   phy_if_index;
    unsigned long   vrfid;
    unsigned long   expire;
    unsigned long   flags;
    unsigned long   state;
}db_neighbour_entry_t;

struct db_qos_qdisc_entry_s {
    db_qdisc_msg_type_t msg_type;
    hal_ifindex_t  ifindex;
    db_qos_tca_type_t      qdisc_type;
    uint16_t        qdisc;
    uint32_t        parent;
};

typedef struct db_class_rate_info_s {
    // Rate info of the node
    uint32_t minBw;
    uint32_t maxBw;
    uint32_t burst;
    uint32_t cburst;
} db_qos_class_rate_info_t;

struct db_qos_class_entry_s {
    db_qos_class_msg_type_t msg_type;
    hal_ifindex_t   ifindex;
    uint32_t        qos_class;
    uint16_t        qdisc;
    db_qos_class_rate_info_t rinfo;
};

#endif /* DB_EVENT_INTERFACE_H_ */
