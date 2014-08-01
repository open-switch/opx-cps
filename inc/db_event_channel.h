/** OPENSOURCELICENSE */
/*
 * db_event_channel.h
 *
 */

#ifndef DB_EVENT_CHANNEL_H_
#define DB_EVENT_CHANNEL_H_

#include "std_error_codes.h"

typedef enum hal_event_interface_types {
    hal_event_interface_INTERFACE,
    hal_event_interface_INTERFACE_ADDR,
}hal_event_interface_t;

typedef enum hal_event_route_types {
    hal_event_route_ROUTE,
    hal_event_route_NEIBH,
}hal_event_route_t;

typedef enum hal_event_qos_types {
     hal_event_qos_qdisc,
    hal_event_qos_class,
    hal_event_qos_filter,
}hal_event_qos_t;


t_std_error db_event_channel_init(void);

#endif /* DB_EVENT_CHANNEL_H_ */
