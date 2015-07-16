/** OPENSOURCELICENSE */
/*
 * cps_class_map.h
 *
 *  Created on: Apr 19, 2015
 */

#ifndef BASE_MODEL_INC_CPS_CLASS_MAP_H_
#define BASE_MODEL_INC_CPS_CLASS_MAP_H_

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

/** @defgroup CPSAPI The CPS API
@{
*/

#define CPS_DEF_SEARCH_PATH "/opt/ngos/lib"        //the location of the generated class
#define CPS_DEF_CLASS_FILE_NAME "cpsclass"      //must match with the generated lib name

typedef enum  {
    CPS_CLASS_ATTR_T_LEAF=1<<4,
    CPS_CLASS_ATTR_T_LEAF_LIST=2<<4,
    CPS_CLASS_ATTR_T_CONTAINER=3<<4,
}CPS_CLASS_ATTR_TYPES_t;
/**
 * Structure representing a node in the CPS yang map.  This map can be used to get extended inforamtion
 * about a CPS object.
 */
typedef struct {
    const char *name;   //!name of the class element
    const char *desc;   //!Description of the element
    bool embedded;      //!true if the element is embedded
    uint_t type;        //!the type of the element
} cps_class_map_node_details;

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
cps_api_return_code_t cps_class_map_init(cps_api_attr_id_t id, const cps_api_attr_id_t *ids, size_t ids_len, cps_class_map_node_details *details);


/**
 * Convert the CPS API key to attribute IDs - the length of ids needs to be big enough
 * @param ids the pointer to the IDs to set
 * @param ids_len the length of the ids array
 * @param key the key to convert
 * @return true if successfully converted otherwise false
 */
bool cps_api_key_to_class_attr(cps_api_attr_id_t *ids, size_t ids_len, const cps_api_key_t * key);


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
 * From the attribute ID, return the full path of the element.
 *
 * @param id the ID to check
 * @return the full name or NULL
 */
const char * cps_attr_id_to_name(cps_api_attr_id_t id);

/**
 * Given a name give a attribute ID for the element.
 * @param name the name of the field
 * @param found will be set to true if the value is located
 * @return the attribute value or -1 if not found.
 */
cps_api_attr_id_t cps_name_to_attr(const char *name, bool &found);

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
 * /opt/ngos/lib folder if it exists
 */
void cps_api_class_map_init(void);

/**
@}
*/

#ifdef __cplusplus
}
#endif


#endif /* BASE_MODEL_INC_CPS_CLASS_MAP_H_ */
