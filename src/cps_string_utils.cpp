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

#include "cps_string_utils.h"
#include "std_utils.h"
#include <cstdio>
#include <cstdarg>
#include <memory>

namespace cps_string{

std::string sprintf(const char *fmt, ...) {
    std::va_list args_len;
    va_start(args_len,fmt);
    std::va_list args_real;
    va_copy(args_real,args_len);

    size_t len = std::vsnprintf(nullptr,0,fmt,args_len)+1;
    va_end(args_len);

    std::unique_ptr<char[]> buf( new char[ len ] );
    std::vsnprintf(buf.get(),len,fmt,args_real);
    va_end(args_real);

    return std::string(buf.get());
}

std::vector<std::string> split(const std::string &str, const std::string &reg) {
    //@todo replace with regex once we switch to gcc 4.9
    std_parsed_string_t parsed=nullptr;
    std_parse_string(&parsed,str.c_str(),reg.c_str());
    size_t ix = 0;
    std::vector<std::string> _lst;
    const char * head = nullptr;
    while ((head=std_parse_string_next(parsed,&ix))!=nullptr) {
        _lst.push_back(head);
    }

    std_parse_string_free(parsed);
    return std::move(_lst);
}

std::string tostring(const void *data, size_t len) {
    std::string s;
    for (size_t ix =0; ix<len ; ++ix ) {
        if ((ix%16==0) && (ix!=0)) s+="\n";
        s+=cps_string::sprintf("%02x ",((unsigned char*)data)[ix]);
    }
    return s;
}

}
