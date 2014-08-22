
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

/* IP ADDR */
typedef struct 
{
    uint32_t ipaddr;
    uint32_t ipmask;
}db_acl_ipaddr_mask_t;

/* IPv6 ADDR */
typedef struct 
{
    uint16_t ipaddr[6];
    uint16_t ipmask[6];
}db_acl_ipaddr_mask_t;



/* MAC INFO */
typedef struct
{
    uint8_t macaddr[6];
    uint8_t macmask[6];
}db_acl_macinfo_mask_t;

/* VLAN INFO */
typedef struct
{
    uint16_t vlanid;
    uint16_t vlanmask;
}db_acl_vlaninfo_mask_t;

/* PORT */
typedef struct
{
    uint32_t portid;
    uint32_t portmask;
}db_acl_port_mask_t;

/* IP FLAG */
typedef struct
{
    uint8_t ipflag;
    uint8_t ipflagmask;
}db_acl_ipflag_mask_t;

/* ETHER TYPE */
typedef uint16_t db_acl_ethertype_t;

/* COLOR */

typedef enum
{
    GREEN = 1,
    YELLOW,
    RED
}db_acl_color_t;

/* L2 COLOR */

typedef struct
{
    uint8_t l2_class_id;
    uint8_t l2_class_mask;
}db_acl_l2_class_info_mask_t;

/* IP PROTO */

typedef struct
{
    uint8_t ipproto;
    uint8_t ipprotomask;
}db_acl_ipproto_mask_t;

/* IP TTL */

typedef struct 
{
    uint8_t ipttl;
    uint8_t ipttlmask;
}db_acl_ipttl_mask_t;

/* L4 PORT*/
typedef struct 
{
    uint32_t l4port;
    uint32_t l4portmask;
}hal_low_acl_l4port_mask_t;

/* DSCP */
typedef struct 
{
    uint8_t dscp;
    uint8_t dscpmask;
}db_acl_dscp_mask_t;

/* INGRESS STP STATE */
typedef struct 
{
    uint8_t ingstpstate;
    uint8_t ingstpstatemask;
}db_acl_ingstpstate_mask_t;


/* FWD VLAN VALID */
typedef struct 
{
    uint8_t fwdvlanvalid;
    uint8_t fwdvlanvalidmask;
}db_acl_fwdvlanvalid_mask_t;

/* IP FRAG */

typedef enum {
    hal_acl_tcam_ip_frag_non,            /* Non-fragmented packet. */
    hal_acl_tcam_ip_frag_first,          /* First fragment of fragmented packet. */
    hal_acl_tcam_ip_frag_non_or_first,   /* Non-fragmented or first fragment. */
    hal_acl_tcam_ip_frag_not_first,      /* Not the first fragment. */
    hal_acl_tcam_ip_frag_any,            /* Any fragment of fragmented packet. */
    hal_acl_tcam_ip_frag_count           /* Always last. Not a usable value. */
} db_acl_ip_frag_mask_t;

/* SRC CLASS */
typedef struct 
{
    uint8_t srcclass;
    uint8_t srcclassmask;
}db_acl_srcclass_mask_t;

/* ICMP */
typedef struct 
{
    uint32_t icmp;
    uint32_t icmpmask;
}db_acl_icmp_mask_t;

/* SRC TRUNK */

typedef struct 
{
    uint8_t srctrunk;
    uint8_t srctrunkmask;
}db_acl_srctrunk_mask_t;

/* DST TRUNK */

typedef struct 
{
    uint8_t dsttrunk;
    uint8_t dsttrunkmask;
}db_acl_dsttrunk_mask_t;

// TODO:MANI: 














#endif
