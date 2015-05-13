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
    cps_class_map_key(const vector &ids) {
        _ids = ids;
    }

    //require move constructor
    cps_class_map_key(cps_class_map_key &&k)  = default;

    //Required because constructor
    cps_class_map_key& operator=(const cps_class_map_key&k) = default;

    const vector &get() const { return _ids; }

    cps_class_map_key(const cps_api_attr_id_t *ids, size_t ids_len) {
        _ids = vector(ids,ids+ids_len);
    }

private:
    vector _ids;
};

using ids_to_string_map = std::map<cps_api_attr_id_t,std::string>;

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

using cps_class_map_type_t = std::map<cps_api_attr_id_t,cps_class_map_node_details_int_t>;
using cps_class_map_reverse_string_t = std::map<std::string,cps_api_attr_id_t>;


static std_mutex_lock_create_static_init_fast(lock);
static ids_to_string_map _id_to_string;
static cps_class_map_type_t _cmt;
static cps_class_map_reverse_string_t _rev_string;

void cps_class_ids_from_key(std::vector<cps_api_attr_id_t> &v,
        cps_api_key_t *key) {
    v.resize(0);
    size_t max_len = cps_api_key_get_len(key);
    if (max_len < 1 ) return;
    --max_len;

    for ( ; max_len > 0 ; --max_len) {
        cps_api_attr_id_t id = cps_api_key_element_at(key,max_len);
        auto it = _cmt.find(id);
        if (it==_cmt.end()) continue;
        if (it->second.embedded) {
            v = it->second.ids;
            break;
        }
    }
}

extern "C" {


const char * cps_attr_id_to_name(cps_api_attr_id_t id) {
    auto it = _cmt.find(id);
    if (it!=_cmt.end()) {
        return it->second.full_path.c_str();
    }
    return NULL;
}

cps_api_attr_id_t cps_name_to_attr(const char *name) {
    auto it = _rev_string.find(name);
    if (it!=_rev_string.end()) {
        return it->second;
    }
    return cps_api_attr_id_t(-1);
}

bool cps_api_key_from_attr(cps_api_key_t *key,cps_api_attr_id_t id, size_t key_start_pos) {
    auto it = _cmt.find(id);
    if (it ==_cmt.end()) return false;
    size_t ix = 0;
    size_t mx = it->second.ids.size();
    for ( ; ix < mx ; ++ix ){
        cps_api_key_set(key,key_start_pos+ix,(uint32_t)it->second.ids[ix]);
    }
    cps_api_key_set_len(key,key_start_pos+ix);
    return true;
}

bool cps_api_key_from_attr_with_qual(cps_api_key_t *key,cps_api_attr_id_t id,
        cps_api_qualifier_t cat) {
    bool rc = cps_api_key_from_attr(key,id,CPS_OBJ_KEY_INST_POS+1);
    if (!rc) return false;
    cps_api_key_set(key,CPS_OBJ_KEY_INST_POS,cat);
    return true;
}

cps_api_return_code_t cps_class_map_init(cps_api_attr_id_t id, const cps_api_attr_id_t *ids, size_t ids_len, cps_class_map_node_details *details) {
    cps_class_map_key::vector v(ids,ids+(ids_len));
    if(ids_len==0) return cps_api_ret_code_ERR;

    if (_cmt.find(ids[ids_len-1])!=_cmt.end()) {
        return cps_api_ret_code_ERR;
    }

    std::string name = details->name;
    if (name.find('/')!=std::string::npos) {
        name = name.substr(name.rfind('/')+1);
    }

    cps_class_map_node_details_int_t &ref = _cmt[id];
    ref.ids = std::move(v);
    ref.desc = details->desc;
    ref.name = std::move(name);
    ref.full_path = details->name;
    ref.embedded = details->embedded;
    ref.type = details->type;
    ref.id = id;

    _rev_string[ref.full_path] = ref.id;
    return cps_api_ret_code_OK;
}

bool cps_class_attr_is_embedded(const cps_api_attr_id_t *ids, size_t ids_len) {
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(ids[ids_len-1]);
    if (it==_cmt.end()) return false;
    return it->second.embedded;
}

bool cps_class_attr_is_valid(const cps_api_attr_id_t *ids, size_t ids_len) {
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(ids[ids_len-1]);
    if (it==_cmt.end()) return false;
    return true;
}
const char * cps_class_attr_name(const cps_api_attr_id_t *ids, size_t ids_len) {
    std_mutex_simple_lock_guard lg(&lock);
    const auto it = _cmt.find(ids[ids_len-1]);
    if (it==_cmt.end()) return false;
    return it->second.name.c_str();
}

bool cps_class_string_to_key(const char *str, cps_api_attr_id_t *ids, size_t *max_ids) {
    auto nit = _rev_string.find(str);
    if (nit!=_rev_string.end()) return false;

    auto it = _cmt.find(nit->second);
    if (it==_cmt.end()) return false;

    size_t ix = 0;
    size_t mx = std::min((size_t)*max_ids,(size_t)it->second.ids.size());
    *max_ids = mx;
    for ( ; ix < mx ; ++ix ) {
        ids[ix] = it->second.ids[ix];
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

bool cps_api_key_to_class_attr(cps_api_attr_id_t *ids, size_t ids_len, const cps_api_key_t * key) {
    size_t klen = cps_api_key_get_len(const_cast<cps_api_key_t *>(key));
    if (ids_len < klen) {
        return false;
    }
    size_t ix = 0;
    for ( ; ix < klen ; ++ix ) {
        ids[ix] = cps_api_key_element_at(const_cast<cps_api_key_t *>(key),ix);
    }
    return true;
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
    static const int BUFF_LEN=1024;
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

std::string cps_key_to_string(const cps_api_key_t * key) {
    const static size_t BUFF_LEN=1024;
    char buff[BUFF_LEN];
    return std::string(cps_api_key_print(const_cast<cps_api_key_t*>(key),buff,sizeof(buff)));
}

void cps_class_map_level(const cps_api_attr_id_t *ids, size_t max_ids,
        cps_class_node_detail_list_t &details) {
    cps_class_map_key k(ids,max_ids);
    std_mutex_simple_lock_guard lg(&lock);

    auto it = _cmt.cbegin();
    auto end = _cmt.cend();
    for ( ; it != end ; ++it ) {
        cps_class_map_key k2(it->second.ids);
        bool match = cps_class_map_key_subcomp()(k2,k);
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

