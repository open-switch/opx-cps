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

/*
 * cps_api_object_attr.cpp
 *
 *  Created on: Jun 4, 2015
 */


#include "cps_api_object_attr.h"
#include "cps_string_utils.h"

#include "std_assert.h"

#include <stdio.h>
#include <algorithm>
#include <string.h>

extern "C" {

uint_t cps_api_object_attr_data_uint(cps_api_object_attr_t attr) {
    size_t len = cps_api_object_attr_len(attr);
    if(len==sizeof(uint8_t)) return *(uint8_t*)cps_api_object_attr_data_bin(attr);
    if(len==sizeof(uint16_t)) return cps_api_object_attr_data_u16(attr);
    if(len==sizeof(uint32_t)) return cps_api_object_attr_data_u32(attr);
    return 0;
}

void cps_api_object_it_from_attr(cps_api_object_attr_t attr, cps_api_object_it_t *iter) {
    iter->len = std_tlv_total_len(attr);
    iter->attr = attr;
}

const char * cps_api_object_attr_to_string(cps_api_object_attr_t attr, char *buff, size_t len) {
	std::string _s;
	_s = cps_string::sprintf("Attr %X, Len %d\n",(int)cps_api_object_attr_id(attr),
            (int)cps_api_object_attr_len(attr));
	_s += cps_string::tostring(cps_api_object_attr_data_bin(attr),cps_api_object_attr_len(attr));
	strncpy(buff,_s.c_str(),len);
	buff[len-1] = '\0';

    return buff;
}

void cps_api_object_attr_id_as_enum(cps_api_object_attr_t attr, void *enumptr) {
    *reinterpret_cast<int*>(enumptr) = static_cast<int>(std_tlv_tag(attr));
}

cps_api_attr_id_t cps_api_object_attr_id(cps_api_object_attr_t attr) {
    return std_tlv_tag(attr);
}

size_t cps_api_object_attr_len(cps_api_object_attr_t attr) {
    return (size_t)std_tlv_len(attr);
}

uint16_t cps_api_object_attr_data_u16(cps_api_object_attr_t attr) {
    return std_tlv_data_u16(attr);
}

uint32_t cps_api_object_attr_data_u32(cps_api_object_attr_t attr) {
    return std_tlv_data_u32(attr);
}

uint64_t cps_api_object_attr_data_u64(cps_api_object_attr_t attr) {
    return std_tlv_data_u64(attr);
}

void *cps_api_object_attr_data_bin(cps_api_object_attr_t attr) {
    return std_tlv_data(attr);
}

int cps_api_object_attrs_compare(cps_api_object_attr_t lhs, cps_api_object_attr_t rhs) {
    size_t len_l = cps_api_object_attr_len(lhs);
    size_t len_r = cps_api_object_attr_len(rhs);
    size_t smallest = std::min(len_l,len_r);
    int rc = memcmp( cps_api_object_attr_data_bin(lhs),
            cps_api_object_attr_data_bin(rhs),smallest);
    if (rc!=0) return rc;
    return  len_l - len_r;
}

void cps_api_object_it_attr_replace(const cps_api_object_it_t *it, const cps_api_attr_id_t *match,
        const cps_api_attr_id_t *replace, size_t len) {
    cps_api_object_it_t cp = *it;
    while (cps_api_object_it_valid(&cp)) {
        cps_api_attr_id_t id = cps_api_object_attr_id(cp.attr);
        size_t ix = 0;
        for ( ; ix < len ; ++ix ) {
            if (id==match[ix]) std_tlv_set_tag(cp.attr,replace[ix]);
        }
        cps_api_object_it_next(&cp);
    }
}

bool cps_api_object_it_attr_walk(cps_api_object_it_t *it, cps_api_attr_id_t attr_id) {
    cps_api_object_attr_t attr = cps_api_object_it_find(it,attr_id);
    if (attr==NULL) {
        it->attr = ((char*)it->attr)+it->len;
        it->len = 0;
        return false;
    }
    size_t offset =  ((char*)attr - (char*)it->attr);
    STD_ASSERT(offset < it->len);
    it->len -= offset;    //Calculate offset of returned data
    it->attr = attr;
    return true;
}


}
