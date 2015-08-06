/** OPENSOURCELICENSE */
/*
 * cps_dictionary.h
 *
 *  Created on: Aug 5, 2015
 */

#ifndef CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_
#define CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_

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

#endif /* CPS_API_INC_PRIVATE_CPS_DICTIONARY_H_ */
