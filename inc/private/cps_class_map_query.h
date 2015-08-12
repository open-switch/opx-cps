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
#include "cps_class_map.h"


#include "private/cps_dictionary.h"

#include <stdbool.h>
#include <string>
#include <vector>
#include <string>

using cps_class_node_detail_list_t = std::vector<cps_class_map_node_details_int_t>;

void cps_class_map_level(const cps_api_attr_id_t *ids,
        size_t max_ids, cps_class_node_detail_list_t &details, bool children_only=false);

bool cps_class_map_detail(const cps_api_attr_id_t id, cps_class_map_node_details_int_t &details);


void cps_class_ids_from_key(std::vector<cps_api_attr_id_t> &v, cps_api_key_t *key);
void cps_class_ids_from_string(std::vector<cps_api_attr_id_t> &v, const char * str);

std::string cps_class_ids_to_string(const std::vector<cps_api_attr_id_t> &v);
std::string cps_key_to_string(const cps_api_key_t * key);

#endif /* CPS_API_INC_PRIVATE_CPS_CLASS_MAP_QUERY_H_ */
