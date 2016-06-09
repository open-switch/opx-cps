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


#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_RESPONSE_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_RESPONSE_H_

#include "cps_api_object.h"
#include "cps_api_key.h"

#include "cps_api_errors.h"

#include <vector>
#include <unordered_map>
#include <functional>

namespace cps_db {

class response {
	void * _reply;
public:
	response(void *p) { _reply = p ; }

	//simple string returns
	int get_str_len() ;
	const char *get_str() ;
	bool is_str();

	//error status check
	bool is_ok();

	//int gets
	bool is_int();
	int get_int();

	//for walking array responses
	bool has_elements();
	void *element_at(size_t ix);

	std::tuple<void *, size_t> get_element(size_t ix);

	size_t elements();

	void iterator(const std::function<void(size_t ix, int type, const void *data, size_t len)> &fun);

	bool is_status();



	using response_element_t = std::tuple<int, const void *, size_t>;
	size_t extract_elements(response_element_t *lst, size_t max);

	void dump();
};

class response_set {
	std::vector<void*> _data;
public:
	std::vector<void*>& get() { return _data; }
	size_t size() const { return _data.size(); }
	response get_response(size_t ix) { return response(_data[ix]); }
	~response_set();
};

}


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_RESPONSE_H_ */
