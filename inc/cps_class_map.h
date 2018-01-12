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

/*
 */

#ifndef BASE_MODEL_INC_CPS_CLASS_MAP_H_
#define BASE_MODEL_INC_CPS_CLASS_MAP_H_

/** @addtogroup CPSAPI
 * @{
 * @addtogroup Dictionary Attribute and Object Dictionary Utilities
 *  <p>Applications need to add the following instruction:</p>

 @verbatim
 #include <cps_class_map.h>
 @endverbatim

 * @{
*/

#include "std_error_codes.h"
#include "cps_api_object_category.h"
#include "cps_api_key.h"
#include "cps_api_object_attr.h"
#include "cps_api_object.h"
#include "cps_api_errors.h"
#include "cps_api_operation.h"

#include <stdbool.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup typesandconstsDictionary Types and Constants
 * @{
 */
/** Class map version number */
#define CPS_CLASS_MAP_VER (2)

/** Class attribute types */
typedef enum  {
    CPS_CLASS_ATTR_T_LEAF=1,
    CPS_CLASS_ATTR_T_LEAF_LIST=2,
    CPS_CLASS_ATTR_T_CONTAINER=3,
    CPS_CLASS_ATTR_T_SUBSYSTEM=4,
    CPS_CLASS_ATTR_T_LIST=5,
}CPS_CLASS_ATTR_TYPES_t;

/** Class data types */
typedef enum {
    CPS_CLASS_DATA_TYPE_T_UINT8,
    CPS_CLASS_DATA_TYPE_T_UINT16,
    CPS_CLASS_DATA_TYPE_T_UINT32,
    CPS_CLASS_DATA_TYPE_T_UINT64,
    CPS_CLASS_DATA_TYPE_T_INT8,
    CPS_CLASS_DATA_TYPE_T_INT16,
    CPS_CLASS_DATA_TYPE_T_INT32,
    CPS_CLASS_DATA_TYPE_T_INT64,
    CPS_CLASS_DATA_TYPE_T_STRING,
    CPS_CLASS_DATA_TYPE_T_ENUM,
    CPS_CLASS_DATA_TYPE_T_BOOL,
    CPS_CLASS_DATA_TYPE_T_OBJ_ID,
    CPS_CLASS_DATA_TYPE_T_DATE,
    CPS_CLASS_DATA_TYPE_T_IPV4,
    CPS_CLASS_DATA_TYPE_T_IPV6,
    CPS_CLASS_DATA_TYPE_T_IP,
    CPS_CLASS_DATA_TYPE_T_BIN,
    CPS_CLASS_DATA_TYPE_T_DOUBLE,
    CPS_CLASS_DATA_TYPE_T_EMBEDDED,
    CPS_CLASS_DATA_TYPE_T_KEY,
}CPS_CLASS_DATA_TYPE_t;

typedef enum {
    CPS_API_OBJECT_SERVICE, /// the request for get and set will be forwarded to the service which owns the object or objects
    CPS_API_OBJECT_SERVICE_CACHE, ///the request for gets will be redirected to the database while the sets and updates will be
                                ///forwarded to the object after journaled into the database (to support cases where services restart)
    CPS_API_OBJECT_DB            ///this is an object that will be stored directly in the database with no validation
}CPS_API_OBJECT_OWNER_TYPE_t ;

/**
 * Structure representing a node in the CPS yang map.  This map can be used to get extended inforamtion
 * about a CPS object.
 */
typedef struct {
    const char *name;   //!< name of the class element
    const char *desc;   //!< Description of the element
    bool embedded;      //!< true if the element is embedded
    CPS_CLASS_ATTR_TYPES_t attr_type; //<! class attribute type
    CPS_CLASS_DATA_TYPE_t data_type;  //<! class data type
} cps_class_map_node_details;

/**@}*/

/**
 * This API creates a description for the specific model element.  The model element is defined as a
 * path from the root of the yang (and therefore CPS) tree.
 * eg.. category, subcat, ....
 *
 * @param id the actual attribute ID of this current string (must be unique)
 * @param ids the array of the ids including the category, subcategory, etc..
 * @param ids_len the length of the array
 * @param details the details
 * @return STD_ERR_OK if successful otherwise a return code indicating the error type
 */
cps_api_return_code_t cps_class_map_init(cps_api_attr_id_t id, const cps_api_attr_id_t *ids, size_t ids_len, const cps_class_map_node_details *details);

/**
 * Register a enum with the system - support look up/etc.  This will replace the previous mapping if one exists.
 * @param enum_name the enumeration name
 * @param field the string enum ID
 * @param value the enum value
 * @param descr a description for the enumeration
 * @return cps_api_ret_code_OK on success
 */
cps_api_return_code_t cps_class_map_enum_reg(const char *enum_name, const char *field, int value, const char * descr);

/**
 * Associated an enum with a an attribute ID.
 * @param id the attribute ID to associate with the value
 * @param name the name of the attribute to associate
 * @return cps_api_ret_code_OK on success
 */
cps_api_return_code_t cps_class_map_enum_associate(cps_api_attr_id_t id, const char *name);

/**
 * Translate an attribute and value to a name
 * @param id the attribute ID
 * @param val the interger value to translate
 * @return the string or NULL if not found
 */
const char *cps_class_enum_id(cps_api_attr_id_t id, int val);

/**
 * Translate the enum ID to the enum value given the attribute ID
 * @param id the attribute ID that owns the enum
 * @param tag the string enum ID
 * @return the integer value or -1 if not found
 */
int    cps_api_enum_value(cps_api_attr_id_t id, const char *tag);

/**
 * Determine if the node within a CPS object has embedded data
 *
 * @param ids the array of the ids of attribute in the object
 * @param ids_len the length of the array
 * @return true if it has an embedded attribute
 */
bool cps_class_attr_is_embedded(const cps_api_attr_id_t *ids, size_t ids_len);

/**
 * Check the element to see if it is valid.
 * @param ids the array of the ids of attribute in the object
 * @param ids_len the length of the array
 * @return true if the attribute is a known type
 */
bool cps_class_attr_is_valid(const cps_api_attr_id_t *ids, size_t ids_len);

/**
 *
 * @param ids the array of the ids of attribute in the object
 * @param ids_len the length of the array
 * @return the character pointer containing the elements name
 */
const char * cps_class_attr_name(const cps_api_attr_id_t *ids, size_t ids_len);

/**
 * Load all cps class descriptions in the directory specified.  Use the prefix to determine which
 * libraries actually have CPS related information
 * @param path is the path to the shared objects
 * @param prefix the filename prefix
 * @return true if successful otherwise class loading failed and you will get a log
 */
bool cps_class_objs_load(const char *path, const char * prefix);

/**
 * Given a string path, return the CPS object key for it
 * @param str the string to translate to a CPS object ID list
 * @param ids the destination holding the returned ids
 * @param max_ids the maximum number of returned ids
 * @return true if found otherwise false
 */
bool cps_class_string_to_key(const char *str, cps_api_attr_id_t *ids, size_t *max_ids);

/**
 * Take a CPS key, search the CPS meta data for a appropriate key path and return the class string if found
 *
 * @param key the key for which the string path is required
 * @param offset the offset from where the raw key starts
 * @return the pointer to the character string holding the full path translation of the key
 */
const char * cps_class_string_from_key(cps_api_key_t *key, size_t offset);

/**
 * A mechanism to take all existing formats of the key and translate them to a binary CPS key.
 * This includes both formats of x/x/x/x and 1.2.3.4
 *
 * @param string the key in one of the two supported formats mentioned above
 * @param key the destination key
 * @return true if successful otherwise false
 */
bool cps_class_key_from_string(const char *string, cps_api_key_t *key);

/**
 * Take a CPS key, search the CPS qualifier and return the qualifier as a string if found
 *
 * @param key the key for which the qualifier string is required
 * @return the pointer to the character string holding the qualifier
 */

const char * cps_class_qual_from_key(cps_api_key_t *key);


/**
 * From the attribute ID, return the full path of the element.
 *
 * @param id the ID to check
 * @return the full name or NULL
 */
const char * cps_attr_id_to_name(cps_api_attr_id_t id);

/**
 * Given a attribute ID, return the corresponding key
 * @param key a pointer to the destiation to hold the key
 * @param id the attribute ID for which to find the key
 * @param key_start_pos is the start position in the destination key where the found key is copied
 * @return true if possible otherwise false and key is not altered
 */
bool cps_api_key_from_attr(cps_api_key_t *key,cps_api_attr_id_t id,size_t key_start_pos);

/**
 * Create a key from an attribute ID with a qualifier as the first position.
 *
 * @param key the CPS key that will hold the finished key
 * @param id the attribute ID to find the key path for
 * @param cat the category (target, obsered, etc..)
 * @return true if the key can be generated
 */
bool cps_api_key_from_attr_with_qual(cps_api_key_t *key,cps_api_attr_id_t id,
        cps_api_qualifier_t cat);


/**
 * Search through all of the available class maps on the system and load them.
 * The class map will by default also search through all of the LD_LIBRARY_PATHS along with the
 * /opt/dell/os10/lib folder if it exists
 */
void cps_api_class_map_init(void);

/**
 * For a given attribute, get the data type
 *
 * @param id the attribute ID requested
 * @param[out] t  the location to put the type
 *
 * @return true if the attribute type is found otherwise false
 */
bool cps_class_map_attr_type(cps_api_attr_id_t id, CPS_CLASS_DATA_TYPE_t *t);


/**
 * This function will return the attribute's "class type" - eg leaf list, etc..
 * @param id the attribute ID
 * @param type return the type of the attribute if found
 * @return false if not found otherwise true
 */
bool cps_class_map_attr_class(cps_api_attr_id_t id, CPS_CLASS_ATTR_TYPES_t *type);

/****
 * The following API indicate an object's ownership (or also considered as storage location) of an object.
 *     This can be a:
 *         service object - the request for get and set will be forwarded to the service which owns the object or objects
 *         service cached object - the request for gets will be redirected to the database while the sets and updates will be
 *                                 forwarded to the object after journaled into the database (to support cases where services restart)
 *         database object - this is an object that will be stored directly in the database with no validation
 *
 * @param obj the object to check
 * @return based on the object, return the type of storage
 */
CPS_API_OBJECT_OWNER_TYPE_t cps_api_obj_get_ownership_type(cps_api_object_t obj);


/****
 * This API will update the object storage type.  For a longer description of the storage type take a look at the documentation for
 * cps_api_obj_get_storage_type.
 * @param key is the the class of the object
 * @param type is the object's ownership
 */
void cps_api_obj_set_ownership_type(cps_api_key_t *key, CPS_API_OBJECT_OWNER_TYPE_t type);


/**
 * Set or clear the automated event flag for a given object.
 * @param key the key of the object
 * @param automated_events true if automated events are required false if the owner of the object will generate their own
 * events.
 */
void cps_api_obj_set_auto_event(cps_api_key_t *key, bool automated_events);

/**
 * This will return true if the object in question will automatically generate events based on successful operations
 * for sets, delete, creates
 * @param obj the object to check
 * @return true if enabled false, if the events are handled by the applicaiton.
 */
bool cps_api_obj_has_auto_events(cps_api_object_t obj);


/**
 * Print the contents of the object to the standard output
 * @param obj the object to display
 */
void cps_api_object_print(cps_api_object_t obj);


/**
 * This API will return true if the attribute ID is a CPS reserved attribute ID otherwise false
 * @param id the attribute ID to check
 * @return true if this is a CPS infra attribute ID or false if it is a general purpose application attribute ID
 */
bool cps_api_attr_id_is_cps_reserved(cps_api_attr_id_t id);


/**
 * This function will indicate if the attribute ID specified is an attribute that is managed by CPS and
 * can be safely deleted or ignored (can be used during object compression)
 * @param id the attribute ID
 * @return true if the attribute who's ID is specified can be safely removed as it is a CPS internal attribute otherwise false
 */
bool cps_api_attr_id_is_temporary(cps_api_attr_id_t id);

/**
 * This will set internal CPS library and/or service parameters.
 * @param flag the CPS internal control value to setup
 * @param val the value of what the string/entry should be
 * @return either cps_api_return_code_OK or cps_api_return_code_ERROR depending on the
 *     success or failure condition
 */
cps_api_return_code_t cps_api_set_library_flags(const char * flag, const char *val);


/**
 * This API will ge the value for the specified cps flag.  Mostly used
 *     internally by the CPS though available for debug purposes.
 * @param flag the CPS flag to configure (timeouts, buffers, etc..)
 * @param val the place that will contain the output
 * @param val_size the size of the buffer
 * @return true if successful otherwise the flag doesn't exists
 */
bool cps_api_get_library_flags(const char * flag, char *val, size_t val_size) ;


#ifdef __cplusplus
}

#include <string>

/**
 * Convert the attribute ID and data to a printable string
 * @param id the attribute ID
 * @param data the data to display
 * @param len the length of the data
 * @return a string representation in the form of:
 *             attrid : "data"
 */
std::string cps_api_object_attr_as_string(cps_api_attr_id_t id, const void * data, size_t len);

/**
 * Take the object and create a string that has the name (+ key attributes) along with the object
 * @param obj the object to convert to a string
 * @return a string containing a ascii representation of the object
 */
std::string cps_api_object_to_c_string(cps_api_object_t obj);

/**
 * Get the value of a given CPS library/service configuration flag.  A C++ string based
 * implementation
 * @param flag the configuration flag to get the value of
 * @return the string value
 */
std::string cps_api_get_library_flag_value(const char * flag);


#endif

/**
 * @}
 * @}
 */

#endif /* BASE_MODEL_INC_CPS_CLASS_MAP_H_ */
