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
    inline bool valid() { return _reply!=nullptr; }

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
    size_t elements();

    bool is_status();

    void dump();
};

class response_set {
    static constexpr size_t ARRAY_LEN=10;
    void *_data[ARRAY_LEN];
    size_t _used=0;
public:
    bool add(void *entry) {
        if (_used < ARRAY_LEN) {
            _data[_used++] = entry;
            return true;
        }
        return false;
    }
    void** get() { return _data; }
    size_t size() const { return _used; }
    response get_response(size_t ix) { return response(_data[ix]); }
    ~response_set();
};

}


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_RESPONSE_H_ */
