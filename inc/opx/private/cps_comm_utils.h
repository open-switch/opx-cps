/*
 * Copyright (c) 2018 Dell Inc.
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


#ifndef INC_CPS_COMM_UTILS_H_
#define INC_CPS_COMM_UTILS_H_

#include <string>
#include <unistd.h>

namespace cps_utils {

std::string cps_generate_unique_process_key();


template <typename _guard_element,_guard_element INVALID ,int (*close_func)(_guard_element param)>
class object_watch {
	_guard_element __elem;
public:
	object_watch(_guard_element &e) : __elem(e) {}
	void close() {
		if (__elem!=INVALID) close_func(__elem);
		__elem = INVALID;
	}
	void release() { __elem=INVALID; }
	~object_watch() { close(); }
};

using file_watch = object_watch<int,-1,close>;

}

#endif /* INC_CPS_COMM_UTILS_H_ */
