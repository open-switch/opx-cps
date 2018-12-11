/*
 * Copyright (c) 2018 Dell Inc.
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
 * cps_ns.h
 */

#ifndef CPS_NS_H_
#define CPS_NS_H_

/** @addtogroup CPSAPI
 *  @{
 */ /*
 *  @addtogroup Internal Internal Headers
 *  @warning this is an internal API.  Do not use directly
 *  @{
*/

#include "cps_api_errors.h"
#include "cps_api_operation.h"

#include "std_socket_tools.h"

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus

#include "std_envvar.h"
#include <string>


extern "C" {

static inline std::string cps_api_user_queue(const char *q) {
    std::string ns = q;
    const char *u = std_getenv("CPSNS");
    if (u!=NULL) {
        ns+=u;
    }
    return ns;
}

#endif


typedef struct cps_api_object_owner_reg_t {
    cps_api_key_t key;
    std_socket_address_t addr;
}cps_api_object_owner_reg_t;

void cps_api_create_process_address(std_socket_address_t *addr) ;
void cps_api_ns_get_address(std_socket_address_t *addr);

cps_api_return_code_t cps_api_ns_startup();


#ifdef __cplusplus
}
#endif

/**
 * @}
 * @}
 */

#endif /* CPS_NS_H_ */
