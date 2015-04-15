/* OPENSOURCELICENSE */
/*
 * cps_api_errors.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_ERRORS_H_
#define CPS_API_ERRORS_H_

/** @defgroup CPSAPI The CPS API
 * These are the standard cps API return codes.  These are different then the standard return codes for a reason -
 * these APIs are isolated from the standard code base error codes
@{
*/

typedef enum {
    cps_api_ret_code_OK=0, //!< an OK return code
    cps_api_ret_code_ERR=1,//!< a generic error return code
} cps_api_return_code_t;

/**
 * @}
 */

#endif /* CPS_API_ERRORS_H_ */
