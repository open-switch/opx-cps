/**
 * filename: ds_acl_qualifier_mask_union.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */

/* DO NOT EDIT THIS FILE!
 * This file is auto-generated.
 * Edits to this file will be lost when it is regenerated.
 */
#ifndef __DS_ACL_QUALIFIER_MASK_UNION_H__
#define __DS_ACL_QUALIFIER_MASK_UNION_H__

/**
 * Union holding all acl valmask structs
 */
typedef union {

  ds_acl_valmask32_t valmask32;              /* Generic Valmask struct holding uint32_t types */
  ds_acl_valmask16_t valmask16;              /* Generic Valmask struct holding uint16_t types */  
  ds_acl_valmask8_t valmask8;                /* Generic Valmask struct holding uint8_t types */
  ds_acl_macinfo_valmask_t macinfo_valmask;  /* Generic Valmask struct holding mac addr types */
  ds_acl_ipaddr_valmask_t ipaddr_valmask;    /* IPv4 Address */
  ds_acl_ip6addr_valmask_t ip6addr_valmask;  /* IPv6 Address */
  ds_acl_srcmac_mask_t srcmac_mask;          /* Source Mac Address*/
  ds_acl_dstmac_mask_t dstmac_mask;          /* Destination Mac Address */
  ds_acl_outervlan_mask_t outervlan_mask;    /* Outer Vlan */
  ds_acl_srcip_mask_t srcip_mask;            /* Source IPv4 Address */
  ds_acl_dstip_mask_t dstip_mask;            /* Destination IPv4 Address */
  ds_acl_srcip6_mask_t srcip6_mask;          /* Source IPv6 Address */
  ds_acl_dstip6_mask_t dstip6_mask;          /* Destination IPv6 Address */
  ds_acl_portclass_mask_t portclass_mask;    /* Port Class */
  ds_acl_ethertype_mask_t ethertype_mask;    /* Ethernet Type */
  ds_acl_intclassl2_mask_t intclassl2_mask;  /* L2 Interface Class */
  ds_acl_ingstpstate_mask_t ingstpstate_mask; /* Ingress STP State */
  ds_acl_fwdvlanvalid_mask_t fwdvlanvalid_mask; /* Forwarding Vlan Valid */
  ds_acl_l2stationmove_mask_t l2stationmove_mask; /* L2 Station Move */
  ds_acl_ipflags_mask_t ipflags_mask;           /* IP Flags */
  ds_acl_ipproto_mask_t ipproto_mask;           /* IP Protocol */
  ds_acl_ipttl_mask_t ipttl_mask;               /* IP TTL */
  ds_acl_dscp_mask_t dscp_mask;                /* DSCP */ 
  ds_acl_srcclass_mask_t srcclass_mask;        /* Source Class */
  ds_acl_srctrunk_mask_t srctrunk_mask;        /* Source Trunk Mode */
  ds_acl_dsttrunk_mask_t dsttrunk_mask;        /* Destination Trunk Mode */
  ds_acl_tcpcontrol_mask_t tcpcontrol_mask;    /* TCP Control */
  ds_acl_packetres_mask_t packetres_mask;      /* Packet Resolution */
  ds_acl_vlaninfo_valmask_t vlaninfo_valmask;  /* Vlan Info */
  ds_acl_color_mask_t color_mask;              /* Color */
  ds_acl_l2_class_info_mask_t l2_class_info_mask; /* L2 Class info */
  ds_acl_l2format_mask_t l2format_mask;           /* L2 Format */
  ds_acl_inport_mask_t inport_mask;               /* Ingress Single Port */
  ds_acl_inports_mask_t inports_mask;             /* Ingress Ports */
  ds_acl_outport_mask_t outport_mask;             /* Egress Singe Port */
  ds_acl_dstport_mask_t dstport_mask;             /* Destination Port */
  ds_acl_l4srcport_mask_t l4srcport_mask;         /* L4 Source Port */
  ds_acl_l4dstport_mask_t l4dstport_mask;         /* L4 Destination Port */
  ds_acl_ip_frag_mask_t ip_frag_mask;             /* IP Fragmentation */
  ds_acl_iptype_mask_t iptype_mask;               /* IP Type */ 
  ds_acl_icmp_mask_t icmp_mask;                   /* ICMP */
  ds_acl_tunneltype_mask_t tunneltype_mask;       /* Tunnel Type */

} ds_acl_qualifier_mask_t;

#endif
