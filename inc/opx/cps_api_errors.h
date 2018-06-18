/*
 * Copyright (c) 2016 Dell Inc.
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
 * cps_api_errors.h
 */

#ifndef CPS_API_ERRORS_H_
#define CPS_API_ERRORS_H_

/** @addtogroup CPSAPI
 * @{
 */

/** @addtogroup ReturnCodes Return Codes
 * These are the standard cps API return codes.  These are different then the standard return codes for a reason -
 * these APIs are isolated from the standard code base error codes

 * @{
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
 *
 * We have reserved the range of 0-20 for CPS specific errors.
 */
typedef int cps_api_return_code_t;

/**
 * The enum list of return codes supported by the CPS.
 */
typedef enum {
    cps_api_ret_code_OK=0, //!< an OK return code
    cps_api_ret_code_ERR=1,//!< a generic error return code
    cps_api_ret_code_NO_SERVICE=2,          //!< returned when there is no CPS service matching the object in the request
    cps_api_ret_code_SERVICE_CONNECT_FAIL=3,//!< returned when there is a service but it is not responding
    cps_api_ret_code_INTERNAL_FAILURE=4,    //!< returned if there is an error but it is memory allocation or other internal reasons
    cps_api_ret_code_TIMEOUT=5,             //!< returned when the client specified a timeout and the timeout is expired
    cps_api_ret_code_COMMUNICATION_ERROR=6,	//!< returned when there is a communication error
    cps_api_ret_code_NO_EXIST=7,		    //!< returned when the queried object doesn't exist (applies to db cached objects)
    cps_api_ret_code_CORRUPT=8,				//!< returned when the entry exists but is corrupt and may need to be cleaned up
    cps_api_ret_code_PARAM_INVALID=9,		//!< returned if parameters to the function are invalid

} cps_api_return_code_enum_val_t;

/**
 * @}
 * @}
 */

#endif /* CPS_API_ERRORS_H_ */
