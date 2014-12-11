/*
 * filename: cps_api_operation.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * cps_api_operation.h
 */

#ifndef CPS_API_OPERATION_H_
#define CPS_API_OPERATION_H_

#include "ds_object_category.h"
#include "cps_api_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cps_api_errors.h"

/**
 * These are the two database instances.
 */
typedef enum {
    ds_inst_TARGET=0, //!< db_inst_TARGET the config database
    ds_inst_OBSERVED=1,//!< db_inst_OBSERVED the status or observed database
    ds_inst_PROPOSED=2,
    ds_inst_MAX
} cps_api_instance_t;

typedef enum {
    ds_oper_DELETE=1,
    ds_oper_CREATE=2,
    ds_oper_SET=3
}cps_api_operation_types_t;

/*
 * The structure for a get request
 */
typedef struct {
    cps_obj_key_t *keys;
    size_t            key_count;
    cps_api_object_list_t object;
}cps_api_get_params_t;

/**
 * The API for a set request
 */
typedef struct {
    cps_api_object_list_t list; //! list of objects to modify
    cps_api_object_list_t prev; //! the previous state of the object modified
}cps_api_transaction_params_t;

/**
 * Initialize a get request
 * @param req request to initialize
 * @return db return code
 */
cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req);

/**
 * Clean up used get request
 * @param req request to clean up
 * @return db return code
 */
cps_api_return_code_t cps_api_get_request_close(cps_api_get_params_t *req);

/**
 * Get a database element or list of elements.  If the is a specific
 * element being queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried
 * @param param the structure containing the object request
 * @return db_return_code_t
 */
cps_api_return_code_t cps_api_get(cps_api_get_params_t * param);

/**
 * Initialize the db transaction for use with the set and commit API
 * The idea is that you init a db request, add objects with "set" then finally commit
 * @param req is the request to initialize
 * @param db_type type of db to update
 * @return db return code
 */
cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req, cps_api_instance_t db_type);
/**
 * Clean up after a transaction or close a pending transaction.  Before a commit is made
 * this will cancel a uncommitted transaction.
 * @param req is the transaction to cancel or clean up
 * @return db return code
 */
cps_api_return_code_t cps_api_transaction_close(cps_api_transaction_params_t *req);

/**
 * Return the db object type operation for a given object type.  This will be valid for components
 * implementing the write db API.  A write function call will be executed and the type field
 * will indicate if the write is due to a Create/Delete or Set
 * @param obj the object type to check.
 * @return
 */
cps_api_operation_types_t cps_api_object_type_operation(cps_obj_key_t *key) ;

/**
 * Add a set to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param object the object containing the data to set
 * @return standard db return code
 */
cps_api_return_code_t cps_api_set(cps_api_transaction_params_t * trans,
                                    cps_api_object_t object);

/**
 * Add a create to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param object the object containing the data to create
 * @return standard db return code
 */
cps_api_return_code_t cps_api_create(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Add a delete to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction struct
 * @param object the object to delete - only the key required
 * @return standard db return code
 */
cps_api_return_code_t cps_api_delete(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Set a database element or list of elements.  the list must
 * contain an array of objects.  Each object has it's own key.
 * The request is committed atomically... or rolled back atomically
 * @param param the structure containing the object request.
 * @return db_return_code_t
 */
cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param);

/**
 * A registration function for the database
 * Implementers of the write API need to handle the db operation types and as part of the type field
 * in the db_list_entry structure.  This can be done by calling
 */
typedef struct {
    void * context; //! some application specific context to pass to the read/write function
    cps_obj_key_t key;
    cps_api_return_code_t (*db_read_function) (void * context, cps_api_get_params_t * param, size_t key_ix); //! the read db function
    cps_api_return_code_t (*db_write_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //! the set db function
    cps_api_return_code_t (*db_rollback_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //! the set db function
}cps_api_registration_functions_t;

/**
 * API used to register a db handler
 * @param reg is the registration structure
 * @return standard db return code
 */
cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg);

#ifdef __cplusplus
}
#endif

#endif /* DB_OPERATION_H_ */
