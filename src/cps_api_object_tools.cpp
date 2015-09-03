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

    if (cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),id,qual)!=cps_api_ret_code_OK) {
        return nullptr;
    }

    return og.release();
}

bool cps_api_obj_tool_matches_filter(cps_api_object_t filter, cps_api_object_t obj, bool require_all_attribs) {
    cps_api_object_it_t it;
    cps_api_object_it_begin(filter,&it);

    cps_api_object_it_t obj_it;
    cps_api_object_it_begin(obj,&obj_it);

    for ( ; cps_api_object_it_valid(&it); cps_api_object_it_next(&it)) {
        cps_api_attr_id_t id = cps_api_object_attr_id(it.attr);
        cps_api_object_it_t tgt = obj_it;

        obj_it.attr = cps_api_object_it_find(&tgt,id);
        if (!cps_api_object_it_valid(&tgt)) {
            if (!require_all_attribs) continue;
            return false;
        }
        if (!cps_api_object_attrs_compare(it.attr,tgt.attr)) {
            return false;
        }
    }

    return true;
}
