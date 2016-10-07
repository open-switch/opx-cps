
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
#include "cps_class_map_query.h"

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


static size_t _get_instance_key_attrs(cps_api_object_t obj, void *lst[],size_t lst_len[],size_t len, bool &wildcard, bool &contains_all) {
	//size of key is fixed to less then some const max - safe for use in stack vector
	auto &key_ids = cps_api_key_attrs(cps_api_object_key(obj));

	wildcard = false;
	contains_all = false;

	size_t _lst_used = 0;

	for ( auto id : key_ids ) {
        cps_api_object_attr_t attr = cps_api_get_key_data(obj,id);
        if (attr==nullptr) continue;

        lst_len[_lst_used] = cps_api_object_attr_len(attr);
        lst[_lst_used] = (void*) cps_api_object_attr_data_bin(attr);

        wildcard |= (lst_len[_lst_used]==1 && (*((char*)lst[_lst_used])=='*'));

        ++_lst_used;
	}
	contains_all = (_lst_used==key_ids.size());

	return _lst_used;
}

bool cps_db::dbkey_instance_or_wildcard(std::vector<char> &lst,cps_api_object_t obj, bool &wildcard) {
	cps_api_key_t *key = cps_api_object_key(obj);
	if (!dbkey_from_class_key(lst,key)) return false;

	size_t _key_len = cps_api_key_get_len(key);

	void *_lst[_key_len];
	size_t _lst_len[_key_len];
	bool contains_all =false;

	size_t _lst_used = _get_instance_key_attrs(obj,_lst,_lst_len,_key_len,wildcard,contains_all);
	wildcard = wildcard || !contains_all;

	if (_lst_used==0) return true;
	for (size_t ix = 0; ix < _lst_used ; ++ix ) {
        if (!wildcard && !cps_utils::cps_api_vector_util_append(lst,_lst[ix],_lst_len[ix])) return false;
        if (wildcard && !db_key_copy_with_escape(lst,_lst[ix],_lst_len[ix])) return false;
	}
    return true;
}

bool cps_db::dbkey_from_instance_key(std::vector<char> &lst,cps_api_object_t obj, bool escape) {
	cps_api_key_t *key = cps_api_object_key(obj);
	if (!dbkey_from_class_key(lst,key)) return false;

	size_t _key_len = cps_api_key_get_len(key);

	void *_lst[_key_len];
	size_t _lst_len[_key_len];
	bool contains_all =false;
	bool wildcard = false;

	size_t _lst_used = _get_instance_key_attrs(obj,_lst,_lst_len,_key_len,wildcard, contains_all);

    for ( size_t ix = 0; ix < _lst_used ; ++ix ) {
        bool wildcard = _lst_len[ix]==1 && (*((char*)_lst[ix])=='*');
        if (wildcard) { lst.push_back('*'); continue; }

        if (!escape && !cps_utils::cps_api_vector_util_append(lst,_lst[ix],_lst_len[ix])) return false;
        if (escape && !db_key_copy_with_escape(lst,_lst[ix],_lst_len[ix])) return false;
    }
    return true;
}




