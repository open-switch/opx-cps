/*
 * cps_class_ut_data.h
 *
 *  Created on: May 24, 2016
 *      Author: cwichmann
 */

#ifndef CPS_API_SRC_UNIT_TEST_CPS_CLASS_UT_DATA_H_
#define CPS_API_SRC_UNIT_TEST_CPS_CLASS_UT_DATA_H_

/*
 * Data used in multiple unit tests
 * */

#define cps_api_obj_CAT_BASE_IP (34)

#define DELL_BASE_IP_MODEL_STR "dell-base-ip"


/* Object base-ip/ipv6/address */

typedef enum {
/*IP address*/
/*type=binary*/
  BASE_IP_IPV6_ADDRESS_IP_BASE_CMN_IPV4_ADDRESS = 2228248,
/*type=binary*/
  BASE_IP_IPV6_ADDRESS_IP_BASE_CMN_IPV6_ADDRESS = 2228249,
  BASE_IP_IPV6_ADDRESS_IP = 2228225,

/*type=uint32*/
  BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH = 2228227,
} BASE_IP_IPV6_ADDRESS_t;
/* Object base-ip/ipv4/address */

typedef enum {
/*IP address*/
/*type=binary*/
  BASE_IP_IPV4_ADDRESS_IP_BASE_CMN_IPV4_ADDRESS = 2228250,
/*type=binary*/
  BASE_IP_IPV4_ADDRESS_IP_BASE_CMN_IPV6_ADDRESS = 2228251,
  BASE_IP_IPV4_ADDRESS_IP = 2228228,

/*type=uint8*/
  BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH = 2228230,
} BASE_IP_IPV4_ADDRESS_t;
/* Object base-ip/ipv6 */

typedef enum {
/*The interface index*/
/*type=uint32*/
  BASE_IP_IPV6_IFINDEX = 2228231,
/*This is true if IPv4 is enabled.*/
/*type=boolean*/
  BASE_IP_IPV6_ENABLED = 2228232,
/*This will be set true if the system has IP forwarding enabled.
This will only set the kernel IP forwarding flags.*/
/*type=boolean*/
  BASE_IP_IPV6_FORWARDING = 2228233,
/*A numerical value of the vrf that contains the interface.  Use 0 for the default.*/
/*type=uint32*/
  BASE_IP_IPV6_VRF_ID = 2228244,
/*The interfaces's name.*/
/*type=string*/
  BASE_IP_IPV6_NAME = 2228245,

/*type=uint32*/
  BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS = 2228243,

/*type=binary*/
  BASE_IP_IPV6_ADDRESS = 2228235,
} BASE_IP_IPV6_t;
/* Object base-ip/ipv4 */

typedef enum {
/*The interface index*/
/*type=uint32*/
  BASE_IP_IPV4_IFINDEX = 2228236,
/*This is true if IPv4 is enabled.*/
/*type=boolean*/
  BASE_IP_IPV4_ENABLED = 2228237,
/*This will be set true if the system has IP forwarding enabled.
This will only set the kernel IP forwarding flags.*/
/*type=boolean*/
  BASE_IP_IPV4_FORWARDING = 2228238,
/*A numerical value of the vrf that contains the interface.  Use 0 for the default.*/
/*type=uint32*/
  BASE_IP_IPV4_VRF_ID = 2228246,
/*The interfaces's name.*/
/*type=string*/
  BASE_IP_IPV4_NAME = 2228247,

/*type=binary*/
  BASE_IP_IPV4_ADDRESS = 2228240,
} BASE_IP_IPV4_t;

/* Object's continued */

typedef enum{
  BASE_IP_IPV4 = 2228241,
  BASE_IP_IPV4_OBJ = 2228241,

  BASE_IP_IPV6 = 2228242,
  BASE_IP_IPV6_OBJ = 2228242,

} BASE_IP_OBJECTS_t;


#include <vector>

static struct {
  std::vector<cps_api_attr_id_t> _ids;
  cps_api_attr_id_t id;
  cps_class_map_node_details details;
} lst[] = {
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP }, BASE_IP_IPV6_ADDRESS, { "base-ip/ipv6/address", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP,BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH }, BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH, { "base-ip/ipv4/address/prefix-length", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT8 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_FORWARDING }, BASE_IP_IPV6_FORWARDING, { "base-ip/ipv6/forwarding", "This will be set true if the system has IP forwarding enabled.This will only set the kernel IP forwarding flags.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP }, cps_api_obj_CAT_BASE_IP, { "base-ip", "This model will support the configruation of IP address.***Depreciated****", true, CPS_CLASS_ATTR_T_CONTAINER, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX }, BASE_IP_IPV6, { "base-ip/ipv6", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX }, BASE_IP_IPV4, { "base-ip/ipv4", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_NAME }, BASE_IP_IPV6_NAME, { "base-ip/ipv6/name", "The interfaces's name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_NAME }, BASE_IP_IPV4_NAME, { "base-ip/ipv4/name", "The interfaces's name.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_STRING }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_VRF_ID }, BASE_IP_IPV4_VRF_ID, { "base-ip/ipv4/vrf-id", "A numerical value of the vrf that contains the interface.  Use 0 for the default.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_FORWARDING }, BASE_IP_IPV4_FORWARDING, { "base-ip/ipv4/forwarding", "This will be set true if the system has IP forwarding enabled.This will only set the kernel IP forwarding flags.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_VRF_ID }, BASE_IP_IPV6_VRF_ID, { "base-ip/ipv6/vrf-id", "A numerical value of the vrf that contains the interface.  Use 0 for the default.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_IFINDEX }, BASE_IP_IPV6_IFINDEX, { "base-ip/ipv6/ifindex", "The interface index", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP }, BASE_IP_IPV4_ADDRESS, { "base-ip/ipv4/address", "", true, CPS_CLASS_ATTR_T_LIST, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV4_ADDRESS_IP,BASE_IP_IPV4_ADDRESS_IP }, BASE_IP_IPV4_ADDRESS_IP, { "base-ip/ipv4/address/ip", "IP address", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP,BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH }, BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH, { "base-ip/ipv6/address/prefix-length", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ADDRESS,BASE_IP_IPV6_ADDRESS_IP,BASE_IP_IPV6_ADDRESS_IP }, BASE_IP_IPV6_ADDRESS_IP, { "base-ip/ipv6/address/ip", "IP address", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BIN }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_ENABLED }, BASE_IP_IPV4_ENABLED, { "base-ip/ipv4/enabled", "This is true if IPv4 is enabled.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_ENABLED }, BASE_IP_IPV6_ENABLED, { "base-ip/ipv6/enabled", "This is true if IPv4 is enabled.", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_BOOL }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV6,BASE_IP_IPV6_VRF_ID,BASE_IP_IPV6_IFINDEX,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS }, BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS, { "base-ip/ipv6/dup-addr-detect-transmits", "", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
{ { cps_api_obj_CAT_BASE_IP,BASE_IP_IPV4,BASE_IP_IPV4_VRF_ID,BASE_IP_IPV4_IFINDEX,BASE_IP_IPV4_IFINDEX }, BASE_IP_IPV4_IFINDEX, { "base-ip/ipv4/ifindex", "The interface index", false, CPS_CLASS_ATTR_T_LEAF, CPS_CLASS_DATA_TYPE_T_UINT32 }},
};

static const size_t lst_len = sizeof(lst)/sizeof(*lst);


#endif /* CPS_API_SRC_UNIT_TEST_CPS_CLASS_UT_DATA_H_ */
