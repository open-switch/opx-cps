/* OPENSOURCELICENSE */
/*
 * cps_api_service.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_SERVICE_H_
#define CPS_API_SERVICE_H_

#include "cps_api_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CPS_API_EVENT_CHANNEL_NAME "/tmp/cps_event_service"
#define CPS_API_CHANNEL_NAME "/tmp/cps_service"
#define CPS_API_EVENT_THREADS (1)

/**
 * Initialize the CPS and start the cps services in the context of the calling process.  The CPS services like
 * event notifications and the transaction/get APIs are dependant on starting this process.
 *
 * @return cps_api_ret_code_OK on successful initialization otherwise will return one of the CPS return codes.
 */
cps_api_return_code_t cps_api_services_start(void);

#ifdef __cplusplus
}
#endif

#endif /* CPS_API_SERVICE_H_ */
