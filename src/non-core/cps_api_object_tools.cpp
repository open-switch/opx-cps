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
 * cps_api_object_tools.cpp
 *
 *  Created on: Sep 2, 2015
 */
#include "cps_api_object_tools.h"
#include "cps_api_object.h"
#include "cps_class_map.h"

#include "cps_api_object_key.h"

#include <unordered_map>
#include <functional>
#include <vector>
#include <string.h>

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


bool cps_api_obj_tool_merge(cps_api_object_t main, cps_api_object_t overlay) {
	return cps_api_object_attr_merge(main,overlay,true);
}

bool cps_api_obj_tool_attr_matches(cps_api_object_t obj, cps_api_attr_id_t *ids, void ** values, size_t *len, size_t mx_) {
    for ( size_t ix = 0 ; ix < mx_ ; ++ix ) {
        cps_api_object_attr_t attr = cps_api_get_key_data(obj,ids[ix]);
        if (attr==nullptr) return false;
        size_t _len = cps_api_object_attr_len(attr);
        void * _data = cps_api_object_attr_data_bin(attr);
        if (len[ix]==_len) {
            if (memcmp(_data,values[ix],len[ix])==0) continue;
        }
        return false;
    }
    return true;
}

namespace {

void __cps_api_obj_tool_attr_callback(cps_api_object_t obj, cps_api_attr_id_t id,
		const std::function<void(void*,void *data[], size_t sizes[], size_t len, bool &cont)> &func,
		void *context) {

    cps_api_object_it_t it;
    if (!cps_api_object_it(obj,&id,1,&it)) {
        return ;
    }

    bool _cont = true;
    do {
        void *_data = cps_api_object_attr_data_bin(it.attr);
        size_t _size = cps_api_object_attr_len(it.attr);
        func(context,&_data,&_size,1,_cont);
        cps_api_object_it_next(&it);
        it.attr = std_tlv_find_next(it.attr,&it.len,id);
    } while (_cont && cps_api_object_it_valid(&it));
}


void __cps_api_bundle_attr_changes(cps_api_object_t obj, cps_api_attr_id_t id, cps_api_obj_tool_attr_callback_t cb,
		void *context) {

	struct  {
		std::vector<void*> _data;
		std::vector<size_t> _sizes;
	}_priv;

	auto fun = [&](void*,void *data[], size_t sizes[], size_t len, bool &cont) {
		for ( size_t ix = 0; ix < len ; ++ix ) {
			_priv._data.push_back(data[ix]);
			_priv._sizes.push_back(sizes[ix]);
		}
	};

	__cps_api_obj_tool_attr_callback(obj,id,fun,context);

	bool _cont = true;
	cb(context,&(_priv._data[0]),&_priv._sizes[0],_priv._data.size(),&_cont);
}

}

void cps_api_obj_tool_attr_callback(cps_api_object_t obj, cps_api_attr_id_t id, cps_api_obj_tool_attr_callback_t cb,
		void *context) {

	CPS_CLASS_ATTR_TYPES_t _type=CPS_CLASS_ATTR_T_LEAF;
	(void)cps_class_map_attr_class(id,&_type);

	if (_type==CPS_CLASS_ATTR_T_LEAF_LIST) {
		__cps_api_bundle_attr_changes(obj,id,cb,context);
	} else {
		auto fun = [&](void*,void *data[], size_t sizes[], size_t len, bool &cont) {
			cb(context,data,sizes,len,&cont);
		};
		__cps_api_obj_tool_attr_callback(obj,id,fun,context);
	}
}

void cps_api_obj_tool_attr_callback_list(cps_api_object_t obj, cps_api_obj_tool_attr_cb_list_t *lst, size_t len) {
	for (size_t ix = 0; ix < len ; ++ix ) {
		cps_api_obj_tool_attr_callback(obj,lst[ix].id,lst[ix].callback,lst[ix].context);
	}
}


