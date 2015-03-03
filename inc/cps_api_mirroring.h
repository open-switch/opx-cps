/*
 * filename: cps_api_mirroring.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/* OPENSOURCELICENSE */
#ifndef CPS_API_MIRRORING_H_
#define CPS_API_MIRRORING_H_

#define CPS_API_MIRROR_OBJ_KEY_ID (CPS_OBJ_KEY_APP_INST_POS+1)

/* List which contains mapping of the Mirror source port, Mirror destination port and the directions
in which mirroring is enabled */
typedef enum {
 MIRROR_INTERFACE_SRC = 1/*!< uint32 */,
 MIRROR_INTERFACE_DST = 2/*!< uint32 */,
 MIRROR_INTERFACE_DIRECTION = 3/*!< enumeration */,
} MIRROR_INTERFACE_t;

typedef enum {
 MIRROR_RSPAN_OBJECT_VLAN = 1/*!< uint16 */,
 MIRROR_RSPAN_OBJECT_TPID = 2/*!< uint16 */,
 MIRROR_RSPAN_OBJECT_VLAN_PRIORITY = 3/*!< uint8 */,
} MIRROR_RSPAN_OBJECT_t;

/* ERSPAN object attributes */
typedef enum {
 MIRROR_ERSPAN_OBJECT_SOURCE_IP = 1/*!< inet:ipv4-address */,
 MIRROR_ERSPAN_OBJECT_DESTINATION_IP = 2/*!< inet:ipv4-address */,
 MIRROR_ERSPAN_OBJECT_TTL = 3/*!< uint8 */,
 MIRROR_ERSPAN_OBJECT_DSCP = 4/*!< inet:dscp */,
 MIRROR_ERSPAN_OBJECT_VERSION = 5/*!< inet:ip-version */,
 MIRROR_ERSPAN_OBJECT_SOURCE_MAC = 6/*!< yang:mac-address */,
 MIRROR_ERSPAN_OBJECT_DEST_MAC = 7/*!< yang:mac-address */,
 MIRROR_ERSPAN_OBJECT_ERSPAN_VLAN_ID = 8/*!< uint16 */,
 MIRROR_ERSPAN_OBJECT_ERSPAN_TPID = 9/*!< uint16 */,
 MIRROR_ERSPAN_OBJECT_ERSPAN_VLAN_PRIORITY = 10/*!< uint8 */,
} MIRROR_ERSPAN_OBJECT_t;

/* NAS Mirroring Object Attributes */
typedef enum {
 MIRROR_OBJECT_MIRROR_SESSION_ID = 1/*!< uint32 */,
 MIRROR_OBJECT_NPU_SESSION_ID = 2/*!< uint32 */,
 MIRROR_OBJECT_MIRROR_INTERFACE = 3/*!< MIRROR_INTERFACE_t */,
 MIRROR_OBJECT_MIRROR_CLASS_OF_SERVICE = 4/*!< uint8 */,
 MIRROR_OBJECT_MIRROR_FLOW_ENABLED = 5/*!< boolean */,
 MIRROR_OBJECT_MIRROR_TYPE = 6/*!< enumeration */,
 MIRROR_OBJECT_MIRROR_RSPAN_OBJECT = 7/*!< MIRROR_RSPAN_OBJECT_t */,
 MIRROR_OBJECT_MIRROR_ERSPAN_OBJECT = 8/*!< MIRROR_ERSPAN_OBJECT_t */,
} MIRROR_OBJECT_t;

/* This module contains a collection of YANG definitions for NAS Mirroring objects */
typedef enum {
 DELL_MIRROR_MIRROR_OBJECT = 1/*!< MIRROR_OBJECT_t */,
 DELL_MIRROR_MIRROR_RSPAN_OBJECT = 2/*!< MIRROR_RSPAN_OBJECT_t */,
 DELL_MIRROR_MIRROR_ERSPAN_OBJECT = 3/*!< MIRROR_ERSPAN_OBJECT_t */,
} DELL_MIRROR_t;

/* NAS Mirroring Direction types  - Tx/Rx */
typedef enum {
  DIRECTION_INGRESS_MIRROR = 1,
  DIRECTION_EGRESS_MIRROR = 2,
  DIRECTION_INGRESS_EGRESS_MIRROR = 3,
} DIRECTION_t;

/* NAS Mirroring SPAN Session Types */
typedef enum {
  MIRROR_TYPE_SPAN = 1,
  MIRROR_TYPE_RSPAN = 2,
  MIRROR_TYPE_ERSPAN = 3,
} MIRROR_TYPE_t;

#endif /* CPS_API_MIRRORING_H_ */
