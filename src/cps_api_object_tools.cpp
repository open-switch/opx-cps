/** OPENSOURCELICENSE */
/*
 * cps_api_object_tools.cpp
 *
 *  Created on: Sep 2, 2015
 */
#include "cps_api_object_tools.h"

#include "cps_class_map.h"

cps_api_object_t cps_api_obj_tool_create(cps_api_qualifier_t qual,cps_api_attr_id_t id, bool add_defaults) {
	cps_api_object_guard og(cps_api_object_create());
	if (!og.valid()) return nullptr;

	cps_api_key_from_attr_with_qual(cps_api_object_key(og.get(),id,qual));

}

bool cps_api_obj_tool_matches_filter(cps_api_object_t filter, cps_api_object_t obj, bool require_all_attribs) {

}
