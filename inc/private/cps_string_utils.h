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

#ifndef INC_CPS_STRING_UTILS_H_
#define INC_CPS_STRING_UTILS_H_

#include <vector>
#include <string>

namespace cps_string{
	std::string sprintf(const char *fmt, ...);
	std::vector<std::string> split(const std::string &str, const std::string &reg);

	inline std::vector<std::string> split_ws(const std::string &str) {
		return split(str,"[^\\s]+");
	}

	std::string tostring(const void *data, size_t len);
};


#endif /* INC_CPS_STRING_UTILS_H_ */
