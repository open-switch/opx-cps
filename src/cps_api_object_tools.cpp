/** OPENSOURCELICENSE */
/*
 * cps_api_object_tools.cpp
 *
 *  Created on: Sep 2, 2015
 */
#include "cps_api_object_tools.h"
#include "cps_api_object.h"
#include "cps_class_map.h"

extern "C" cps_api_object_t cps_api_obj_tool_create(cps_api_qualifier_t qual,cps_api_attr_id_t id, bool add_defaults) {
    cps_api_object_guard og(cps_api_object_create());
    if (!og.valid()) return nullptr;

    if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),id,qual)) {
        return nullptr;
    }

    return og.release();
}

static bool __cps_api_obj_tool_matches_filter(cps_api_object_it_t &fit, cps_api_object_it_t &oit, bool require_all_attribs) {

    for ( ; cps_api_object_it_valid(&fit); cps_api_object_it_next(&fit)) {
        cps_api_attr_id_t id = cps_api_object_attr_id(fit.attr);

        cps_api_object_it_t tgt = oit;
        tgt.attr = cps_api_object_it_find(&tgt,id);

        if (!cps_api_object_it_valid(&tgt)) {
            if (!require_all_attribs) continue;
            return false;
        }
        if (id == CPS_API_OBJ_KEY_ATTRS) {
            cps_api_object_it_t in_fit = fit;
            cps_api_object_it_inside(&in_fit);
            cps_api_object_it_inside(&tgt);
            if (!cps_api_object_it_valid(&in_fit)) {
                continue;
            }
            if (!cps_api_object_it_valid(&tgt)) {
                if (!require_all_attribs) continue;
                return false;
            }
            if (!__cps_api_obj_tool_matches_filter(in_fit,tgt,require_all_attribs)) {
                return false;
            }
            continue;
        }

        if (!cps_api_object_attrs_compare(fit.attr,tgt.attr)==0) {
            return false;
        }
    }

    return true;
}


extern "C" bool cps_api_obj_tool_matches_filter(cps_api_object_t filter, cps_api_object_t obj, bool require_all_attribs) {
    cps_api_object_it_t it;
    cps_api_object_it_begin(filter,&it);

    cps_api_object_it_t obj_it;
    cps_api_object_it_begin(obj,&obj_it);
    return __cps_api_obj_tool_matches_filter(it,obj_it,require_all_attribs);
}
