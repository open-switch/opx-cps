/** OPENSOURCELICENSE */
/*
 * cps_dictionary.cpp
 *
 *  Created on: Aug 5, 2015
 */


#include "private/cps_dictionary.h"

#include "std_mutex_lock.h"
#include "cps_api_object_attr.h"

#include <unordered_map>
#include <memory>

using cps_class_map_type_t = std::unordered_map<cps_api_attr_id_t,std::unique_ptr<cps_class_map_node_details_int_t>>;
using cps_class_map_string_t = std::unordered_map<std::string,cps_class_map_node_details_int_t*>;

static std_mutex_lock_create_static_init_rec(lock);
static cps_class_map_type_t _class_def;
static cps_class_map_string_t _str_map;


static void cps_class_data_has_been_loaded(void) {
    std_mutex_simple_lock_guard lg(&lock);
    static bool _map_init=false;
    if (!_map_init) {
        cps_api_class_map_init();
        _map_init = true;
    }
}

const cps_class_map_node_details_int_t * cps_dict_find_by_name(const char * name) {
    cps_class_data_has_been_loaded();
    const auto it = _str_map.find(name);
    if (it==_str_map.end()) return nullptr;
    return it->second;
}

const cps_class_map_node_details_int_t * cps_dict_find_by_id(cps_api_attr_id_t id) {
    cps_class_data_has_been_loaded();
    auto it = _class_def.find(id);
    if (it==_class_def.end()) return nullptr;
    return it->second.get();
}

void cps_dict_walk(void *context, cps_dict_walk_fun fun) {
    std_mutex_simple_lock_guard lg(&lock);
    cps_class_data_has_been_loaded();
    auto it = _class_def.cbegin();
    auto end = _class_def.cend();
    for ( ; it != end ; ++it ) {
        if (!fun(context,it->second.get())) break;
    }
}

extern "C" {

cps_api_return_code_t cps_class_map_init(cps_api_attr_id_t id, const cps_api_attr_id_t *ids, size_t ids_len,
        cps_class_map_node_details *details) {
    std_mutex_simple_lock_guard lg(&lock);

    std::vector<cps_api_attr_id_t> v(ids,ids+(ids_len));
    if(ids_len==0) return cps_api_ret_code_ERR;

    if (_class_def.find(id)!=_class_def.end()) {
        return cps_api_ret_code_ERR;
    }

    std::string name = details->name;
    if (name.find('/')!=std::string::npos) {
        name = name.substr(name.rfind('/')+1);
    }
    std::unique_ptr<cps_class_map_node_details_int_t> p (new (std::nothrow)  cps_class_map_node_details_int_t);
    if (p.get() == nullptr) return cps_api_ret_code_ERR;

    cps_class_map_node_details_int_t &ref = *p;

    ref.ids = std::move(v);
    ref.desc = details->desc;
    ref.name = std::move(name);
    ref.full_path = details->name;
    ref.embedded = details->embedded;
    ref.attr_type = details->attr_type;
    ref.data_type = details->data_type;
    ref.id = id;

    _str_map[p->full_path] = p.get();
    _class_def[id] = std::move(p);
    return cps_api_ret_code_OK;
}

bool cps_api_key_from_attr(cps_api_key_t *key,cps_api_attr_id_t id, size_t key_start_pos) {
    std_mutex_simple_lock_guard lg(&lock);

    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    if (it==nullptr) return false;

    size_t ix = 0;
    size_t mx = it->ids.size();
    for ( ; ix < mx ; ++ix ){
        cps_api_key_set(key,key_start_pos+ix,(uint32_t)it->ids.at(ix));
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

const char * cps_attr_id_to_name(cps_api_attr_id_t id) {
    std_mutex_simple_lock_guard lg(&lock);
    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    if (it==nullptr) return nullptr;
    return it->full_path.c_str();
}

const char * cps_class_attr_name(const cps_api_attr_id_t *ids, size_t ids_len) {
    return cps_attr_id_to_name(ids[ids_len-1]);
}

bool cps_class_attr_is_embedded(const cps_api_attr_id_t *ids, size_t ids_len) {
    cps_api_attr_id_t id = ids[ids_len-1];
    if ((id>=CPS_API_ATTR_RESERVE_RANGE_START) &&(id<=CPS_API_ATTR_RESERVE_RANGE_END)) return true;

    std_mutex_simple_lock_guard lg(&lock);
    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    if (it==nullptr) return true;
    return it->embedded;
}

bool cps_class_attr_is_valid(const cps_api_attr_id_t *ids, size_t ids_len) {
    cps_api_attr_id_t id = ids[ids_len-1];
    if ((id>=CPS_API_ATTR_RESERVE_RANGE_START) &&(id<=CPS_API_ATTR_RESERVE_RANGE_END)) return true;

    std_mutex_simple_lock_guard lg(&lock);
    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    return (it!=nullptr) ;
}

bool cps_class_string_to_key(const char *str, cps_api_attr_id_t *ids, size_t *max_ids) {
    std_mutex_simple_lock_guard lg(&lock);
    const cps_class_map_node_details_int_t * rec = cps_dict_find_by_name(str);
    if (rec==nullptr) return false;

    size_t ix = 0;
    size_t mx = std::min((size_t)*max_ids,(size_t)rec->ids.size());
    *max_ids = mx;
    for ( ; ix < mx ; ++ix ) {
        ids[ix] = rec->ids[ix];
    }
    return true;
}

}
