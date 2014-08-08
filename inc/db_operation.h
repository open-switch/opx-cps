/** OPENSOURCELICENSE */
/*
 * db_operation.h
 */

#ifndef DB_OPERATION_H_
#define DB_OPERATION_H_

#include "db_object_catagory.h"
#include "db_common_list.h"

/**
 * These are the standard DB return codes.  These are different then the standard return codes for a reason -
 * these APIs are isolated from the standard code base
 */
typedef enum {
    db_ret_code_OK, //!< db_ret_code_OK
    db_ret_code_ERR,//!< db_ret_code_ERR
} db_return_code_t;

/**
 * These are the two database instances.
 */
typedef enum {
    db_inst_TARGET, //!< db_inst_TARGET the config database
    db_inst_OBSERVED//!< db_inst_OBSERVED the status or observed database
} db_instance_t;

typedef struct {
    db_instance_t instance;
    db_object_type_t type;
    db_common_list_t *list;
}db_function_params_t;

/**
 * Get a database element or list of elements.  If the is a specific
 * element being queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried
 * @param param the structure containing the object request
 * @return db_return_code_t
 */
db_return_code_t db_get(db_function_params_t * param);

/**
 * Set a database element or list of elements.  the list must
 * contain an array of objects.  Each object has it's own key.
 * The request is committed atomically... or rolled back atomically
 * @param param the structure containing the object request.
 * @return db_return_code_t
 */
db_return_code_t db_commit(db_function_params_t * param);

#endif /* DB_OPERATION_H_ */
