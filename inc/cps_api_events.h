/*
 * filename: db_event_api.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/* OPENSOURCELICENSE */
#ifndef __DS_EVENT_API_H
#define __DS_EVENT_API_H

#include "cps_api_key.h"
#include "cps_api_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup CPSAPI The CPS API
 *
      This file consists of the APIs to publish and subscribe to events.
@{
*/

/**
 * handle for the DS event subsystem
 */
typedef void * cps_api_event_service_handle_t;

/**
 * The priority for an cps api event handler callback
 */
typedef unsigned int cps_api_event_reg_prio_t;

/**
 * The message structure sent and received by the API.  The data, will be appended
 * to the message itself.  Use the macros to define a message or allocate one with
 * the API provided.  Use the set data API to fill up the data
 */
typedef struct {
    cps_api_key_t key;
    unsigned int data_len;/** the length of the data */
    unsigned int max_data_len; /**the max buffer space contained by data */
} cps_api_event_header_t;

//Optionally pad the structure so that it always starts on a 4 byte boundary
static inline uint8_t * cps_api_event_msg_data(cps_api_event_header_t *p) {
    return ((uint8_t*)p) + sizeof(*p);
}

/**
 * This is the event API registration structure that provides a priority for the
 * client's registration along with the clients events they want to be covered
 * under the priority.  This could be implemented via separate queues for the client
 * to receive messages on or some other type of implementation.
 */
typedef struct {
    cps_api_event_reg_prio_t priority; //! priority of the registration optional for the implementation
    cps_api_key_t *objects;    //! the objects
    size_t number_of_objects;
} cps_api_event_reg_t;

/**
 * Initialize the internal cps api event library.  This must be called by any process
 * using the cps event API.
 * @return standard event return code
 */
cps_api_return_code_t cps_api_event_service_init(void);

/**
 * @brief initialize to the CPS event forwarding service for to handle events.
 * @param handle the handle that is returned
 * @return standard return code
 */
cps_api_return_code_t cps_api_event_client_connect(cps_api_event_service_handle_t * handle);

/**
 * Deregister with the event service and therefore remove any registration requests
 * destined for the handle
 *
 * @param handle the handle to the event service
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_event_client_disconnect(cps_api_event_service_handle_t handle);

/**
 * Have the client register with the event service for one or more specific events
 * @param handle is the handle to the cps api events service
 * @param req the registration request
 * @return cps_api_ret_code_OK if success otherwise a failure
 */
cps_api_return_code_t cps_api_event_client_register(cps_api_event_service_handle_t *handle,
        cps_api_event_reg_t * req);

/**
 * Send an event to the event service for publishing using a previously registered handle
 * @param handle the handle to the hal event service
 * @param evt the event to send
 * @return standard return code
 */
cps_api_return_code_t cps_api_event_publish(cps_api_event_service_handle_t handle,
        cps_api_event_header_t *evt);

/**
 * @brief allocate a message with the specified space
 * @param space the maximum size of data that will supported by this event.
 *  This cause a buffer with the space + header size to be allocated and therefore
 *  the maximum buffer space in the message is "space" length.
 * @return NULL on error otherwise a cps_api_event_header_t is returned along with the space appended
 *   for the client's use.  The max_data_len field is set appropriately by this API and MUST
 *   not be touched
 */
cps_api_event_header_t * cps_api_event_allocate(unsigned int space);

/**
 * @brief Free the message that was allocated with the cps_api_event_allocate. This will be both
 * the event and the data.
 *
 * @param evt a pointer to the event header and corresponding data to be freed
 */
void cps_api_event_free(cps_api_event_header_t *evt);

/**
 * A debug API for converting the out the contents of a event to a string (header only)
 * @param evt to convert to string
 */
const char * cps_api_event_print(cps_api_event_header_t *evt, char *buff,size_t len);


#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
