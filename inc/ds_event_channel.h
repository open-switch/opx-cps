/*
 * filename: ds_event_channel.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_event_channel.h
 *
 */
#ifndef _DS_EVENT_CHANNEL_H_
#define _DS_EVENT_CHANNEL_H_

#include "std_error_codes.h"

/**
 * This API initializes the DB event sub system.  This must be done before anyone
 * tries to use the DS event service and must only be done by one process in AR.
 *
 * @return will return STD_ERR_OK on successful execution or a specific return code
 *        based on the failure.
 *
 */
t_std_error db_event_channel_init(void);

#endif /* DB_EVENT_CHANNEL_H_ */
