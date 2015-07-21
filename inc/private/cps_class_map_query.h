/** OPENSOURCELICENSE */
/*
 * cps_class_map_query.h
 *
 *  Created on: Apr 22, 2015
 */

#ifndef CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_
#define CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_

#include "cps_api_object.h"
#include "cps_api_object_attr.h"

#include <stdbool.h>
#include <string>
#include <vector>
#include <string>

struct cps_class_map_node_details_int_t {
    std::string name;
    std::string full_path;
    std::string desc;
    bool embedded=false;
    uint_t type=0;
    cps_api_attr_id_t id=0;
    std::vector<cps_api_attr_id_t> ids;
};

using cps_class_node_detail_list_t = std::vector<cps_class_map_node_details_int_t>;

void cps_class_map_level(const cps_api_attr_id_t *ids, size_t max_ids, cps_class_node_detail_list_t &details);

bool cps_class_map_detail(const cps_api_attr_id_t id, cps_class_map_node_details_int_t &details);


bool cps_class_map_query(const cps_api_attr_id_t *ids, size_t max_ids, const char * node,
        cps_class_map_node_details_int_t &details);


void cps_class_ids_from_string(std::vector<cps_api_attr_id_t> &v, const char * str);
std::string cps_class_ids_to_string(const std::vector<cps_api_attr_id_t> &v);

void cps_class_ids_from_key(std::vector<cps_api_attr_id_t> &v, cps_api_key_t *key);
std::string cps_key_to_string(const cps_api_key_t * key);



/**
 * Given a name give a attribute ID for the element.
 * @param name the name of the field
 * @param found will be set to true if the value is located
 * @return the attribute value or -1 if not found.
 */
cps_api_attr_id_t cps_name_to_attr(const char *name, bool &found);

#endif /* CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_ */
