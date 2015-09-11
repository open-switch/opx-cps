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

/**
 * The CPS allows a return code to be application defined for any given CPS requests like cps_api_get or cps_api_commit.
 *
 * The CPS itself also has its own return codes that can be reused by an application if it wishes.  Otherwise
 * the application can ensure that they return 0 for success otherwise an error will be assumed.
 *
 * When an application return a non-zero return code from a CPS callback, the CPS will expect that an error occurs
 * and will return that result to the client.  If the error occurs during a transaction, the CPS will begin requesting
 * the rolling back of the transaction.
 */
typedef int cps_api_return_code_t;

typedef enum {
    cps_api_ret_code_OK=0, //!< an OK return code
    cps_api_ret_code_ERR=1,//!< a generic error return code
} cps_api_return_code_enum_val_t;

/**
 * @}
 */

#endif /* CPS_API_ERRORS_H_ */
