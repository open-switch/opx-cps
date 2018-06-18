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

#include "cps_api_db_response.h"

#include "cps_string_utils.h"

#include <iostream>
#include <hiredis/hiredis.h>

#include <tuple>

int cps_db::response::get_int() {
    return static_cast<redisReply*>(_reply)->integer;
}

bool cps_db::response::is_status() {
    return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_STATUS;
}

bool cps_db::response::has_elements() {
    return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_ARRAY;
}
void *cps_db::response::element_at(size_t ix) {
    return static_cast<redisReply*>(_reply)->element[ix];
}

bool cps_db::response::is_str() {
    //status and string both have string replies
    return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_STRING ||
            static_cast<redisReply*>(_reply)->type == REDIS_REPLY_STATUS ;
}
bool cps_db::response::is_nill() {
    return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_NIL;
}

size_t cps_db::response::elements()  {
    return static_cast<redisReply*>(_reply)->elements;
}

int cps_db::response::get_str_len()  {
    return static_cast<redisReply*>(_reply)->len;
}
const char *cps_db::response::get_str()  {
    return static_cast<redisReply*>(_reply)->str;
}

bool cps_db::response::is_int() {
    return static_cast<redisReply*>(_reply)->type == REDIS_REPLY_INTEGER;
}

bool cps_db::response::is_ok() {
    return static_cast<redisReply*>(_reply)->type != REDIS_REPLY_ERROR ;
}

void cps_db::response::dump() {
    std::cout << "Resp type is : " << static_cast<redisReply*>(_reply)->type << std::endl;

    if(is_int()) {
        std::cout << "Int : " << get_int() << std::endl;
    }
    if (is_ok()) std::cout << "OK Resp..." << std::endl;

    if (is_str()) {
        std::cout << get_str() << std::endl;
    }
    if (has_elements()) {
        for (size_t ix =0,mx=this->elements(); ix < mx ; ++ix ) {
            redisReply * data = (redisReply *)this->element_at(ix);
            std::cout << "IX:" << ix << " Type:" << data->type << " " << cps_string::tostring(data->str,data->len) << std::endl;
        }
    }
}
