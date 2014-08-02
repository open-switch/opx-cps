/** OPENSOURCELICENSE */
/*
 * db_event_channel.h
 *
 */

#ifndef DB_EVENT_CHANNEL_H_
#define DB_EVENT_CHANNEL_H_

#include "std_error_codes.h"
#include "std_event_service.h"

typedef enum hal_event_types_e {
    hal_event_t_TEST=STD_EVENT_FIST_CLASS_ID,
    hal_event_t_INTERFACE,
    hal_event_t_ROUTE,
    hal_event_t_EVENT,
    hal_event_t_SFP,
    hal_event_t_QOS,
    hal_event_t_MAXCLASS
}hal_event_types_t;


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

void db_event_print(std_event_msg_t *evt);

t_std_error db_event_channel_init(void);

#endif /* DB_EVENT_CHANNEL_H_ */
