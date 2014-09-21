/*
 * filename: db_event_channel.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_event_channel.h
 *
 */

#ifndef DB_EVENT_CHANNEL_H_
#define DB_EVENT_CHANNEL_H_

#include "std_error_codes.h"
#include "std_event_service.h"

#include "ds_object_category.h"
#include "ds_interface.h"
#include "ds_route.h"
#include "ds_qos.h"

void db_event_print(std_event_msg_t *evt);

t_std_error db_event_channel_init(void);

#endif /* DB_EVENT_CHANNEL_H_ */
