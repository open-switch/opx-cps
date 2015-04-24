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
#include "private/cps_class_map_query.h"
#include "std_utils.h"


#include <errno.h>
#include <limits.h>
#include <string.h>
#include <map>
#include <string>
#include <algorithm>
#include <ctype.h>

class cps_class_map_key {
public:
    using vector = std::vector<cps_api_attr_id_t>;
public:
    cps_class_map_key(){}
    auto size() const -> std::vector<cps_api_attr_id_t>::size_type {
        return _ids.size();
    }
    auto operator[](size_t ix) const -> std::vector<cps_api_attr_id_t>::const_reference {
        return _ids[ix];
    }
    cps_class_map_key(vector &&v) {
        _ids = std::move(v);
    }

    //require move constructor
    cps_class_map_key(cps_class_map_key &&k)  = default;

    //requried because constructor
    cps_class_map_key& operator=(const cps_class_map_key&k) = default;

    const vector &get() const { return _ids; }

    cps_class_map_key(const cps_api_key_t *key,const cps_api_attr_id_t *ids, size_t ids_len) {
        size_t klen = 0;
        size_t len = ids_len;
        if (key!=NULL) {
            klen = cps_api_key_get_len(key);
        }
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

static bool key_compare_less(const cps_api_attr_id_t *ids_a, size_t ids_len_a,
        const cps_api_attr_id_t *ids_b, size_t ids_len_b) {
    size_t ix = 0;
    size_t mx = std::min(ids_len_a,ids_len_b);
    for ( ; ix < mx ; ++ix) {
        if (ids_a[ix] ==  ids_b[ix]) continue;
        if (ids_a[ix] <  ids_b[ix]) return true;
        return false;
    }
    return (ids_len_a<ids_len_b);
}

struct cps_class_map_key_comp {
    bool operator()( const cps_class_map_key &lhs, const cps_class_map_key &rhs) const {
        return key_compare_less(&(lhs.get()[0]),lhs.get().size(),&(rhs.get()[0]),rhs.get().size());
    }
};

static bool key_compare_match(const cps_api_attr_id_t *ids_a, size_t ids_len_a,
        const cps_api_attr_id_t *ids_b, size_t ids_len_b) {
    size_t ix = 0;
    if (ids_len_a <ids_len_b) return false;

    size_t mx = std::min(ids_len_a,ids_len_b);
    for ( ; ix < mx ; ++ix) {
        if (ids_a[ix] !=  ids_b[ix]) return false;
    }
    return true;
}

struct cps_class_map_key_subcomp {
    bool operator()( const cps_class_map_key &lhs, const cps_class_map_key &rhs) const {
        if (lhs.get().size()< rhs.get().size()) return false;
        size_t mx = std::min(lhs.get().size(),rhs.get().size());
        return key_compare_match(&(lhs.get()[0]),mx,&(rhs.get()[0]),mx);
    }
};

using cps_class_map_type_t = std::map<cps_class_map_key,cps_class_map_node_details_int_t,cps_class_map_key_comp>;
using cps_class_map_reverse_string_t = std::map<std::string,cps_class_map_key>;


static std_mutex_lock_create_static_init_fast(lock);

extern "C" {

static cps_class_map_type_t _cmt;
static cps_class_map_reverse_string_t _rev_string;

cps_api_return_code_t cps_class_map_init(const cps_api_attr_id_t *ids, size_t ids_len, cps_class_map_node_details *details) {
    cps_class_map_key::vector v(ids,ids+(ids_len));
    cps_class_map_key::vector cloned = v;

    cps_class_map_key k(std::move(v));
    std_mutex_simple_lock_guard lg(&lock);

    if (_cmt.find(k)!=_cmt.end()) {
        return cps_api_ret_code_ERR;
    }

    _rev_string[details->name] = k;

    std::string name = details->name;
    if (name.find('/')!=std::string::npos) {
        name = name.substr(name.rfind('/')+1);
    }

    cps_class_map_node_details_int_t &ref = _cmt[std::move(k)];
    ref.ids = std::move(cloned);
    ref.desc = details->desc;
    ref.name = std::move(name);
    ref.full_path = details->name;
    ref.embedded = details->embedded;
    ref.type = details->type;
    ref.id = ids[ids_len-1];

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

bool cps_class_string_to_key(const char *str, cps_api_attr_id_t *ids, size_t *max_ids) {
    const auto it = _rev_string.find(str);
    if (it==_rev_string.end()) return false;
    if (*max_ids >= it->second.get().size()) {
        *max_ids = it->second.get().size();
    }
    size_t ix = 0;
    for ( ; ix < *max_ids ; ++ix ) {
        ids[ix] = it->second[ix];
    }
    return true;
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
    static const int BUFF_LEN=100;
    char buff[BUFF_LEN];
    bool first_time = true;
    size_t ix = 0;
    size_t mx = v.size();
    for ( ; ix < mx ; ++ix ) {
        snprintf(buff,sizeof(buff)-1,"%d",(int)v[ix]);
        if (!first_time) s+=".";
        first_time=false;
        s+=buff;
    }
    return std::move(s);
}

void cps_class_map_level(const cps_api_attr_id_t *ids, size_t max_ids,
        cps_class_node_detail_list_t &details) {
    cps_class_map_key k(nullptr,ids,max_ids);
    std_mutex_simple_lock_guard lg(&lock);

    auto it = _cmt.cbegin();
    auto end = _cmt.cend();
    for ( ; it != end ; ++it ) {
        bool match = cps_class_map_key_subcomp()(it->first,k);
        if (match) {
            details.push_back(it->second);
        }
    }
}

bool cps_class_map_query(const cps_api_attr_id_t *ids, size_t max_ids, const char * node,
        cps_class_map_node_details_int_t &details){
    cps_class_node_detail_list_t lst;
    cps_class_map_level(ids,max_ids,lst);
    size_t ix = 0;
    size_t mx = lst.size();
    for ( ; ix < mx ; ++ix ) {
        if ((lst[ix].name == node) || (lst[ix].full_path==node) ||
                (isdigit(node[0]) && atoi(node)==(int)lst[ix].id)) {
            details = lst[ix];
            return true;
        }
    }
    return false;
}
