/** OPENSOURCELICENSE */
/*
 * cps_class_map.cpp
 *
 *  Created on: Apr 19, 2015
 */

#include "cps_class_map.h"
#include "std_mutex_lock.h"
#include "cps_api_key.h"
#include "std_directory.h"
#include "std_shlib.h"
#include "event_log.h"

#include <string.h>
#include <map>
#include <string>
#include <algorithm>

static std_mutex_lock_create_static_init_fast(lock);

class cps_class_map_key {
public:
    using vector = std::vector<cps_api_attr_id_t>;
public:
    auto size() const -> std::vector<cps_api_attr_id_t>::size_type {
        return _ids.size();
    }
    auto operator[](size_t ix) const -> std::vector<cps_api_attr_id_t>::const_reference {
        return _ids[ix];
    }
    cps_class_map_key & operator=(cps_class_map_key&& rhs) {
        _ids = std::move(rhs._ids);
        return *this;
    }
    cps_class_map_key(vector &&v) {
        _ids = std::move(v);
    }
    cps_class_map_key(cps_class_map_key &&k) {
        *this = std::move(k);
    }
    cps_class_map_key(const cps_api_key_t *key,const cps_api_attr_id_t *ids, size_t ids_len) {
        size_t klen = cps_api_key_get_len(key);
        size_t len = ids_len;
        if(klen>0) {
            len+=klen-1;
        }

        _ids.resize(len);
        size_t ix = 0;
        if (klen>0){
            klen -=1;
            const uint32_t *elem = cps_api_key_elem_start(const_cast<cps_api_key_t *>(key))+1;

            for ( ; ix < klen ; ++ix ) {
                _ids[ix]= elem[ix];
            }
        }
        for (size_t ids_ix=0 ;ix < len ; ++ix,++ids_ix ) {
            _ids[ix] = ids[ids_ix];
        }
    }
private:
    vector _ids;
};

struct cps_class_map_key_comp {
    bool operator()( const cps_class_map_key &lhs, const cps_class_map_key &rhs) const {
        size_t ix = 0;
        size_t mx = std::min(lhs.size(),rhs.size());
        for ( ; ix < mx ; ++ix) {
            if (lhs[ix] ==  rhs[ix]) continue;
            if (lhs[ix] <  rhs[ix]) return true;
            return false;
        }
        return (lhs.size()<rhs.size());
    }

};

struct cps_class_map_node_details_int_t {
    std::string name;
    std::string desc;
    bool embedded;
    cps_api_object_ATTR_TYPE_t type;
};

extern "C" {

using cps_class_map_type_t = std::map<cps_class_map_key,cps_class_map_node_details_int_t,cps_class_map_key_comp>;
using cps_class_map_reverse_string_t = std::map<std::string,cps_class_map_key>;

static cps_class_map_type_t _cmt;
static cps_class_map_reverse_string_t _rev_string;

cps_api_return_code_t cps_class_map_init(const cps_api_attr_id_t *ids, size_t ids_len, cps_class_map_node_details *details) {
    cps_class_map_key::vector v(ids,ids+(ids_len));
    cps_class_map_key k(std::move(v));
    std_mutex_simple_lock_guard lg(&lock);
    if (_cmt.find(k)!=_cmt.end()) {
        return cps_api_ret_code_ERR;
    }

    _rev_string[details->name] = k;

    cps_class_map_node_details_int_t &ref = _cmt[std::move(k)];
    ref.desc = details->desc;
    ref.name = details->name;
    ref.embedded = details->embedded;
    ref.type = details->type;

    return cps_api_ret_code_OK;
}

bool cps_class_attr_is_embedded(const cps_api_key_t *key,const cps_api_attr_id_t *ids, size_t ids_len) {
    cps_class_map_key k(key,ids,ids_len);
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(k);
    if (it==_cmt.end()) return false;
    return it->second.embedded;
}
bool cps_class_attr_is_valid(const cps_api_key_t *key,const cps_api_attr_id_t *ids, size_t ids_len) {
    cps_class_map_key k(key,ids,ids_len);
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(k);
    if (it==_cmt.end()) return false;
    return true;
}
const char * cps_class_attr_name(const cps_api_key_t *key,const cps_api_attr_id_t *ids, size_t ids_len) {
    cps_class_map_key k(key,ids,ids_len);
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(k);
    if (it==_cmt.end()) return false;
    return it->second.name.c_str();
}

static bool _cps_class_data(const char *name, std_dir_file_TYPE_t type,void *context) {
    if (type != std_dir_file_T_FILE) return true;

    if (strstr(name,(const char*)context)!=NULL) {
        void (*class_data_init)(void);
         static std_shlib_func_map_t func_map[] = {
             { "module_init", (void **)&class_data_init }
         };
         static const size_t func_map_size = sizeof(func_map)/sizeof(*func_map);
         std_shlib_hndl lib_hndl = STD_SHLIB_INVALID_HNDL;

         if (STD_ERR_OK != std_shlib_load(name, &lib_hndl, func_map, func_map_size)) {
             EV_LOG(ERR,DSAPI,0,"cps_class_data","Can not load function map");
         } else {
             class_data_init();
             //Since we don't need to use any functions in the library after initialized
             //then we can unload the library
             std_shlib_unload(lib_hndl);
         }
    }
    return true;
}

bool cps_class_objs_load(const char *path, const char * prefix) {
    t_std_error rc = std_dir_iterate(path,_cps_class_data,(void*)prefix,false);
    return rc==STD_ERR_OK;
}

}
