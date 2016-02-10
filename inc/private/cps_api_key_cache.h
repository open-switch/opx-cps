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
 * cps_api_key_cache.h
 *
 *  Created on: Sep 24, 2015
 */

#ifndef CPS_API_INC_PRIVATE_CPS_API_KEY_CACHE_H_
#define CPS_API_INC_PRIVATE_CPS_API_KEY_CACHE_H_

/** @addtogroup CPSAPI
 *  @{
 *
 *  @addtogroup Internal Internal Headers
 *  @warning this is an internal API.  Do not use directly
 *  @{
*/

#include "cps_api_key.h"

#include <vector>
#include <unordered_map>

template <typename data_type>
class cps_api_key_cache {
public:
    struct field_entry {
        cps_api_key_t key;
        data_type data;
    };
    using cache_data = std::unordered_map<size_t,std::vector<field_entry>>;
    using cache_data_iterator = typename cache_data::iterator;
private:
    cache_data _cache;
public:
    bool insert(cps_api_key_t *key, const data_type &dt);
    void erase(cps_api_key_t *key);
    bool find(cps_api_key_t *key, data_type &dt, bool exact);
    data_type * at(cps_api_key_t *key,bool exact);

    class cache_walker {
        bool operator()(cps_api_key_cache<data_type>::cache_data_iterator &it);
    };

    /**
     * Walk the internal tree - if the length of the vector is zero at the iterator, it will be cleaned
     * up once the callback is returned - the walk may restart if that happens
     *
     * @param cb the callback used to walk each node
     */
    template <typename walking_function>
    void walk(walking_function cb);
private:
    bool find_entry(cps_api_key_t *key, std::vector<field_entry> **ent, size_t &ix, bool exact=true);
    bool find_entry(cps_api_key_t *key, std::vector<field_entry> **ent, size_t &ix, size_t key_ken=0);
};

template <typename data_type>
bool cps_api_key_cache<data_type>::find_entry(cps_api_key_t *key, std::vector<field_entry> **ent, size_t &ix, bool exact) {
    if (exact) return find_entry(key,ent,ix,(size_t)0);    //need to specify the arguments to avoid the wrong overload
    size_t key_len = cps_api_key_get_len(key);
    while (key_len>0) {
        if (find_entry(key,ent,ix,key_len--)) {
            return true;
        }
    }
    return false;
}

template <typename data_type>
bool cps_api_key_cache<data_type>::find_entry(cps_api_key_t *key, std::vector<field_entry> **ent, size_t &_ix,
        size_t key_len) {

    cps_api_key_t _key;
    if (key_len>0) {
        cps_api_key_copy(&_key,key);
        cps_api_key_set_len(&_key,key_len);
        key = &_key;
    }

    uint64_t hash = cps_api_key_hash(key);
    auto it = _cache.find(hash);
    if (it==_cache.end()) return false;

    auto & v = it->second;
    for (size_t ix = 0, mx = v.size(); ix < mx ; ++ix ) {
        if (cps_api_key_matches(&v[ix].key,key,true)==0) {
            *ent = &v;
            _ix = ix;
            return true;
        }
    }
    return false;
}

template <typename data_type>
bool cps_api_key_cache<data_type>::insert(cps_api_key_t *key, const data_type &dt) {
    uint64_t hash = cps_api_key_hash(key);
    try {
        field_entry fe;
        fe.data = dt;
        cps_api_key_copy(&fe.key,key);
        _cache[hash].push_back(std::move(fe));
    } catch (...) {
        return false;
    }
    return true;
}


template <typename data_type>
void cps_api_key_cache<data_type>::erase(cps_api_key_t *key) {
    std::vector<field_entry> *v;
    size_t ix = 0;
    if (find_entry(key,&v,ix,true)) {
        (*v).erase((*v).begin()+ix);
    }
}

template <typename data_type>
bool cps_api_key_cache<data_type>::find(cps_api_key_t *key, data_type &dt, bool exact) {
    std::vector<field_entry> *v;
    size_t ix = 0;
    if (find_entry(key,&v,ix,exact)) {
        dt = (*v)[ix].data;
        return true;
    }
    return false;
}

template <typename data_type>
data_type * cps_api_key_cache<data_type>::at(cps_api_key_t *key,bool exact) {
    std::vector<field_entry> *v;
    size_t ix = 0;
    if (find_entry(key,&v,ix,exact)) {
        return &(*v)[ix].data;
    }
    return nullptr;
}
template <typename data_type>
template <typename walking_function>
void cps_api_key_cache<data_type>::walk(walking_function cb) {
    auto it = _cache.begin();
    auto end = _cache.end();
    for ( ; it != end ; ++it ) {
        bool res = cb(it);
        if (!res || it->second.size()==0) {
            _cache.erase(it);
            it = _cache.begin();
        }

    }
}

/**
 * @}
 * @}
 */

#endif /* CPS_API_INC_PRIVATE_CPS_API_KEY_CACHE_H_ */
