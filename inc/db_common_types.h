/** OPENSOURCELICENSE */
/*
 * db_common_types.h
 */

#ifndef DB_COMMON_TYPES_H_
#define DB_COMMON_TYPES_H_

#include <stdint.h>

/**
 * The length in octets of an V6 IP address.
 */
#define HAL_INET6_LEN (16)

/**
 * Defined by the standard as 0000:0000:0000:0000:0000:0000:127.127.127.127.
 * tools.ietf.org/html/rfc4291, And this is 46 chars (including trailing null)
 * Alternate for INET6_ADDRSTRLEN (linux)
 */
#define HAL_INET6_TEXT_LEN (46)

/**
 * The length in octets of an V4 IP address.
 */
#define HAL_INET4_LEN (4)

/**
 * This the size of an ethernet address.  There are OS defines for this field but to make
 * The HAL OS independant it has been redefined
 */
#define HAL_MAC_ADDR_LEN (6)

/**
 * Represents the IFNAMSIZ define from if.h in linux.  Currently kept separate to maintain a difference between
 * the OS and Dell software
 */
#define HAL_IF_NAME_SZ (16)

/** Type used to refer to a physical NPU in the lower HAL API */
typedef int32_t npu_id_t;

/** Type used to refer to a generic port NPU or not */
typedef uint32_t port_t;

/** Type used to refer to a physical NPU port in the lower HAL API */
typedef port_t npu_port_t;

/** The type of an Ethernet address */
typedef uint8_t hal_mac_addr_t[HAL_MAC_ADDR_LEN];

/**
 * Interface index
 */
typedef int hal_ifindex_t;

/**
 * The VLAN ID type used for all lower hal operations
 */
typedef unsigned int hal_vlan_id_t;

/**
 * HAL VRF ID type
 */
typedef unsigned int hal_vrf_id_t;

/**
 * This is the object type of anything contained within the database.
 * this object type has no instance or key data within it.. it is
 * just a generic object type.  To create an object ID, use the API
 * in db_object_catagory.h.
 */
typedef uint64_t db_object_type_t;

/**
 * This is the object subtype.  There are two elements to an object ID
 * The object catagory and object subtype.
 */
typedef uint32_t db_object_sub_type_t;


#endif /* DB_COMMON_TYPES_H_ */
