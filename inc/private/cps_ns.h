/* OPENSOURCELICENSE */
/*
 * cps_ns.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
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
