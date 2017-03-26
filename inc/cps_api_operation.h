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



#ifndef CPS_API_OPERATION_H_
#define CPS_API_OPERATION_H_

/**
 * @addtogroup CPSAPI
 * @{
 * @addtogroup Operation Operation (Get/Set) and Operation Wrapper Functions
 * Create, delete, get and update operations provided by the CPS API.
 *     <p>In order to access the functionality provided by this module, applications need to add the following instruction:</p>

 @verbatim
 #include <cps_api_operation.h>
 #include <cps_api_operation_tools.h>
 @endverbatim

 * @{
 */

#include "cps_api_object_category.h"
#include "cps_api_object.h"
#include "dell-cps.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "cps_api_errors.h"

/**
 * @addtogroup typesandconstsOperation Types and Constants
 * @{
 */

/**
 * These are the qualifiers for the CPS objects.
 * <p>There are four qualifiers at this time:<p>
 *  - Proposed
 *  - Target
 *  - Observed
 *  - Realtime.
 *
 * <p>An application can 'propose' a change and therefore the qualifier should be set to <i>Proposed</i> when applications
 * attempt an operation on a object.</p>
 *
 * <p>If the owner of the object approves of the request, the request is converted to a <i>Target</i>
 * state.  It is expected that the owner of the object will send out a target change notification (event).</p>
 *
 * <p>The <i>Observed</i> state what is used at runtime.  For a situation where the object owner
 * has to perform some complicated operation on hardware/software before the target state
 * is reached, for a duration of time the target and observed may contain different values.</p>
 *
 * <p>The <i>Realtime</i> qualifier is a request to go directly to hardware for an instantaneous reading
 * This is only used for statistics at this time.</p>
 */
typedef enum {
    cps_api_qualifier_NULL=0,
    cps_api_qualifier_TARGET=1,
    cps_api_qualifier_OBSERVED=2,
    cps_api_qualifier_PROPOSED=3,
    cps_api_qualifier_REALTIME=4,
    cps_api_qualifier_REGISTRATION=5,
    cps_api_qualifier_RESERVED1=6,
    /* This range provide qualifiers only to be used with DB service back ends or through the CPS DB API. */
    cps_api_qualifier_DB_SERVICE_REALM_START=7,
    /* The Startup Config contains (meant to store) the application's configuration - used in the case of a
     * cold start (eg. no traffic to recover after the reboot). */
    cps_api_qualifier_RUNNING_CONFIG=7,
    /* Running Config that contains the data required to recover the state of a process/system after it restarts due to a application
     * restart or system restart system restart.  */
    cps_api_qualifier_STARTUP_CONFIG=8,
    cps_api_qualifier_DB_SERVICE_REALM_END=10,
    cps_api_qualifier_MAX,
} cps_api_qualifier_t;

/**
 * This enum range is designed to help with the configuration of objects in the system.  A application can set the following enums
 * on any objects that they perform a commit operation on (set/delete/create).
 *
 * Using this information (configuration type), an application can make changes to:
 * 1) Running configuration - used for warm restart or application/process restart
 * 2) Startup configuration - used in the case that the system goes through a full cold start process
 * 3) Running configuration only - used in the case that an application is dynamically changing the system based on observations or
 *         other actions that should not be stored for cold starts (eg.. iscsi auto provisioning, temporary IP address, etc..)
 */
typedef enum {
  CPS_CONFIG_TYPE_STARTUP_CONFIG = 1, /*This configuration should be placed into the startup config only and has no effect on the existing running config.*/
  CPS_CONFIG_TYPE_RUNNING_CONFIG = 2, /*This configuration should be placed into the running configuration but is a candidate in the future for copying to startup
based on user requests.*/
  CPS_CONFIG_TYPE_STARTUP_AND_RUNNING = 3, /*This configuration request should be placed in the running config and startup config both*/
  CPS_CONFIG_TYPE_RUNNING_CONFIG_ONLY = 4, /*This configuration request should never be copied into the startup configuration.  This is applicable for running
configuration only.*/
  CPS_CONFIG_TYPE_MIN=1,
  CPS_CONFIG_TYPE_MAX=4,
} CPS_CONFIG_TYPE_t;


/**@{*/
/**
 * CPS API Key related indexes.
 *
 * All objects have a key consisting of the following components:
 *  - Qualifier (described above)
 *  - Object category
 *  - Object sub-category
 *  - one or more uint32_t which contains instance IDs for the object
 */
/** Instance element position in key */
#define CPS_OBJ_KEY_INST_POS (0)
/** Category element position in key */
#define CPS_OBJ_KEY_CAT_POS (1)
/** Sub-category element position in key */
#define CPS_OBJ_KEY_SUBCAT_POS (2)
/** Application instance element position in key. */
#define CPS_OBJ_KEY_APP_INST_POS (3)
/**@}*/
/**@}*/

/**
 * Get the cps_api_qualifier_t for a key
 * @param key the key to query
 * @return the cps_api_qualifier_t
 */
static inline cps_api_qualifier_t cps_api_key_get_qual(cps_api_key_t *key) {
    return (cps_api_qualifier_t)cps_api_key_element_at(key,CPS_OBJ_KEY_INST_POS);
}

/**
 * Set the qualifier of the key to the provided value.
 * @param key the key to change the qualifier on
 * @param qual the qualifier to use
 */
void cps_api_key_set_qualifier(cps_api_key_t *key, cps_api_qualifier_t qual);


/**
 * Setup the key for use with the CPS API.  Initialize the length of the key
 *     to contain all of the default fields + the extra instance components that
 *     are specified by the user in the len_of_inst_comps.
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
 * Get the key's object category
 * @param key the key to query
 * @return the key's cps_api_object_category_types_t
 * @deprecated
 */
static inline cps_api_object_category_types_t cps_api_key_get_cat(cps_api_key_t *key) {
    return (cps_api_object_category_types_t)cps_api_key_element_at(key,CPS_OBJ_KEY_CAT_POS);
}

/**
 * Get the key's object sub category
 * @param key the key to query
 * @return the key's sub category
 * @deprecated
 */
static inline uint32_t cps_api_key_get_subcat(cps_api_key_t *key) {
    return cps_api_key_element_at(key,CPS_OBJ_KEY_SUBCAT_POS);
}

/**
 * Given a filter object (used in a get request) set a attribute used to specify the maximum
 * number of objects of that time returned at one time.
 *
 * @param obj the filter in question
 * @param obj_count the maximum count of elements
 * @return true if successful otherwise false
 */
bool cps_api_filter_set_count(cps_api_object_t obj, size_t obj_count);

/**
 * Given a filter object, determine if it has a max number of objects to be retrieved at any one time
 *
 * @param obj the filter in question
 * @param obj_count the count of objects
 * @return true if found the count otherwise there is no limit to the get request
 */
bool cps_api_filter_get_count(cps_api_object_t obj, size_t *obj_count);

/**
 * Set the a flag on the filter that tells the application to return the object located after this object (key order).
 * Note: This may be provided along with a filter count and therefore applications should be looking at both the filter count and the
 * filter get next settings to determine how to process requests.
 *
 * @param obj is the filter which to set the flag on
 * @return true if successful otherwise false (likely due to a memory issue)
 */
bool cps_api_filter_set_getnext(cps_api_object_t obj);

/**
 * This API returns true if the next object should be returned
 * @param obj the object filter
 * @return true if the user wants the next object otherwise false
 */
bool cps_api_filter_is_getnext(cps_api_object_t obj);

/**
 * This API indicates to the back end that the object filter has wild-cards as part of the attributes in the object.
 * Normally back end functionality can assume when a attribute is passed, the attributes being passed are specific values.
 * For instance, if the name of an interface has a "*" in the name, the behavior should be different then an application searching
 * for a list of interfaces eg. "eth*"
 *
 * @param obj the object filter
 * @param has_wildcard_attributes when true, this means that the attributes (one or more) in the filter contain wildcards and
 *     a more complicated search may be required.
 *
 * @return true if the attribute filter could be set or false if there was no space or out of memory
 */
bool cps_api_filter_wildcard_attrs(cps_api_object_t obj, bool has_wildcard_attributes);

/**
 * This API is used to insert escape characters for CPS attribute values.
 * For example if the name of an interface has a "*" in the name, this "*" should be escaped so that it wont be considered as a wildcard character.
 *
 * @param type Type of the CPS attribute to be escaped as defined by cps_api_object_ATTR_TYPE_t
 * @param buff Input CPS attribute value to be escaped
 * @param len Length of the input buffer. It needs to be appropriate as per its type.
 *            For example if the type of the attribute is cps_api_object_ATTR_T_U32 then the length is expected to be 4 bytes.
 *
 * @param attr Output buffer where the escaped CPS attribute value will be stored.
 * @param attr_len Maximum length of the attr buffer will be passed in to the API. This length should be twice as big as the length (len) of the input buff
 *          (Safe assumption when all of the characters in the input buff was to be escaped). The actual length of the attr buffer will be returned in attr_len
 *
 * @return true if the attribute value was escaped and stored successfully in attr buffer
 *         false if attr_len is not twice the input buffer length (len) or if the input length requirements are not met.
 *
 */
bool cps_api_attr_create_escaped(cps_api_object_ATTR_TYPE_t type, void *buff, size_t len, const void *attr, size_t *attr_len);


/**
 * This API will returns the value of the cps_api_filter_wildcard_attrs setting.  If the cps_api_filter_wildcard_attrs has not been
 * called, the default value will be false
 *
 * @param obj the object filter
 * @return the contents of the filter wildcard attributes flag
 */
bool cps_api_filter_has_wildcard_attrs(cps_api_object_t obj);


/**
 * @addtogroup typesandconstsOperation Types and Constants
 * @{
 */

/**
 * Data structure for 'get' requests.  Each get request can have one or more keys
 * and will receive a list of objects in response.
 *
 * <p>A request will be made for each object listed in filters followed by a query for each key
 * in the keys list.</p>
 *
 * <p>In the response "list", each object will contain its own key.</p>
 */
typedef struct {
    cps_api_key_t   *keys;                //!< A list of keys to be queried
    size_t           key_count;           //!< Number of keys
    cps_api_object_list_t list;
    cps_api_object_list_t filters;        //!< a list of objects to be queried.

    /* An optional timeout allowable on an operation - by default all operations have a
     * set this to 0 to ignore timeout*/
    size_t timeout;                          //!< timeout in ms - set this to 0 to ignore the timeout
}cps_api_get_params_t;


/**
 * Data structure for 'set' requests.
 * <p>The change_list is the list of objects that will be updated/changed.</p>
 *
 *<p> The prev is a list of objects that will be used internally by the API for the rollback functions
 * The prev will contain a list of previous values before the object has changed.  Not
 * guaranteed to be valid for callers of the cps_api_commit function.</p>
 *
 */
typedef struct {
    cps_api_object_list_t change_list; //! list of objects to modify
    cps_api_object_list_t prev; //! the previous state of the object modified

    /* An optional timeout allowable on an operation
     * set this to 0 to ignore timeout which is the default. */
    size_t timeout;                          //!<a timeout in ms - set this to 0 to ignore the timeout
}cps_api_transaction_params_t;

/**@}*/

/**
 * Initialize a get request.  Must call cps_api_get_request_close on any initialized
 * get request.
 *
 * @param req request to initialize
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 * @sa  cps_api_get_request_close
 */
cps_api_return_code_t cps_api_get_request_init(cps_api_get_params_t *req);

/**
 * Create an object that will be added to the get response and automatically add the object to the
 * req's list of returned objects.
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
 * Get a database element or list of elements.
 * <p>If a specific
 * element is queried, then the list must contain an array of keys
 * that are specific to the object queried otherwise all objects of the type will be queried.</p>
 *
 * @param param the structure containing the object request
 *
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 */
cps_api_return_code_t cps_api_get(cps_api_get_params_t * param);

/**
 * Initialize the transaction for use with the set and commit API.
 * <p>The sequence of operations is to initialize a transaction request, add objects with
 * "set/delete/create/action" then finally commit the transaction to perform the operation.</p>
 *
 * <p>Applications must call cps_api_transaction_close on any initialized transaction to cleanup.</p>
 *
 * @param req the request to initialize
 *
 * @return return code cps_api_ret_code_OK if successful otherwise an error
 * @sa cps_api_transaction_close
 */
cps_api_return_code_t cps_api_transaction_init(cps_api_transaction_params_t *req);

/**
 * Clean up after a transaction or close a pending transaction.  Before a commit is made
 * this will cancel a uncommitted transaction.  This will remove any objects in the req
 * including cleaning up.
 *
 * @param req the transaction to cancel or clean up
 *
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_transaction_close(cps_api_transaction_params_t *req);


/**
 * The following API will check the config type within the object and return the type to the caller.  If there is no config type
 * in the object, the API will default to runtime config.
 *
 * @param obj the object in question.
 * @return returns a valid configuration type
 */
CPS_CONFIG_TYPE_t cps_api_object_get_config_type(cps_api_object_t obj);

/**
 * Set the config type within the object.
 * @param obj the object in question
 * @param type tye config type to set
 * @return true if successful false on a memory allocation error
 */
bool cps_api_object_set_config_type(cps_api_object_t obj, CPS_CONFIG_TYPE_t type);

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
 * Add an action to the existing transaction.  Does not run the action at this time.
 * @param trans the transaction struct
 * @param object the object to delete - only the key required
 * @return cps_api_ret_code_OK if successful
 */
cps_api_return_code_t cps_api_action(cps_api_transaction_params_t * trans,
        cps_api_object_t object);

/**
 * Set a database element or list of elements.  <p>The list must
 * contain an array of objects.  Each object has its own key.
 * The request is committed atomically or rolled back atomically.</p>
 *
 * @param param the structure containing the object request.
 * @return cps_api_ret_code_OK if successful otherwise an error code. If the transaction fails
 *         the cps_api infrastructure will attempt to rollback an unsuccessful commit operation
 *         on the supported objects
 */
cps_api_return_code_t cps_api_commit(cps_api_transaction_params_t * param);

/**
 * @addtogroup typesandconstsOperation Types and Constants
 * @{
 */

/**
 * Attributes of the key in the CPS that will indicate the specific operation.
 * These attributes will be available to handlers that implement the read/write/rollback APIs
 * of the CPS
 */
typedef enum {
    cps_api_oper_NULL=0,//!< no specific operation
    cps_api_oper_DELETE=1,//!< delete operation
    cps_api_oper_CREATE=2,//!< create operation
    cps_api_oper_SET=3,    //!< set operation
    cps_api_oper_ACTION=4  //!< action operation
}cps_api_operation_types_t;
/**@}*/

/**
 * Return the db object type operation for a given object type.  This will be valid for components
 * implementing the write db API.  A write function call will be executed and the type field
 * will indicate if the write is due to Create/Delete or Set operations.
 * @param key the object type to check.
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
 * @warning Regular applications must not use this function.
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
 * to in further calls to register a CPS object.
 * @param handle handle to the CPS owner
 * @param number_of_threads number of threads to process
 * @return standard db return code
 */
cps_api_return_code_t cps_api_operation_subsystem_init(
        cps_api_operation_handle_t *handle, size_t number_of_threads) ;


/**
 * @addtogroup typesandconstsOperation Types and Constants
 * @{
 */
/**
 * A registration function for the database
 * Implementers of the write API need to handle the db operation types and as part of the type field
 * in the db_list_entry structure.
 */
typedef struct {
    cps_api_operation_handle_t handle; //!< handle obtained by calling cps_api_operation_subsystem_init
    void * context; //!< some application specific context to pass to the read/write function
    cps_api_key_t key; //!< key to register
    cps_api_return_code_t (*_read_function) (void * context, cps_api_get_params_t * param, size_t key_ix); //!< the read db function
    cps_api_return_code_t (*_write_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //!< the set db function
    cps_api_return_code_t (*_rollback_function)(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated); //!< the set db function
}cps_api_registration_functions_t;
/**@}*/

/**
 * API used to register a db handler
 * @param reg the registration structure
 * @return standard db return code
 */
cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg);

/**
 * An API that queries the object to get the latest stats based on the cps_api_operation_stats.h.
 * Applications that use this function must use the instruction:
 @verbatim
 #include <cps_api_operation_stats.h>
 @endverbatim

 * @param key  the object to query
 * @param stats_obj the object that will contain the stats
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
 * Cleanup a transaction automatically when you go out of scope with this transaction helper
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

    void release() { param=NULL; }
    void set(cps_api_get_params_t*p=NULL) { close(); param = p; }
    ~cps_api_get_request_guard() {
        close();
    }

    void close() { if (param!=NULL) cps_api_get_request_close(param); param = NULL; }
};

#endif
/**
 * @}
 * @}
 */
#endif /* DB_OPERATION_H_ */
