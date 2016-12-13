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

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_

#include "cps_api_operation.h"
#include "cps_api_errors.h"

cps_api_return_code_t cps_api_db_operation_get(cps_api_get_params_t * param, size_t ix);
cps_api_return_code_t cps_api_db_operation_commit(cps_api_transaction_params_t * param, size_t ix);
cps_api_return_code_t cps_api_db_operation_rollback(cps_api_transaction_params_t * param, size_t ix) ;

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_ */
