
#ifndef __DB_ACL_QUAL_MASKS_H__
#define __DB_ACL_QUAL_MASKS_H__

typedef struct 
{
    uint32_t data;
    uint32_t mask;
}db_acl_valmask32_t;

typedef struct 
{
    uint16_t data;
    uint16_t mask;
}db_acl_valmask16_t;

typedef struct 
{
    uint8_t data;
    uint8_t mask;
}db_acl_valmask8_t;

/* MAC INFO */
typedef struct
{
    uint8_t data[6];
    uint8_t mask[6];
}db_acl_macinfo_valmask_t;

/* IP ADDR */
typedef struct 
{
    uint32_t data;
    uint32_t mask;
}db_acl_ipaddr_valmask_t;

/* IPv6 ADDR */
typedef struct 
{
    uint8_t data[16];
    uint8_t mask[16];
}db_acl_ip6addr_valmask_t;

/* SRC MAC */
typedef db_acl_macinfo_valmask_t db_acl_srcmac_mask_t;

/* DST MAC */
typedef db_acl_macinfo_valmask_t db_acl_dstmac_mask_t;

/* ETHER TYPE */
typedef db_acl_valmask8_t db_acl_ethertype_mask_t;

/* VLAN INFO */
typedef db_acl_valmask16_t db_acl_vlaninfo_valmask_t; 

/* COLOR */
typedef enum
{
    GREEN = 1,
    YELLOW,
    RED
}db_acl_color_e;

/* COLOR */
typedef db_acl_valmask32_t db_acl_color_mask_t;

/* OUTER VLAN */
typedef db_acl_vlaninfo_valmask_t db_acl_outervlan_mask_t;

/* L2 CLASS INFO */
typedef db_acl_valmask16_t db_acl_l2_class_info_mask_t;

/* INTERFACE L2 CLASS */
typedef db_acl_valmask8_t db_acl_intclassl2_mask_t;

/* INGRESS STP STATE */
typedef db_acl_valmask8_t db_acl_ingstpstate_mask_t;

/* FWD VLAN VALID */
typedef db_acl_valmask8_t db_acl_fwdvlanvalid_mask_t;

/* L2 STATION MOVE */
typedef db_acl_valmask8_t db_acl_l2stationmove_mask_t;

/* Qualify packets based on L2 header format.*/
typedef enum {
    db_acl_L2FORMATANY,            /* Do not qualify on L2 format. */
    db_acl_L2FORMATETHII,          /* Ethernet 2 (802.2). */
    db_acl_L2FORMATSNAP,           /* Sub-Network Access Protocol (SNAP). */
    db_acl_L2FORMATLLC,            /* Logical Link Control. */
    db_acl_L2FORMAT802DOT3,        /* 802.3 frame format. */
    db_acl_L2FORMATSNAPPRIVATE,    /* Sub-Network Access Protocol (SNAP).
                                       Vendor specific protocol. */
    db_acl_L2FORMATMIM,            /* MAC-In-MAC. */
    db_acl_L2FORMATPPPOE,          /* PPPoE frame. */
    db_acl_L2FORMATCOUNT           /* Always Last. Not a usable value. */
} db_acl_l2format_e;


/* L2 FORMAT */
typedef db_acl_valmask32_t db_acl_l2format_mask_t;

/* IN PORT */
typedef db_acl_valmask32_t db_acl_inport_mask_t;

/* OUT PORT */
typedef db_acl_valmask32_t db_acl_outport_mask_t;

/* SRC IP */
typedef db_acl_ipaddr_valmask_t db_acl_srcip_mask_t;

/* DST IP */
typedef db_acl_ipaddr_valmask_t db_acl_dstip_mask_t;

/* SRC IP6 */
typedef db_acl_ip6addr_valmask_t db_acl_srcip6_mask_t;

/* DST IP6 */
typedef db_acl_ip6addr_valmask_t db_acl_dstip6_mask_t;

/* IP FLAGS */
typedef db_acl_valmask8_t db_acl_ipflags_mask_t;

/* IP PROTO */
typedef db_acl_valmask8_t db_acl_ipproto_mask_t;

/* IP TTL */
typedef db_acl_valmask8_t db_acl_ipttl_mask_t;

/* DST PORT */
typedef db_acl_valmask32_t db_acl_dstport_mask_t;

/* L4 SRC PORT */
typedef db_acl_valmask32_t db_acl_l4srcport_mask_t;

/* L4 DST PORT */
typedef db_acl_valmask32_t db_acl_l4dstport_mask_t;

/* DSCP */
typedef db_acl_valmask8_t db_acl_dscp_mask_t;

/* IP FRAG */
typedef enum {
    db_acl_IP_FRAG_NON,            /* Non-fragmented packet. */
    db_acl_IP_FRAG_FIRST,          /* First fragment of fragmented packet. */
    db_acl_IP_FRAG_NON_OR_FIRST,   /* Non-fragmented or first fragment. */
    db_acl_IP_FRAG_NOT_FIRST,      /* Not the first fragment. */
    db_acl_IP_FRAG_ANY,            /* Any fragment of fragmented packet. */
    db_acl_IP_FRAG_COUNT           /* Always last. Not a usable value. */
}db_acl_ip_frag_mask_e;

/* IP FRAG */
typedef db_acl_valmask32_t db_acl_ip_frag_mask_t;

/* IP Type */
typedef enum {
    db_acl_IP_TYPE_ANY,              /* Don't care. */
    db_acl_IP_TYPE_NON_IP,           /* Non-Ip packet. */
    db_acl_IP_TYPE_IPV4_NOT,         /* Anything but IPv4 packets. */
    db_acl_IP_TYPE_IPV4_NO_OPTS,     /* IPv4 without options. */
    db_acl_IP_TYPE_IPV4_WITH_OPTS,   /* IPv4 with options. */
    db_acl_IP_TYPE_IPV4_ANY,         /* Any IPv4 packet. */
    db_acl_IP_TYPE_IPV6_NOT,         /* Anything but IPv6 packets. */
    db_acl_IP_TYPE_IPV6_NO_EXT_HDR,  /* IPv6 packet without any extension header. */
    db_acl_IP_TYPE_IPV6_ONE_EXT_HDR, /* IPv6 packet with one extension header. */
    db_acl_IP_TYPE_IPV6_TWO_EXT_HDR, /* IPv6 packet with two or more extension
                                              headers. */
    db_acl_IP_TYPE_IPV6,             /* IPv6 packet. */
    db_acl_IP_TYPE_IP,               /* IPv4 and IPv6 packets. */
    db_acl_IP_TYPE_ARP,              /* ARP/RARP. */
    db_acl_IP_TYPE_ARP_REQUEST,      /* ARP Request. */
    db_acl_IP_TYPE_ARP_REPLY,        /* ARP Reply. */
    db_acl_IP_TYPE_MPLS_UNICAST,     /* Mpls unicast frame (ethertype = 0x8847). */
    db_acl_IP_TYPE_MPLS_MULTICAST,   /* Mpls mcast frame   (ethertype = 0x8848). */
    db_acl_IP_TYPE_TRILL,            /* Trill packet. */
    db_acl_IP_TYPE_MIM,              /* Mac-in-Mac frame. */
    db_acl_IP_TYPE_MPLS,             /* MPLS Packets. */
    db_acl_IP_TYPE_CFM,              /* CFM Packets (0x8902). */
    db_acl_IP_TYPE_FCOE,             /* Fiber Channel Packets (0x8906). */
    db_acl_IP_TYPE_COUNT             /* Always Last. Not a usable value. */
} db_acl_ip_type_e;

/* IP TYPE */
typedef db_acl_valmask32_t db_acl_ip_type_mask_t;

/* SRC CLASS */
typedef db_acl_valmask8_t db_acl_srcclass_mask_t;

/* ICMP */
typedef db_acl_valmask32_t db_acl_icmp_mask_t;

/* SRC TRUNK */
typedef db_acl_valmask8_t db_acl_srctrunk_mask_t;

/* DST TRUNK */
typedef db_acl_valmask8_t db_acl_dsttrunk_mask_t;

/* PORT CLASS */
typedef db_acl_ipaddr_valmask_t db_acl_portclass_mask_t;

/* TCP CONTROL */
typedef db_acl_valmask8_t db_acl_tcpcontrol_mask_t;

/* TUNNEL TYPE */ 
typedef enum {
    db_acl_TUNNEL_TYPE_ANY,                /* Don't care. */
    db_acl_TUNNEL_TYPE_NONE,               /* L2 termination. */
    db_acl_TUNNEL_TYPE_IP,                 /* IP in IP, Istap, GRE. */
    db_acl_TUNNEL_TYPE_MPLS,               /* MPLS. */
    db_acl_TUNNEL_TYPE_MIM,                /* Mac in Mac. */
    db_acl_TUNNEL_TYPE_WLAN_WTP_TO_AC,     /* WLAN access point to access control. */
    db_acl_TUNNEL_TYPE_WLAN_AC_TO_AC,      /* WLAN access control to access
                                                    control. */
    db_acl_TUNNEL_TYPE_AUTO_MULTICAST,     /* IPV4 Automatic multicast. */
    db_acl_TUNNEL_TYPE_TRILL,              /* Trill. */
    db_acl_TUNNEL_TYPE_L2GRE,              /* L2 GRE. */
    db_acl_TUNNEL_TYPE_IP6,                /* IPv6 termination. */
    db_acl_TUNNEL_TYPE_MPLS_CONTROL_WORD,  /* MPLS with Control Word terminated. */
    db_acl_TUNNEL_TYPE_MPLS_LABEL2,        /* 2 MPLS labels terminated. */
    db_acl_TUNNEL_TYPE_MPLS_LABEL2_CONTROL_WORD, /* 2 MPLS labels with Control Word
                                                          terminated. */
    db_acl_TUNNEL_TYPE_MPLS_LABEL3,        /* 3 MPLS labels terminated. */
    db_acl_TUNNEL_TYPE_MPLS_LABEL3_CONTROL_WORD, /* 3 MPLS labels with Control Word
                                                          terminated. */
    db_acl_TUNNEL_TYPE_VXLAN,            /* Vxlan Tunnel Packet. */
    db_acl_TUNNEL_TYPE_COUNT             /* Always Last. Not a usable value. */
}db_acl_tunnel_type_e;

/* TUNNEL TYPE */
typedef db_acl_valmask32_t db_acl_tunnel_type_mask_t;

/* Packet Result */
typedef db_acl_valmask8_t db_acl_packetres_mask_t;

#endif /*__DB_ACL_QUAL_MASKS_H__ */
