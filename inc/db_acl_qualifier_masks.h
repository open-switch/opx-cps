
#ifndef __DB_ACL_QUAL_MASKS_H
#define __DB_ACL_QUAL_MASKS_H

typedef struct 
{
    unsigned int data;
    unsigned int mask;
}db_acl_valmask32;

typedef struct 
{
    unsigned short data;
    unsigned short mask;
}db_acl_valmask16;


typedef struct 
{
    unsigned char data;
    unsigned char mask;
}acl_valmask8;

typedef struct ip_info
{
    unsigned int ipaddr;
    unsigned int ipmask;
}ipinfo;

typedef struct mac_info
{
    unsigned char macaddr[6];
    unsigned char macmask[6];
}macinfo;

typedef struct _vlaninfo
{
    hal_acl_vlan_t data;
    hal_acl_vlan_t mask;
}vlaninfo;


typedef struct _ports
{
    hal_acl_port_t data;
    hal_acl_port_t mask;
}acl_port_t;

typedef struct ip_flag
{
    uint8_t data;
    uint8_t mask;
}ipflag_t;



#endif
