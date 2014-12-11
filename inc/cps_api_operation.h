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

#include "cps_api_object_category.h"
#include "cps_api_object.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cps_api_errors.h"

/** @defgroup CPSAPI The CPS API
 * This file handles all public CRUD operations of the CPS API.
@{
*/

/**
 * These are the two database instances.
 */
typedef enum {
    cps_api_inst_TARGET=0, //!<  cps_api_inst_TARGET the config database
    cps_api_inst_OBSERVED=1,//!< cps_api_inst_OBSERVED the status or observed database
    cps_api_inst_PROPOSED=2,
    cps_api_inst_MAX
} cps_api_instance_t;

/**
 * CPS API Key related indexes
 */
#define CPS_OBJ_KEY_INST_POS (0)
#define CPS_OBJ_KEY_CAT_POS (1)
#define CPS_OBJ_KEY_SUBCAT_POS (2)
#define CPS_OBJ_KEY_APP_INST_POS (3)

/**
 * Setup the key for use with the CPS API.  Initialize the length of the key
 *     to contain all of the default fields + the extra instance components that
 *     are specified by the user in the len_of_inst_comps
 *
 * @param key the key to initialize
 * @param len_of_inst_comps the extra fields that will be used as instances
 *     these should be placed at CPS_OBJ_KEY_APP_INST_POS and beyond.
 *     If any of the parameters for inst, cat or subcat are 0, then they will not
 *     be included in the length.
 *
 *     @verbatim
 *     cps_api_key_init(key, 0,cps_api_inst_TARGET,0,0) will initialize a key
 *         with size set to the target instance only
 *
 *     cps_api_key_init(key, 0,cps_api_inst_TARGET,XXXX,0) will initialize a key
 *         with size set to the target and XXX only
 *
 *     cps_api_key_init(key, 3,cps_api_inst_TARGET,XXX,YYY) will initialize a key
 *         with size set with the target, cat of XXX and sub of yyy with three positions
 *         for holding specific instances.  Then use
 *             cps_api_key_set(key,CPS_OBJ_KEY_APP_INST_POS,INST1) and
 *             cps_api_key_set(key,CPS_OBJ_KEY_APP_INST_POS+1,INST2) and
 *             cps_api_key_set(key,CPS_OBJ_KEY_APP_INST_POS+2,INST3) to set the instance key values
 *
 * @endverbatim
 * @param inst the CPS instance (prop, targ, obs)
 * @param cat the CPS instance category
 * @param subcat the CPS instance sub category
 */
void cps_api_key_init(cps_api_key_t * key, size_t len_of_inst_comps,
        cps_api_instance_t inst,
        cps_api_object_category_types_t cat,
        cps_api_object_subcategory_types_t subcat);

/**
 * Attributes of the key in the CPS that will indicate the specific operation
 */
typedef enum {
    cps_api_oper_DELETE=1,//!< delete operation
    cps_api_oper_CREATE=2,//!< create operation
    cps_api_oper_SET=3,    //!< set operation
    cps_api_oper_ACTION=4
}cps_api_operation_types_t;

/*
 * The structure for a get request
 */
typedef struct {
    cps_api_key_t      *keys;
    size_t           key_count;
    cps_api_object_list_t list;
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
cps_api_operation_types_t cps_api_object_type_operation(cps_api_key_t *key) ;

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
 * Add a action to the existing transaction.  Do not run the action at this time.
 * @param trans the transaction struct
 * @param object the object to delete - only the key required
 * @return standard db return code
 */
cps_api_return_code_t cps_api_action(cps_api_transaction_params_t * trans,
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
    cps_api_key_t key;
    cps_api_return_code_t (*_read_function) (void * context, cps_api_get_params_t * param, size_t key_ix); //! the read db function
    cps_api_return_code_t (*_write_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //! the set db function
    cps_api_return_code_t (*_rollback_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //! the set db function
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
/**
 * @}
 */
#endif /* DB_OPERATION_H_ */
