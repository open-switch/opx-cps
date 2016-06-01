

#include "cps_api_db.h"


#include "cps_api_vector_utils.h"
#include "cps_api_object_key.h"

bool cps_db::dbkey_from_class_key(std::vector<char> &lst,const cps_api_key_t *key) {
	const cps_api_key_element_t *p = cps_api_key_elem_start_const(key);
	size_t len = cps_api_key_get_len_in_bytes((cps_api_key_t*)key);

	return cps_utils::cps_api_vector_util_append(lst,p,len);
}

bool cps_db::dbkey_from_instance_key(std::vector<char> &lst,cps_api_object_t obj) {
	if (!dbkey_from_class_key(lst,cps_api_object_key(obj))) return false;

	cps_api_key_element_t *p = cps_api_key_elem_start(cps_api_object_key(obj));
	size_t len = cps_api_key_get_len(cps_api_object_key(obj));

	for ( size_t ix = 0; ix < len ; ++ix ) {
		cps_api_object_attr_t attr = cps_api_get_key_data(obj,p[ix]);
		if (attr==nullptr) continue;
		if (!cps_utils::cps_api_vector_util_append(lst,cps_api_object_attr_data_bin(attr),cps_api_object_attr_len(attr))) return false;
	}
	return true;
}




