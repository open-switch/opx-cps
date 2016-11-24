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

#include "cps_api_vector_utils.h"

#include <stddef.h>
#include <string.h>

bool cps_utils::cps_api_vector_util_append(std::vector<char> &lst_,const void *data_, size_t len_) {
    size_t len = lst_.size();
    try {
        lst_.resize(len_ + len);
    } catch (...) {
        return false;
    }
    memcpy(&lst_[len],data_,len_);
    return true;
}

bool cps_utils::cps_api_range_split(size_t total ,ssize_t step,const cps_api_range_split_function &range) {
    for ( size_t ix = 0; ix < total ; ) {
        size_t now = ix;
        ix += step;
        if (ix>total) ix = total;
        if (range(now,ix)) {
            continue;
        }
        return false;
    }
    return true;
}
