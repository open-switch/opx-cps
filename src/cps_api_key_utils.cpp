/*
 * cps_api_key_utils.cpp
 *
 *  Created on: Sep 1, 2016
 */

#include "cps_api_key_utils.h"
#include "cps_api_object_key.h"

void cps_api_object_iterate_key_attrs(cps_api_object_t obj, const std::function<void(cps_api_object_t,
								cps_api_attr_id_t,void *,size_t)> &iter) {
	cps_api_key_element_t *p = cps_api_key_elem_start(cps_api_object_key(obj));
    size_t len = cps_api_key_get_len(cps_api_object_key(obj));
    for ( size_t ix = 0; ix < len ; ++ix ) {
        cps_api_object_attr_t attr = cps_api_get_key_data(obj,p[ix]);
        if (attr==nullptr) continue;
        iter(obj,p[ix],cps_api_object_attr_data_bin(attr),cps_api_object_attr_len(attr));
    }
}

size_t cps_api_object_count_key_attrs(cps_api_object_t obj) {
	size_t count = 0;
	cps_api_object_iterate_key_attrs(obj,[&count](cps_api_object_t,
								cps_api_attr_id_t,void *,size_t)->void {
		++count;
	});
	return count;
}

