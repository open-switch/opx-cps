
#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_

#include "cps_api_operation.h"
#include "cps_api_errors.h"

cps_api_return_code_t cps_api_db_operation_get(cps_api_get_params_t * param, size_t ix);
cps_api_return_code_t cps_api_db_operation_commit(cps_api_transaction_params_t * param, size_t ix);
cps_api_return_code_t cps_api_db_operation_rollback(cps_api_transaction_params_t * param, size_t ix) ;

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_OPERATIONS_H_ */
