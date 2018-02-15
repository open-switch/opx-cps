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
 * cps_class_map_query.h
 *
 *  Created on: Apr 22, 2015
 */

#ifndef CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_
#define CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_

/** @addtogroup CPSAPI
 *  @{
 *
 *  @addtogroup Internal Internal Headers
 *  @warning this is an internal API.  Do not use directly
 *  @{
*/
#include "cps_api_object.h"
#include "cps_api_object_attr.h"
#include "cps_class_map.h"
#include "cps_api_node.h"

#include "private/cps_dictionary.h"

#include <stdbool.h>
#include <string>
#include <vector>
#include <string>
#include <functional>

/** @cond HIDDEN_SYMBOLS */
#define CPS_DEF_PRODUCT_LOC "/opt/dell/os10"
#define CPS_DEF_SEARCH_PATH CPS_DEF_PRODUCT_LOC "/lib"        //the location of the generated class
#define CPS_DEF_SEARCH_PATH_CFG "/etc" CPS_DEF_PRODUCT_LOC
#define CPS_DEF_META_SEARCH_PATHS  CPS_DEF_SEARCH_PATH ":/etc/sonic:" CPS_DEF_SEARCH_PATH_CFG

#define CPS_DEF_CLASS_FILE_NAME "cpsclass"      //must match with the generated lib name
#define CPS_DEF_CLASS_XML_SUFFIX "-cpsmetadata.xml"
/** @endcond */

using cps_class_node_detail_list_t = std::vector<cps_class_map_node_details_int_t>;

void cps_class_map_level(const cps_api_attr_id_t *ids,
        size_t max_ids, cps_class_node_detail_list_t &details, bool children_only=false);

bool cps_class_map_detail(const cps_api_attr_id_t id, cps_class_map_node_details_int_t &details);


void cps_class_ids_from_key(std::vector<cps_api_attr_id_t> &v, cps_api_key_t *key);
void cps_class_ids_from_string(std::vector<cps_api_attr_id_t> &v, const char * str);

std::string cps_class_ids_to_string(const cps_api_attr_id_t *ids, size_t len);
std::string cps_key_to_string(const cps_api_key_t * key);

const CPS_API_OBJECT_OWNER_TYPE_t *cps_class_owner_type_from_string(const char *str);
const CPS_CLASS_DATA_TYPE_t *cps_class_data_type_from_string(const char *str);
const CPS_CLASS_ATTR_TYPES_t *cps_class_attr_type_from_string(const char *str);
const cps_api_qualifier_t *cps_class_qual_from_string(const char *str);
const cps_api_operation_types_t* cps_operation_type_from_string(const char *str);
const cps_api_node_data_type_t* cps_node_type_from_string(const char *str);

const char * cps_class_owner_type_to_string(CPS_API_OBJECT_OWNER_TYPE_t data);
const char * cps_class_attr_type_to_string(CPS_CLASS_ATTR_TYPES_t data);
const char * cps_class_qual_to_string(cps_api_qualifier_t qual) ;
const char * cps_class_data_type_to_string(CPS_CLASS_DATA_TYPE_t data);
const char * cps_operation_type_to_string(cps_api_operation_types_t data);

/**
 * This API searches for the first matchign name.  If the at_end is true, then the API will ensure that the string
 * is the suffix for a successful match.
 * @param either the whole or part of the attribute name
 * @return a pointer to the attribute ID or null if it doesn't exist
 */
cps_api_attr_id_t *cps_api_attr_name_to_id(const char *name);

extern "C" {
//Load the CPS yang model definitions
t_std_error cps_api_yang_module_init(void);
}

/**
 * Find the key attribute list for an object.  Default key offset is 1 as we are skipping the qualifier (target/observed/etc..)
 */
const std::vector<cps_api_attr_id_t> & cps_api_key_attrs(const cps_api_key_t *key, size_t key_offset=1);


/**
 * All registration to updates of the CPS parameters.  internal systems can cache values
 * such as polling interval, etc..  Handlers should avoid calling any set flag APIs in
 * the context of the callback to avoid recursive behavior.
 * @param param the name of the parameter to watch
 * @param handler the function call when the parameter changes
 */
void cps_api_add_flag_set_handler(const char * param, const std::function<void(const char*)> *handler) ;

void cps_api_update_ssize_on_param_change(const char * param, ssize_t *value_to_set)  ;

/**
 * @}
 * @}
 */

#endif /* CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_ */
