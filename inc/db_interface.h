/** OPENSOURCELICENSE */
/*
 * db_interface.h
 */

#ifndef DB_INTERFACE_H_
#define DB_INTERFACE_H_

#include "db_interface_types.h"
#include "db_common_types.h"
#include "std_error_codes.h"
#include "db_common_list.h"

typedef enum {
    db_int_obj_INTERFACE,
    db_int_obj_ROUTE,
    db_int_obj_IP,
    db_int_obj_NEIGHBOUR,
}db_interface_attr_types_t ;

typedef enum {
    db_interface_key_IFNAME,
    db_interface_key_INDEX,
} db_interface_key_type;

typedef struct {
    db_interface_key_type type;
    union {
        hal_ifindex_t index;
        hal_ifname_t name;
    };
};

/**
 * Copy an interface name from in to out.  Handles non-terminated strings
 * @param out destination of ifname
 * @param in source of ifname
 */
void db_interface_ifname_copy(hal_ifname_t *out,const hal_ifname_t* const in);

t_std_error db_interface_mtu_set(hal_ifindex_t ifIndex, unsigned int mtu);
t_std_error db_interface_admin_state_set(hal_ifindex_t ifIndex, db_interface_state_t state);
t_std_error db_interface_mac_addr_set(hal_ifindex_t ifIndex, hal_mac_addr_t *macAddr);

t_std_error db_interface_mtu_get(hal_ifindex_t ifIndex, unsigned int *mtu);
t_std_error db_interface_admin_state_get(hal_ifindex_t ifIndex, db_interface_state_t *state);
t_std_error db_interface_mac_addr_get(hal_ifindex_t ifIndex, hal_mac_addr_t *macAddr);


void db_interface_print(db_common_list_t list);

#endif /* DB_INTERFACE_H_ */
