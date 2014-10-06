/**
 * filename: ds_acl_qualifier_masks.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 **/

/** OPENSOURCELICENSE */

#ifndef __DS_ACL_QUAL_MASKS_H__
#define __DS_ACL_QUAL_MASKS_H__

#include "ds_common_types.h"

/*
 * ds_acl_valmask(8/16/32)_t are generic struct having data types of 8/16/32
 * unsigned int.Most of the specific maks types (i.e., vlaninfo, inport, etc.,)
 *  are aliases of generic valmask struct type.
 */


/* Generic data type*/
typedef struct
{
    uint32_t data;
    uint32_t mask;
}ds_acl_valmask32_t;

/* Generic data type*/
typedef struct
{
    uint16_t data;
    uint16_t mask;
}ds_acl_valmask16_t;

/* Generic data type*/
typedef struct
{
    uint8_t data;
    uint8_t mask;
}ds_acl_valmask8_t;

/* MAC INFO */
typedef struct
{
    hal_mac_addr_t data;
    hal_mac_addr_t mask;
}ds_acl_macinfo_valmask_t;

/* IP ADDR */
typedef struct
{
    uint32_t data;
    uint32_t mask;
}ds_acl_ipaddr_valmask_t;

/* IPv6 ADDR */
typedef struct
{
    uint8_t data[HAL_INET6_LEN];
    uint8_t mask[HAL_INET6_LEN];
}ds_acl_ip6addr_valmask_t;

/* SRC MAC */
typedef ds_acl_macinfo_valmask_t ds_acl_srcmac_mask_t;

/* DST MAC */
typedef ds_acl_macinfo_valmask_t ds_acl_dstmac_mask_t;

/* ETHER TYPE */
typedef ds_acl_valmask16_t ds_acl_ethertype_mask_t;

/* VLAN INFO */
typedef ds_acl_valmask16_t ds_acl_vlaninfo_valmask_t;

/* COLOR */
typedef enum
{
    GREEN = 1,
    YELLOW,
    RED
}ds_acl_color_e;

/* COLOR */
typedef ds_acl_valmask32_t ds_acl_color_mask_t;

/* OUTER VLAN */
typedef ds_acl_vlaninfo_valmask_t ds_acl_outervlan_mask_t;

/* L2 CLASS INFO */
typedef ds_acl_valmask16_t ds_acl_l2_class_info_mask_t;

/* INTERFACE L2 CLASS */
typedef ds_acl_valmask8_t ds_acl_intclassl2_mask_t;

/* INGRESS STP STATE */
typedef ds_acl_valmask8_t ds_acl_ingstpstate_mask_t;

/* FWD VLAN VALID */
typedef ds_acl_valmask8_t ds_acl_fwdvlanvalid_mask_t;

/* L2 STATION MOVE */
typedef ds_acl_valmask8_t ds_acl_l2stationmove_mask_t;

/* Qualify packets based on L2 header format.*/
typedef enum {
    ds_acl_L2FORMATANY,            /* Do not qualify on L2 format. */
    ds_acl_L2FORMATETHII,          /* Ethernet 2 (802.2). */
    ds_acl_L2FORMATSNAP,           /* Sub-Network Access Protocol (SNAP). */
    ds_acl_L2FORMATLLC,            /* Logical Link Control. */
    ds_acl_L2FORMAT802DOT3,        /* 802.3 frame format. */
    ds_acl_L2FORMATSNAPPRIVATE,    /* Sub-Network Access Protocol (SNAP).
                                       Vendor specific protocol. */
    ds_acl_L2FORMATMIM,            /* MAC-In-MAC. */
    ds_acl_L2FORMATPPPOE,          /* PPPoE frame. */
    ds_acl_L2FORMATCOUNT           /* Always Last. Not a usable value. */
} ds_acl_l2format_e;


/* L2 FORMAT */
typedef ds_acl_valmask32_t ds_acl_l2format_mask_t;

/* IN PORT */
typedef ds_acl_valmask32_t ds_acl_inport_mask_t;

/* IN PORTS */
typedef ds_acl_valmask32_t ds_acl_inports_mask_t;

/* OUT PORT */
typedef ds_acl_valmask32_t ds_acl_outport_mask_t;

/* SRC IP */
typedef ds_acl_ipaddr_valmask_t ds_acl_srcip_mask_t;

/* DST IP */
typedef ds_acl_ipaddr_valmask_t ds_acl_dstip_mask_t;

/* SRC IP6 */
typedef ds_acl_ip6addr_valmask_t ds_acl_srcip6_mask_t;

/* DST IP6 */
typedef ds_acl_ip6addr_valmask_t ds_acl_dstip6_mask_t;

/* IP FLAGS */
typedef ds_acl_valmask8_t ds_acl_ipflags_mask_t;

/* IP PROTO */
typedef ds_acl_valmask8_t ds_acl_ipproto_mask_t;

/* IP TTL */
typedef ds_acl_valmask8_t ds_acl_ipttl_mask_t;

/* DST PORT */
typedef ds_acl_valmask32_t ds_acl_dstport_mask_t;

/* L4 SRC PORT */
typedef ds_acl_valmask32_t ds_acl_l4srcport_mask_t;

/* L4 DST PORT */
typedef ds_acl_valmask32_t ds_acl_l4dstport_mask_t;

/* DSCP */
typedef ds_acl_valmask8_t ds_acl_dscp_mask_t;

/* IP FRAG */
typedef enum {
    ds_acl_IP_FRAG_NON,            /* Non-fragmented packet. */
    ds_acl_IP_FRAG_FIRST,          /* First fragment of fragmented packet. */
    ds_acl_IP_FRAG_NON_OR_FIRST,   /* Non-fragmented or first fragment. */
    ds_acl_IP_FRAG_NOT_FIRST,      /* Not the first fragment. */
    ds_acl_IP_FRAG_ANY,            /* Any fragment of fragmented packet. */
    ds_acl_IP_FRAG_COUNT           /* Always last. Not a usable value. */
}ds_acl_ip_frag_mask_e;

/* IP FRAG */
typedef ds_acl_valmask32_t ds_acl_ip_frag_mask_t;

/* IP Type */
typedef enum {
    ds_acl_IP_TYPE_ANY,              /* Don't care. */
    ds_acl_IP_TYPE_NON_IP,           /* Non-Ip packet. */
    ds_acl_IP_TYPE_IPV4_NOT,         /* Anything but IPv4 packets. */
    ds_acl_IP_TYPE_IPV4_NO_OPTS,     /* IPv4 without options. */
    ds_acl_IP_TYPE_IPV4_WITH_OPTS,   /* IPv4 with options. */
    ds_acl_IP_TYPE_IPV4_ANY,         /* Any IPv4 packet. */
    ds_acl_IP_TYPE_IPV6_NOT,         /* Anything but IPv6 packets. */
    ds_acl_IP_TYPE_IPV6_NO_EXT_HDR,  /* IPv6 packet without any extension header. */
    ds_acl_IP_TYPE_IPV6_ONE_EXT_HDR, /* IPv6 packet with one extension header. */
    ds_acl_IP_TYPE_IPV6_TWO_EXT_HDR, /* IPv6 packet with two or more extension
                                              headers. */
    ds_acl_IP_TYPE_IPV6,             /* IPv6 packet. */
    ds_acl_IP_TYPE_IP,               /* IPv4 and IPv6 packets. */
    ds_acl_IP_TYPE_ARP,              /* ARP/RARP. */
    ds_acl_IP_TYPE_ARP_REQUEST,      /* ARP Request. */
    ds_acl_IP_TYPE_ARP_REPLY,        /* ARP Reply. */
    ds_acl_IP_TYPE_MPLS_UNICAST,     /* Mpls unicast frame (ethertype = 0x8847). */
    ds_acl_IP_TYPE_MPLS_MULTICAST,   /* Mpls mcast frame   (ethertype = 0x8848). */
    ds_acl_IP_TYPE_TRILL,            /* Trill packet. */
    ds_acl_IP_TYPE_MIM,              /* Mac-in-Mac frame. */
    ds_acl_IP_TYPE_MPLS,             /* MPLS Packets. */
    ds_acl_IP_TYPE_CFM,              /* CFM Packets (0x8902). */
    ds_acl_IP_TYPE_FCOE,             /* Fiber Channel Packets (0x8906). */
    ds_acl_IP_TYPE_COUNT             /* Always Last. Not a usable value. */
} ds_acl_iptype_e;

/* IP TYPE */
typedef ds_acl_valmask32_t ds_acl_iptype_mask_t;

/* SRC CLASS */
typedef ds_acl_valmask8_t ds_acl_srcclass_mask_t;

/* ICMP */
typedef ds_acl_valmask16_t ds_acl_icmp_mask_t;

/* SRC TRUNK */
typedef ds_acl_valmask8_t ds_acl_srctrunk_mask_t;

/* DST TRUNK */
typedef ds_acl_valmask8_t ds_acl_dsttrunk_mask_t;

/* PORT CLASS */
typedef ds_acl_ipaddr_valmask_t ds_acl_portclass_mask_t;

/* TCP CONTROL */
typedef ds_acl_valmask8_t ds_acl_tcpcontrol_mask_t;

/* TUNNEL TYPE */
typedef enum {
    ds_acl_TUNNEL_TYPE_ANY,                /* Don't care. */
    ds_acl_TUNNEL_TYPE_NONE,               /* L2 termination. */
    ds_acl_TUNNEL_TYPE_IP,                 /* IP in IP, Istap, GRE. */
    ds_acl_TUNNEL_TYPE_MPLS,               /* MPLS. */
    ds_acl_TUNNEL_TYPE_MIM,                /* Mac in Mac. */
    ds_acl_TUNNEL_TYPE_WLAN_WTP_TO_AC,     /* WLAN access point to access control. */
    ds_acl_TUNNEL_TYPE_WLAN_AC_TO_AC,      /* WLAN access control to access
                                                    control. */
    ds_acl_TUNNEL_TYPE_AUTO_MULTICAST,     /* IPV4 Automatic multicast. */
    ds_acl_TUNNEL_TYPE_TRILL,              /* Trill. */
    ds_acl_TUNNEL_TYPE_L2GRE,              /* L2 GRE. */
    ds_acl_TUNNEL_TYPE_IP6,                /* IPv6 termination. */
    ds_acl_TUNNEL_TYPE_MPLS_CONTROL_WORD,  /* MPLS with Control Word terminated. */
    ds_acl_TUNNEL_TYPE_MPLS_LABEL2,        /* 2 MPLS labels terminated. */
    ds_acl_TUNNEL_TYPE_MPLS_LABEL2_CONTROL_WORD, /* 2 MPLS labels with Control Word
                                                          terminated. */
    ds_acl_TUNNEL_TYPE_MPLS_LABEL3,        /* 3 MPLS labels terminated. */
    ds_acl_TUNNEL_TYPE_MPLS_LABEL3_CONTROL_WORD, /* 3 MPLS labels with Control Word
                                                          terminated. */
    ds_acl_TUNNEL_TYPE_VXLAN,            /* Vxlan Tunnel Packet. */
    ds_acl_TUNNEL_TYPE_COUNT             /* Always Last. Not a usable value. */
}ds_acl_tunneltype_e;

/* TUNNEL TYPE */
typedef ds_acl_valmask32_t ds_acl_tunneltype_mask_t;

/* Packet Result */
typedef ds_acl_valmask8_t ds_acl_packetres_mask_t;

#endif /*__DB_ACL_QUAL_MASKS_H__ */
