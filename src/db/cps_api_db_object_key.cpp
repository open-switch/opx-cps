
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

#include "cps_api_db.h"


#include "cps_api_vector_utils.h"
#include "cps_api_object_key.h"


namespace {
bool db_key_copy_with_escape(std::vector<char> &lst,const void *data, size_t len) {
	size_t _offset = lst.size();
	size_t _space_wanted = len + _offset;

	try {
		lst.resize(_space_wanted);

		const char *_p = (const char *)data;
		for ( size_t ix = 0; ix < len ; ++ix ) {
			if (_p[ix]=='[' || _p[ix]==']' || _p[ix]=='?' || _p[ix]=='\\' || _p[ix]=='*') {
				++_space_wanted;
				lst.resize(_space_wanted);
				lst[_offset++] = '\\';
			}
			lst[_offset++] = _p[ix];
		}
	} catch (...) {
		return false;
	}
	return true;
}

}

bool cps_db::dbkey_from_class_key(std::vector<char> &lst,const cps_api_key_t *key) {
    size_t _len = cps_api_key_get_len_in_bytes((cps_api_key_t*)key);
	return db_key_copy_with_escape(lst,(const void *)cps_api_key_elem_start_const(key),_len);
}

bool cps_db::dbkey_from_instance_key(std::vector<char> &lst,cps_api_object_t obj, bool escape) {
    if (!dbkey_from_class_key(lst,cps_api_object_key(obj))) return false;

    cps_api_key_element_t *p = cps_api_key_elem_start(cps_api_object_key(obj));
    size_t len = cps_api_key_get_len(cps_api_object_key(obj));

    for ( size_t ix = 0; ix < len ; ++ix ) {
        cps_api_object_attr_t attr = cps_api_get_key_data(obj,p[ix]);
        if (attr==nullptr) continue;

        size_t _alen = cps_api_object_attr_len(attr);
        void * _adata = (void*) cps_api_object_attr_data_bin(attr);
        bool wildcard = _alen==1 && (*((char*)_adata)=='*');

        if (wildcard) { lst.push_back('*'); continue; }

        if (!escape && !cps_utils::cps_api_vector_util_append(lst,_adata,_alen)) return false;
        if (escape && !db_key_copy_with_escape(lst,_adata,_alen)) return false;
    }
    return true;
}




