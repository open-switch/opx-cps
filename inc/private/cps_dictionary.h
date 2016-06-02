/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * cps_dictionary.h
 *
 *  Created on: Aug 5, 2015
 */

#ifndef CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_
#define CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_

/** @addtogroup CPSAPI
 *  @{
 *
 *  @addtogroup Internal Internal Headers
 *  @warning this is an internal API.  Do not use directly
 *  @{
*/

#include "cps_api_object_attr.h"
#include "cps_class_map.h"

#include <stdbool.h>
#include <string>
#include <vector>


struct cps_class_map_node_details_int_t {
    std::string name;
    std::string full_path;
    std::string desc;
    bool embedded=false;
    CPS_CLASS_ATTR_TYPES_t attr_type;
    CPS_CLASS_DATA_TYPE_t data_type;
    cps_api_attr_id_t id=0;
    std::vector<cps_api_attr_id_t> ids;
};

const cps_class_map_node_details_int_t * cps_dict_find_by_name(const char * name);
const cps_class_map_node_details_int_t * cps_dict_find_by_id(cps_api_attr_id_t id);

typedef bool (*cps_dict_walk_fun)(void * context, const cps_class_map_node_details_int_t *ptr);

void cps_dict_walk(void * context, cps_dict_walk_fun fun);

/**
 * For now... always return cached state of object
 * @param obj the object to check
 * @return based on the object, return the type of storage
 */
CPS_API_OBJECT_STORAGE_TYPE_t cps_api_obj_get_storage_type(cps_api_object_t obj);
void cps_api_obj_set_storage_type(cps_api_object_t obj, CPS_API_OBJECT_STORAGE_TYPE_t type);

/**
 * @}
 * @}
 */

#endif /* CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_ */
