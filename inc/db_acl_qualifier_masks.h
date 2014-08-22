
#ifndef __DB_ACL_QUAL_MASKS_H
#define __DB_ACL_QUAL_MASKS_H

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
    uint8_t macaddr[6];
    uint8_t macmask[6];
}db_acl_macinfo_valmask_t;

/* VLAN INFO */
typedef db_acl_valmask16_t db_acl_vlaninfo_valmask_t; 


/* IP ADDR */
typedef struct 
{
    uint32_t ipaddr;
    uint32_t ipmask;
}db_acl_ipaddr_valmask_t;

/* IPv6 ADDR */
typedef struct 
{
    uint16_t ipaddr[6];
    uint16_t ipmask[6];
}db_acl_ip6addr_valmask_t;

/* SRC MAC */
typedef db_acl_macinfo_valmask_t db_acl_srcmac_mask_t;

/* DST MAC */
typedef db_acl_macinfo_valmask_t db_acl_dstmac_mask_t;

/* ETHER TYPE */
typedef uint16_t db_acl_ethertype_t;

/* COLOR */
typedef enum
{
    GREEN = 1,
    YELLOW,
    RED
}db_acl_color_t;

/* INTERFACE L2 CLASS */
typedef db_acl_valmask8_t db_acl_intclassl2_mask_t;

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


/* DST PORT */
typedef db_acl_valmask32_t db_acl_dstport_mask_t;

/* OUTER VLAN */
typedef db_acl_vlaninfo_valmask_t db_acl_outervlan_mask_t;


/* L4 SRC PORT */
typedef db_acl_valmask32_t db_acl_l4srcport_mask_t;

/* L4 DST PORT */
typedef db_acl_valmask32_t db_acl_l4dstport_mask_t;

/* L2 COLOR */
typedef db_acl_valmask16_t db_acl_l2_class_info_mask_t;

/* IP PROTO */
typedef db_acl_valmask8_t db_acl_ipproto_mask_t;

/* IP TTL */
typedef db_acl_valmask8_t db_acl_ipttl_mask_t;

/* DST PORT*/
typedef db_acl_valmask32_t db_acl_dstport_mask_t;

/* DSCP */
typedef db_acl_valmask8_t db_acl_dscp_mask_t;

/* INGRESS STP STATE */
typedef db_acl_valmask8_t db_acl_ingstpstate_mask_t;

/* FWD VLAN VALID */
typedef db_acl_valmask8_t db_acl_fwdvlanvalid_mask_t;

/* IP FRAG */
typedef enum {
    db_acl_IP_FRAG_NON,            /* Non-fragmented packet. */
    db_acl_IP_FRAG_FIRST,          /* First fragment of fragmented packet. */
    db_acl_IP_FRAG_NON_OR_FIRST,   /* Non-fragmented or first fragment. */
    db_acl_IP_FRAG_NOT_FIRST,      /* Not the first fragment. */
    db_acl_IP_FRAG_ANY,            /* Any fragment of fragmented packet. */
    db_acl_IP_FRAG_COUNT           /* Always last. Not a usable value. */
}db_acl_ip_frag_mask_t;

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
}db_acl_tunnel_type_t;


/* L2 STATION MOVE */
typedef db_acl_valmask8_t db_acl_l2stationmove_mask_t;

/* TCP CONTROL */
typedef db_acl_valmask8_t db_acl_tcpcontrol_mask_t;

#endif /*__DB_ACL_QUAL_MASKS_H */
