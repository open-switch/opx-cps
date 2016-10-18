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

#ifndef CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_
#define CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <functional>

namespace cps_utils {

bool cps_api_vector_util_append(std::vector<char> &lst,const void *data, size_t len);

inline bool cps_api_vector_util_set(std::vector<char> &lst,const void *data, size_t len) {
    lst.clear();
    return cps_api_vector_util_append(lst,data,len);
}

using cps_api_range_split_function = std::function<bool(size_t ix, size_t mx)>;

bool cps_api_range_split(size_t total ,ssize_t step,const cps_api_range_split_function &range);

template <typename T>
struct vector_hash {
    std::size_t operator() (const std::vector<T> &c) const {
        int64_t rc = 0;
        for ( auto it : c ) {
            int64_t _cur = std::hash<T>()(it);
            rc =  rc ^ (_cur << 3);
        }
        return size_t(rc);
    }
};

}



#endif /* CPS_API_INC_PRIVATE_CPS_API_VECTOR_UTILS_H_ */
