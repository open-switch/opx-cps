/** OPENSOURCELICENSE */
/*
 * cps_api_object_attr.cpp
 *
 *  Created on: Jun 4, 2015
 */


#include "cps_api_object_attr.h"

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
    snprintf(buff,len,"Attr %X, Len %d",(int)cps_api_object_attr_id(attr),
            (int)cps_api_object_attr_len(attr));
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

}