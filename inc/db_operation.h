/** OPENSOURCELICENSE */
/*
 * db_operation.h
 */

#ifndef DB_OPERATION_H_
#define DB_OPERATION_H_

#include "db_object_category.h"
#include "db_common_list.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    db_inst_TARGET=0, //!< db_inst_TARGET the config database
    db_inst_OBSERVED=1,//!< db_inst_OBSERVED the status or observed database
    db_inst_MAX
} db_instance_t;

typedef struct {
    db_instance_t instance;
    db_object_type_t type;
    db_common_list_t keys;
    db_common_list_t list;
}db_get_params_t;

typedef struct {
    db_instance_t instance;
    db_common_list_t list;
}db_set_params_t;


db_return_code_t db_get_request_init(db_get_params_t *req, db_instance_t db_type, db_object_type_t objtype);
db_return_code_t db_get_request_close(db_get_params_t *req);

db_return_code_t db_get_add_key(db_get_params_t * param,db_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Get a database element or list of elements.  If the is a specific
 * element being queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried
 * @param param the structure containing the object request
 * @return db_return_code_t
 */
db_return_code_t db_get(db_get_params_t * param);


db_return_code_t db_transaction_init(db_set_params_t *req);
db_return_code_t db_transaction_close(db_set_params_t *req);


/**
 * Add a set to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param type the object type of the data to set
 * @param data the actual data to set
 * @param len the length of the data to set
 * @param deep_copy true if the data will be contained by the transaction
 * @return standard db return code
 */
db_return_code_t db_set(db_set_params_t * trans,db_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Set a database element or list of elements.  the list must
 * contain an array of objects.  Each object has it's own key.
 * The request is committed atomically... or rolled back atomically
 * @param param the structure containing the object request.
 * @return db_return_code_t
 */
db_return_code_t db_commit(db_set_params_t * param);

typedef struct {
    void * context;
    db_instance_t instance_type;
    db_object_category_types_t object_category;
    db_object_sub_type_t object_range_start;
    db_object_sub_type_t object_range_end;
    db_return_code_t (*db_read_function) (void * context, db_get_params_t * param);
    db_return_code_t (*db_write_function)(void * context, db_list_entry_t * param);
}db_registration_functions_t;

db_return_code_t db_register(db_registration_functions_t * reg);

#ifdef __cplusplus
}
#endif

#endif /* DB_OPERATION_H_ */
