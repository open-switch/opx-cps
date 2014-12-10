/* OPENSOURCELICENSE */
/*
 * cps_api_errors.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#ifndef CPS_API_ERRORS_H_
#define CPS_API_ERRORS_H_

/**
 * These are the standard DB return codes.  These are different then the standard return codes for a reason -
 * these APIs are isolated from the standard code base
 */
typedef enum {
    ds_ret_code_OK, //!< db_ret_code_OK
    ds_ret_code_ERR,//!< db_ret_code_ERR
} ds_return_code_t;

#endif /* CPS_API_ERRORS_H_ */
