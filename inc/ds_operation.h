/*
 * filename: db_operation.h
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_operation.h
 */

#ifndef DS_OPERATION_H_
#define DS_OPERATION_H_

#include "ds_object_category.h"
#include "ds_common_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * These are the standard DB return codes.  These are different then the standard return codes for a reason -
 * these APIs are isolated from the standard code base
 */
typedef enum {
    ds_ret_code_OK, //!< db_ret_code_OK
    ds_ret_code_ERR,//!< db_ret_code_ERR
} ds_return_code_t;

/**
 * These are the two database instances.
 */
typedef enum {
    ds_inst_TARGET=0, //!< db_inst_TARGET the config database
    ds_inst_OBSERVED=1,//!< db_inst_OBSERVED the status or observed database
    ds_inst_MAX
} ds_instance_t;

typedef enum {
    ds_oper_DELETE=1,
    ds_oper_CREATE=2,
    ds_oper_SET=3
}ds_operation_types_t;

/*
 * The structure for a get request
 */
typedef struct {
    ds_instance_t instance; //! instance to use (target or obs)
    ds_object_type_t type; //! type of object to query
    ds_common_list_t keys; //! list of keys to query (emptpy means all)
    ds_common_list_t list; //! list of discovered objects
}ds_get_params_t;

/**
 * The API for a set request
 */
typedef struct {
    ds_instance_t instance; //! db instance
    ds_common_list_t list; //! list of objects to modify
}ds_transaction_params_t;

/**
 * Initialize a get request
 * @param req request to initialize
 * @param db_type type of db (target/obs)
 * @param objtype type of object to query
 * @return db return code
 */
ds_return_code_t ds_get_request_init(ds_get_params_t *req, ds_instance_t db_type, ds_object_type_t objtype);
/**
 * Clean up used get request
 * @param req request to clean up
 * @return db return code
 */
ds_return_code_t ds_get_request_close(ds_get_params_t *req);

/**
 * Add a key to a get request
 * @param param the get request to update
 * @param type the type of db
 * @param data key data to add
 * @param len length of key data
 * @param deep_copy if the key is deep copied
 * @return db return code
 */
ds_return_code_t ds_get_add_key(ds_get_params_t * param,ds_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Get a database element or list of elements.  If the is a specific
 * element being queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried
 * @param param the structure containing the object request
 * @return db_return_code_t
 */
ds_return_code_t ds_get(ds_get_params_t * param);

/**
 * Initialize the db transaction for use with the set and commit API
 * The idea is that you init a db request, add objects with "set" then finally commit
 * @param req is the request to initialize
 * @param db_type type of db to update
 * @return db return code
 */
ds_return_code_t ds_transaction_init(ds_transaction_params_t *req, ds_instance_t db_type);
/**
 * Clean up after a transaction or close a pending transaction.  Before a commit is made
 * this will cancel a uncommitted transaction.
 * @param req is the transaction to cancel or clean up
 * @return db return code
 */
ds_return_code_t ds_transaction_close(ds_transaction_params_t *req);

/**
 * Return the db object type operation for a given object type.  This will be valid for components
 * implementing the write db API.  A write function call will be executed and the type field
 * will indicate if the write is due to a Create/Delete or Set
 * @param obj the object type to check.
 * @return
 */
ds_operation_types_t ds_object_type_operation(ds_object_type_t obj) ;

/**
 * Add a set to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param type the object type of the data to set
 * @param data the actual data to set
 * @param len the length of the data to set
 * @param deep_copy true if the memory will be allocated for the data and it will stay until you close the transaction
 * @return standard db return code
 */
ds_return_code_t ds_set(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Add a create to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param type the object type of the data to create
 * @param data the actual data to create
 * @param len the length of the data to create
 * @param deep_copy true if the memory will be allocated for the data and it will stay until you close the transaction
 * @return standard db return code
 */
ds_return_code_t ds_create(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Add a delete to the existing transaction.  Do not apply the data at this time.
 * @param trans the transaction id
 * @param type the object/key type of the data to delete
 * @param data the actual data containing a key
 * @param len the length of the data
 * @param deep_copy true if the memory will be allocated for the key and it will stay until you close the transaction
 * @return standard db return code
 */
ds_return_code_t ds_delete(ds_transaction_params_t * trans,ds_object_type_t type,
        void *data, size_t len, bool deep_copy);

/**
 * Set a database element or list of elements.  the list must
 * contain an array of objects.  Each object has it's own key.
 * The request is committed atomically... or rolled back atomically
 * @param param the structure containing the object request.
 * @return db_return_code_t
 */
ds_return_code_t ds_commit(ds_transaction_params_t * param);

/**
 * A registration function for the database
 * Implementers of the write API need to handle the db operation types and as part of the type field
 * in the db_list_entry structure.  This can be done by calling
 */
typedef struct {
    void * context; //! some application specific context to pass to the read/write function
    ds_instance_t instance_type;//! the database instance type (obs/target)
    ds_object_category_types_t object_category; //! the object category - see db_object_category.h
    ds_object_sub_type_t object_range_start; //! the start object range (unused for now)
    ds_object_sub_type_t object_range_end;//! the start object range (unused for now)
    ds_return_code_t (*db_read_function) (void * context, ds_get_params_t * param); //! the read db function
    ds_return_code_t (*db_write_function)(void * context, ds_list_entry_t * param); //! the set db function
}ds_registration_functions_t;

/**
 * API used to register a db handler
 * @param reg is the registration structure
 * @return standard db return code
 */
ds_return_code_t ds_register(ds_registration_functions_t * reg);

#ifdef __cplusplus
}
#endif

#endif /* DB_OPERATION_H_ */
