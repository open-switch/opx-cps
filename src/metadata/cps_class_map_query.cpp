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



#include "cps_class_map.h"

#include "private/cps_class_map_query.h"
#include "private/cps_dictionary.h"

#include "std_utils.h"

#include <limits.h>
#include <stdio.h>
#include <inttypes.h>

void cps_class_ids_from_key(std::vector<cps_api_attr_id_t> &v, cps_api_key_t *key) {
    v.resize(0);
    size_t max_len = cps_api_key_get_len(key);
    if (max_len < 1 ) return;
    --max_len;

    for ( ; max_len > 0 ; --max_len) {
        cps_api_attr_id_t id = cps_api_key_element_at(key,max_len);
        const cps_class_map_node_details_int_t *ent = cps_dict_find_by_id(id);
        if (ent==nullptr) continue;
        if (ent->embedded) {
            v = ent->ids;
            break;
        }
    }
}

bool cps_class_map_detail(const cps_api_attr_id_t id, cps_class_map_node_details_int_t &details) {
    const cps_class_map_node_details_int_t *ent = cps_dict_find_by_id(id);
    if (ent==nullptr) return false;
    details = *ent;
    return true;
}

void cps_class_ids_from_string(std::vector<cps_api_attr_id_t> &v, const char * str) {
    std_parsed_string_t handle;
    if (!std_parse_string(&handle,str,".")) return ;

    size_t ix = 0;
    size_t mx = std_parse_string_num_tokens(handle);
    v.resize(mx);
    for ( ; ix < mx ; ++ix ) {
        unsigned long int ul =strtoul(std_parse_string_at(handle,ix),NULL,0);
        if (ul==ULONG_MAX && errno==ERANGE) {
            break;
        }
        v[ix]= ((cps_api_attr_id_t)ul);
    }
    std_parse_string_free(handle);
}

std::string cps_class_ids_to_string(const std::vector<cps_api_attr_id_t> &v) {
    std::string s;
    static const int BUFF_LEN=1024;
    char buff[BUFF_LEN];
    bool first_time = true;
    size_t ix = 0;
    size_t mx = v.size();
    for ( ; ix < mx ; ++ix ) {
        snprintf(buff,sizeof(buff)-1,"%" PRId64,(int64_t)v[ix]);
        if (!first_time) s+=".";
        first_time=false;
        s+=buff;
    }
    return std::move(s);
}

std::string cps_key_to_string(const cps_api_key_t * key) {
    const static size_t BUFF_LEN=1024;
    char buff[BUFF_LEN];
    return std::string(cps_api_key_print(const_cast<cps_api_key_t*>(key),buff,sizeof(buff)));
}

static bool cps_attr_id_key_compare_match(const cps_api_attr_id_t *ids_a, size_t ids_len_a,
        const cps_api_attr_id_t *ids_b, size_t ids_len_b) {
    size_t ix = 0;
    if (ids_len_a <ids_len_b) return false;

    size_t mx = std::min(ids_len_a,ids_len_b);
    for ( ; ix < mx ; ++ix) {
        if (ids_a[ix] !=  ids_b[ix]) return false;
    }
    return true;
}

struct walk_struct {
    cps_class_node_detail_list_t *lst;
    const cps_api_attr_id_t *ids;
    size_t max_ids;
    bool current_level;
};

static bool __cps_dict_walk_fun__(void * context, const cps_class_map_node_details_int_t *ptr) {
    walk_struct *ws = (walk_struct*)context;
    if (ws->current_level) {
        //filter non matching level
        if (ptr->ids.size()!=(ws->max_ids+1))
            return true;
    }
    if (!cps_attr_id_key_compare_match( &(ptr->ids[0]),ptr->ids.size(),ws->ids,ws->max_ids)) return true;
    ws->lst->push_back(*ptr);
    return true;
}

void cps_class_map_level(const cps_api_attr_id_t *ids, size_t max_ids,
        cps_class_node_detail_list_t &details, bool only_children) {
    walk_struct ws;
    ws.ids = ids;
    ws.max_ids = max_ids;
    ws.lst = &details;
    ws.current_level = only_children;

    cps_dict_walk(&ws, __cps_dict_walk_fun__) ;
}
