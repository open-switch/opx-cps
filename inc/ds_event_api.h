/*
 * filename: db_event_api.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
#ifndef __DS_EVENT_API_H
#define __DS_EVENT_API_H
/**
 * The following file compliments the ds_event_api.h header file.
 */

#include "std_event_service.h"
#include "ds_event_channel.h"

/**
 * @brief Create the HAL event service process
 * @return standard event return code
 */
t_std_error ds_event_service_init(void);

/**
 * @brief register a callback with the HAL event service.  remember that this will be executing in the context
 * of the event service itself so please only important callbacks
 * @param reg the registration struct
 * @return standard return code
 */
t_std_error ds_events_register_cb(std_event_srv_reg_t *reg);

/**
 * @brief connect to the event service - use the send and receive events from the std_event_service.h
 * @param handle the handle that is returned
 * @return standard return code
 */
t_std_error ds_event_service_client_connect(std_event_client_handle * handle);


/**
 * @brief push an evnet through the event service directly without the use of a client handle.  Will be a blocking
 * call until all of the clients have received the message.  If this is not desired use hal_evnet_service_client_connect
 * and and send a message using the created handle.
 *
 * @param msg the message to send
 * @return standard return code
 */
t_std_error ds_event_service_publish_direct_event( std_event_msg_t *msg ) ;


/**
 * Send an event to the event service for publishing using a previously registered handle
 * @param handle the handle to the hal event service
 * @param msg the message to send
 * @return standard return code
 */
static inline t_std_error ds_event_service_publish_msg(std_event_client_handle handle, std_event_msg_t *msg) {
    return std_client_publish_msg(handle,msg);
}

#endif
