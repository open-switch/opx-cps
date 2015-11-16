/** OPENSOURCELICENSE */
/*
 * (c) Copyright 2015 Dell Inc. All Rights Reserved.
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
 * These are the qualifiers for the CPS objects.
 * There are four qualifiers at this time.  Proposed, Target, Observed and Realtime.
 *
 * An application will propose a change and therefore the qualifier will be proposed when they
 * attempt an operation on a object.
 *
 * If the owner of the object approves of the request, the request is converted to a target
 * state.  It is expected that the owner of the object will send out a target change notification
 *
 * The observed state what is being used at runtime.  For a situation where the object owner
 * has to perform some complicated operation on hardware/software before the target state
 * is reached, there may be a time where the target and observed contain different objects.
 *
 * The realtime qualifier is a request to go directly to hardware for an instantaneous reading
 * This is only used for statistics at this time.
 */
typedef enum {
    cps_api_qualifier_NULL=0,
    cps_api_qualifier_TARGET=1,
    cps_api_qualifier_OBSERVED=2,
    cps_api_qualifier_PROPOSED=3,
    cps_api_qualifier_REALTIME=4,
    cps_api_qualifier_REGISTRATION=5,
    cps_api_qualifier_MAX,
} cps_api_qualifier_t;

/**
 * CPS API Key related indexes.
 *
 * All objects have a key consisting of the following pieces.
 *  - Qualifier (described above)
 *  - Object category
 *  - Object sub-category
 *  - one or more uint32_t which contains instance IDs for the object
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
 * @param qual the CPS qualifier (prop, targ, obs)
 * @param cat the CPS object category
 * @param subcat the CPS object sub category
 * @param number_of_inst is the number of following instance IDs
 *     @verbatim
 *     Example...
 *     cps_api_key_init(key, cps_api_qual_TARGET,ZZZ,YYY,0) will initialize a key
 *         with size set to the target instance only
 *
 *     cps_api_key_init(key, cps_api_qual_TARGET,ZZZ,YYY,2, acl_table_id, acl_entry_id)
 *
 *             will initialize a key in category ZZZ for object type YYY with
 *             two instance IDs a acl_table_id and acl_entry_id
 *
 * @endverbatim
 */
void cps_api_key_init(cps_api_key_t * key,
        cps_api_qualifier_t qual,
        cps_api_object_category_types_t cat,
        cps_api_object_subcategory_types_t subcat,
        size_t number_of_inst, ...);

/**
 * Get the cps_api_qualifier_t for a key
 * @param key the key to query
 * @return the cps_api_qualifier_t
 */
static inline cps_api_qualifier_t cps_api_key_get_qual(cps_api_key_t *key) {
    return (cps_api_qualifier_t)cps_api_key_element_at(key,CPS_OBJ_KEY_INST_POS);
}

/**
 * Get the key's object category
 * @param key the key to query
 * @return the key's cps_api_object_category_types_t
 */
static inline cps_api_object_category_types_t cps_api_key_get_cat(cps_api_key_t *key) {
    return (cps_api_object_category_types_t)cps_api_key_element_at(key,CPS_OBJ_KEY_CAT_POS);
}

/**
 * Get the key's object sub category
 * @param key the key to query
 * @return the key's sub category
 */
static inline uint32_t cps_api_key_get_subcat(cps_api_key_t *key) {
    return cps_api_key_element_at(key,CPS_OBJ_KEY_SUBCAT_POS);
}

/**
 * Given a filter object (used in a get request) set a count attribute that can be used to specify the maximum
 * number of objects of that time returned at one time.
 *
 * @param obj the filter in question
 * @param obj_count the maximum count of elements
 * @return true if successful otherwise false
 */
bool cps_api_filter_set_count(cps_api_object_t obj, size_t obj_count);

/**
 * Given a filter object, determine if it has a count (max number of objects to be retrieved at any one time)
 *
 * @param obj the filter in question
 * @param obj_count the count of objects
 * @return true if found the count otherwise there is no limit to the get request
 */
bool cps_api_filter_get_count(cps_api_object_t obj, size_t *obj_count);

/*
 * The structure for a get request.  Each get request can have one or more keys
 * and will receive a list of objects in response.
 *
 * A request will be made for each object listed in filters followed by a query for each key
 * in the keys list.
 *
 * In the response "list", each object will contain its own key.
 */
typedef struct {
    cps_api_key_t   *keys;                //!< A list of keys to be queried
    size_t           key_count;
    cps_api_object_list_t list;
    cps_api_object_list_t filters;        //!< a list of objects to be queried.

    /* An optional timeout allowable on an operation - by default all operations have a
     * set this to 0 to ignore timeout*/
    size_t timeout;                          //!< timeout in ms - set this to 0 to ignore the timeout
}cps_api_get_params_t;


/**
 * The API for a set request.
 * The change_list is the list of objects that will be updated/changed
 *
 * The prev is a list of objects that will be used internally by the API for the rollback functions
 * The prev will contain a list of previous values before the object has changed.  Not
 * guaranteed to be valid for callers of the cps_api_commit function.
 *
 */
typedef struct {
    cps_api_object_list_t change_list; //! list of objects to modify
    cps_api_object_list_t prev; //! the previous state of the object modified

    /* An optional timeout allowable on an operation
     * set this to 0 to ignore timeout which is the default. */
    size_t timeout;                          //!<a timeout in ms - set this to 0 to ignore the timeout
}cps_api_transaction_params_t;


/**
 * Initialize a get request.  Must call cps_api_get_request_close on any initialized
 * get request.
 *
 * @param req request to initialize
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 */
cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req);

/**
 * Create an object that will be added to the get response and automatically add the object to the
 * req's list of returned objects
 * @param req the get request to add the object
 * @return the cps_api_object_t created and added to the get request or NULL if the request fails
 */
static inline cps_api_object_t cps_api_get_result_create(cps_api_get_params_t *req ) {
    return cps_api_object_list_create_obj_and_append(req->list);
}

/**
 * Clean up used get request including removing any objects added to the req
 * object list.
 *
 * @param req request to clean up
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 */
cps_api_return_code_t cps_api_get_request_close(cps_api_get_params_t *req);

/**
 * Get a database element or list of elements.  If the is a specific
 * element being queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried
 *
 * @param param the structure containing the object request
 *
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 */
cps_api_return_code_t cps_api_get(cps_api_get_params_t * param);

/**
 * Initialize the transaction for use with the set and commit API
 * The idea is that you init a transaction request, add objects with
 * "set/delete/create/action" then finally commit the transaction to perform the operation
 *
 * You must call cps_api_transaction_close on any initialized transaction to cleanup
 *
 * @param req is the request to initialize
 *
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 */
cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req);

/**
 * Clean up after a transaction or close a pending transaction.  Before a commit is made
 * this will cancel a uncommitted transaction.  This will remove any objects in the req
 * including cleaning up.
 *
 * @param req is the transaction to cancel or clean up
 *
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_transaction_close(cps_api_transaction_params_t *req);

/**
 * Add a set to the existing transaction.  Do not apply the data at this time.
 *
 * @param trans the transaction id
 * @param object the object containing the data to set
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_set(cps_api_transaction_params_t * trans,
                                    cps_api_object_t object);

/**
 * Add a create to the existing transaction.  Does not apply the data at this time.
 * @param trans the transaction id
 * @param object the object containing the data to create
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_create(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Add a delete to the existing transaction.  Does not apply the data at this time.
 * @param trans the transaction struct
 * @param object the object to delete - only the key required
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_delete(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Add a action to the existing transaction.  Does not run the action at this time.
 * @param trans the transaction struct
 * @param object the object to delete - only the key required
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_action(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Set a database element or list of elements.  the list must
 * contain an array of objects.  Each object has it's own key.
 * The request is committed atomically... or rolled back atomically
 *
 * @param param the structure containing the object request.
 * @return cps_api_ret_code_OK if successful otherwise.. if the transaction fails
 *         the cps_api infrastructure will attempt to rollback an unsuccessful commit operation
 *         on the supported objects
 */
cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param);

/**
 * Attributes of the key in the CPS that will indicate the specific operation.
 * These attributes will be available to handers that implement the read/write/rollback APIs
 * of the CPS
 */
typedef enum {
    cps_api_oper_DELETE=1,//!< delete operation
    cps_api_oper_CREATE=2,//!< create operation
    cps_api_oper_SET=3,    //!< set operation
    cps_api_oper_ACTION=4
}cps_api_operation_types_t;

/**
 * Return the db object type operation for a given object type.  This will be valid for components
 * implementing the write db API.  A write function call will be executed and the type field
 * will indicate if the write is due to a Create/Delete or Set
 * @param obj the object type to check.
 * @return cps_api_ret_code_OK if successful
 */
cps_api_operation_types_t cps_api_object_type_operation(cps_api_key_t *key) ;


/**
 * Set the operation type of the provided key to the operation type specified.
 *
 * @param key the key to update with the operation type
 * @param op  the operation that will be set on the key
 */
void cps_api_object_set_type_operation(cps_api_key_t *key,cps_api_operation_types_t op) ;

/**
 * The following function is provided to make unit testing with the CPS library easier.
 * This function will initialize the event service and also provide a way to support
 * CPS API operations in a unit testing environment.
 *
 * @return true if unit testing environment is ready for users.
 */
bool cps_api_unittest_init(void);

/**
 * The handle to the CPS API operation subsystem. Required for use with the object
 * registration below.
 */
typedef void * cps_api_operation_handle_t;


/**
 * Initialize the operation subsystem and get back a handle.  The handle then can be used
 * to in futher calls to register a CPS object.
 * @param handle handle to the CPS owner
 * @param number_of_threads number of threads to process
 * @return standard db return code
 */
cps_api_return_code_t cps_api_operation_subsystem_init(
        cps_api_operation_handle_t *handle, size_t number_of_threads) ;


/**
 * A registration function for the database
 * Implementers of the write API need to handle the db operation types and as part of the type field
 * in the db_list_entry structure.  This can be done by calling
 */
typedef struct {
    cps_api_operation_handle_t handle;
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

/**
 * An API that queries the object to get the latest stats based on the cps_api_operation_stats.h
 * @param key is the object to query
 * @param stats_obj is the object that will contain the stats
 * @return cps_api_ret_code_OK on success otherwise a failure return code
 */
cps_api_return_code_t cps_api_object_stats(cps_api_key_t *key, cps_api_object_t stats_obj);


/**
 * Determine if a an application has registered to receive requests for the given key.
 *
 * @param key the actual key that is being checked to see if there is an owner
 * @param rc in case of false responses, this return code may be able to provide additional details
 *
 * @return true if there is a application registered or false if there is no registration found for that key
 */
bool cps_api_is_registered(cps_api_key_t *key, cps_api_return_code_t *rc);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * Cleanup a transaction automatically when you go out of scope with this transaciton helper
 */
class cps_api_transaction_guard {
    cps_api_transaction_params_t *param;
public:
    cps_api_transaction_guard(cps_api_transaction_params_t*p) {
        param = p;
    }
    ~cps_api_transaction_guard() {
        cps_api_transaction_close(param);
    }
};

/**
 * Cleanup a a get request automatically when you go out of scope with this get
 * request helper
 */
class cps_api_get_request_guard {
    cps_api_get_params_t *param;
public:
    cps_api_get_request_guard(cps_api_get_params_t*p) {
        param = p;
    }
    ~cps_api_get_request_guard() {
        cps_api_get_request_close(param);
    }
};

#endif
/**
 * @}
 */
#endif /* DB_OPERATION_H_ */
