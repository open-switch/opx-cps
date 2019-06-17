/*
 * Copyright (c) 2019 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 */

#ifndef CPS_API_SERVICE_H_
#define CPS_API_SERVICE_H_

/** @defgroup CPSAPI CPS API - C/C++ Language Functions
 */

/** @addtogroup CPSAPI
 * @{
 * @addtogroup Initialization Initialization of CPS Service
 *  <p>Applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_service.h>
 @endverbatim

 * @{
*/

#include "cps_api_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @cond HIDDEN_SYMBOLS */
/**
 * @addtogroup typesandconstsInitialization Types and Constants
 * @warning the following attributes are all internal and should not be used by applications
 * @deprecated
 */
#define CPS_API_EVENT_CHANNEL_NAME "/tmp/cps_event_service"
#define CPS_API_CHANNEL_NAME "/tmp/cps_service"
#define CPS_API_EVENT_THREADS (1)
/** @endcond */


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

/**
 * @}
 * @}
 */
#endif /* CPS_API_SERVICE_H_ */
