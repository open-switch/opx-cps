/* OPENSOURCELICENSE */
/*
 * cps_api_route.h
 */

#ifndef cps_api_route_H_
#define cps_api_route_H_

typedef enum {
    cps_api_route_obj_ROUTE=1,
    cps_api_route_obj_NEIBH
}cps_api_route_sub_category_t;


//cps_api_route_obj_NEIBH
typedef enum {
    cps_api_if_NEIGH_A_FAMILY=0, //uint32_t
    cps_api_if_NEIGH_A_OPERATION=1, //db_nbr_event_type_t
    cps_api_if_NEIGH_A_NBR_ADDR=2, //hal_ip_addr_t
    cps_api_if_NEIGH_A_NBR_MAC=3, //hal_mac_addr_t
    cps_api_if_NEIGH_A_IFINDEX=4, //hal_ifindex_t
    cps_api_if_NEIGH_A_PHY_IFINDEX=5, //hal_ifindex_t
    cps_api_if_NEIGH_A_VRF=7, //uint32_t
    cps_api_if_NEIGH_A_EXPIRE=8, //uint32_t
    cps_api_if_NEIGH_A_FLAGS=9, //uint32_t
    cps_api_if_NEIGH_A_STATE=10, //uint32_t
}cps_api_if_NEIGH_ATTR;

//cps_api_int_obj_ROUTE
typedef enum {
    cps_api_if_ROUTE_A_MSG_TYPE=0, //db_route_msg_t
    cps_api_if_ROUTE_A_DISTANCE=1, //uint32_t
    cps_api_if_ROUTE_A_PROTOCOL=2, //uint32_t
    cps_api_if_ROUTE_A_VRF=3, //uint32_t
    cps_api_if_ROUTE_A_PREFIX=4, //hal_ip_addr_t
    cps_api_if_ROUTE_A_PREFIX_LEN=5, //uint32_t
    cps_api_if_ROUTE_A_NH_IFINDEX=6, //uint32_t
    cps_api_if_ROUTE_A_NEXT_HOP_VRF=7, //uint32_t
    cps_api_if_ROUTE_A_NEXT_HOP_ADDR=8, //hal_ip_addr_t
}cps_api_if_ROUTE_ATTR;

#if 0
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
#endif



#if 0
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
#endif

#endif /* cps_api_route_H_ */
