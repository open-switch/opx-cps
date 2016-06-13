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
 * cps_dictionary.cpp
 *
 *  Created on: Aug 5, 2015
 */


#include "private/cps_dictionary.h"
#include "cps_api_key_cache.h"
#include "cps_api_key.h"
#include "cps_api_object_attr.h"


#include "std_mutex_lock.h"

#include <unordered_map>
#include <memory>


struct enum_field_t {
    std::string name;
    int value;
    std::string descr;
};

class CPSEnum {
    std::string _desc;
    std::string _name;
    std::vector<std::unique_ptr<enum_field_t>> _fields;

    std::unordered_map<std::string,int> _str_to_id;
    std::unordered_map<int,const char*> _id_to_str;

public:
    CPSEnum(){}
    CPSEnum(const char *name, const char *desc) {
        _desc = desc;
        _name = name;
    }
    const std::string & name() const { return _name; }
    const std::string & desc() const { return _desc; }

    void reg(const char *name, int id, const char *desc);
    int value(const char *name) const ;
    const char *name(int value) const;
};

using cps_class_map_type_t = std::unordered_map<cps_api_attr_id_t,std::unique_ptr<cps_class_map_node_details_int_t>>;
using cps_class_map_string_t = std::unordered_map<std::string,cps_class_map_node_details_int_t*>;
using cps_class_map_enums_t = std::unordered_map<std::string,CPSEnum>;
using cps_class_map_id_to_enum_t = std::unordered_map<cps_api_attr_id_t,std::string>;
using cps_class_map_key_to_map_element = cps_api_key_cache<cps_class_map_node_details_int_t*>;
using cps_class_map_key_to_type = cps_api_key_cache<CPS_API_OBJECT_OWNER_TYPE_t>;
using cps_class_map_qual_to_string = std::unordered_map<int,std::string>;


static std_mutex_lock_create_static_init_rec(lock);
static cps_class_map_type_t _class_def;
static cps_class_map_string_t _str_map;
static cps_class_map_enums_t _enum_map;
static cps_class_map_id_to_enum_t _attr_id_to_enum;
static cps_class_map_key_to_map_element _key_to_map_element;
static cps_class_map_key_to_type _key_storage_type;
const static size_t NO_OFFSET=0;
static const cps_class_map_qual_to_string _qual_to_string = {
        {cps_api_qualifier_TARGET, "target"}, 
        {cps_api_qualifier_OBSERVED, "observed"}, 
        {cps_api_qualifier_PROPOSED, "proposed"}, 
        {cps_api_qualifier_REALTIME, "realtime"}, 
        {cps_api_qualifier_REGISTRATION, "registration"}
};



void CPSEnum::reg(const char *name, int id, const char *desc) {
    auto p = std::unique_ptr<enum_field_t>(new enum_field_t);
    p->descr = desc;
    p->name = name;
    p->value = id;

    const char *_name = p->name.c_str();

    _fields.push_back(std::move(p));
    _str_to_id[_name] = id;
    _id_to_str[id] = name;
}

int CPSEnum::value(const char *name) const {
    auto it = _str_to_id.find(name);
    if (it==_str_to_id.end()) return -1;
    return it->second;
}
const char *CPSEnum::name(int value) const  {
    auto it = _id_to_str.find(value);
    if (it==_id_to_str.end()) return nullptr;
    return it->second;
}

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

    cps_api_key_t _key;
    cps_api_key_init_from_attr_array(&_key,&ref.ids[0],ref.ids.size(),NO_OFFSET);

    _key_to_map_element.insert(&_key,p.get());
    _str_map[p->full_path] = p.get();
    _class_def[id] = std::move(p);
    return cps_api_ret_code_OK;
}

bool cps_api_key_from_attr(cps_api_key_t *key,cps_api_attr_id_t id, size_t key_start_pos) {
	cps_api_key_set_len(key,0);
	cps_api_key_set_attr(key,0);

    std_mutex_simple_lock_guard lg(&lock);

    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    if (it==nullptr) return false;

    cps_api_key_set_attr(key,0);
    cps_api_key_init_from_attr_array(key,(cps_api_attr_id_t *)&(it->ids[0]),it->ids.size(),key_start_pos);
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
    std_mutex_simple_lock_guard lg(&lock);

    cps_api_key_t key;
    cps_api_key_init_from_attr_array(&key,(cps_api_attr_id_t *)ids,ids_len,NO_OFFSET);
    cps_class_map_node_details_int_t *p = nullptr;
    _key_to_map_element.find(&key,p,true);

    if (p==nullptr) return nullptr;
    return p->name.c_str();
}

bool cps_class_attr_is_embedded(const cps_api_attr_id_t *ids, size_t ids_len) {
    std_mutex_simple_lock_guard lg(&lock);

    cps_api_key_t key;
    cps_api_key_init_from_attr_array(&key,(cps_api_attr_id_t *)ids,ids_len,NO_OFFSET);
    cps_class_map_node_details_int_t *p = nullptr;
    _key_to_map_element.find(&key,p,true);

    if (p==nullptr) return nullptr;

    return p->embedded;
}

bool cps_class_attr_is_valid(const cps_api_attr_id_t *ids, size_t ids_len) {
    std_mutex_simple_lock_guard lg(&lock);

    cps_api_key_t key;
    cps_api_key_init_from_attr_array(&key,(cps_api_attr_id_t *)ids,ids_len,NO_OFFSET);
    cps_class_map_node_details_int_t *p = nullptr;
    _key_to_map_element.find(&key,p,true);

    return (p!=nullptr);
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

const char * cps_class_string_from_key(cps_api_key_t *key, size_t offset) {
    cps_api_key_t _key ;
    //if the offset is !=0... then need to do some additional processing..
    if (offset > 0) {
        cps_api_key_copy(&_key,key);
        for (size_t ix = 0; ix < offset; ++ix)
            cps_api_key_remove_element(&_key, 0);
        key = &_key;
    }

    cps_class_map_node_details_int_t *ref= nullptr;
    std_mutex_simple_lock_guard lg(&lock);
    _key_to_map_element.find(key,ref,true);
    if (ref==nullptr) return nullptr;
    return ref->full_path.c_str();
}

//Assumption: The first field of the key is the qualifier
const char * cps_class_qual_from_key(cps_api_key_t *key) {
    static const int QUAL_POS=0;

    cps_api_key_element_t qual = cps_api_key_element_at(key, QUAL_POS);
    auto it = _qual_to_string.find((cps_api_qualifier_t)qual);
    
    if (it!=_qual_to_string.end()) {
        return (it->second.c_str());
    }

    return nullptr;
}

cps_api_return_code_t cps_class_map_enum_reg(const char *enum_name, const char *field, int value, const char * descr) {
    auto it = _enum_map.find(enum_name);
    if (it==_enum_map.end()) {
        _enum_map[enum_name] = std::move(CPSEnum(enum_name, enum_name));
        it = _enum_map.find(enum_name);
    }
    if (it==_enum_map.end()) {
        return cps_api_ret_code_ERR;
    }
    it->second.reg(field,value,descr);
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_class_map_enum_associate(cps_api_attr_id_t id, const char *name) {
    _attr_id_to_enum[id] = name;
    return cps_api_ret_code_OK;
}

const char *cps_class_enum_id(cps_api_attr_id_t id, int val) {
    auto it = _attr_id_to_enum.find(id);
    if (it==_attr_id_to_enum.end()) return nullptr;

    auto eit = _enum_map.find(it->second.c_str());
    if (eit == _enum_map.end()) return nullptr;

    return eit->second.name(val);
}

int    cps_api_enum_value(cps_api_attr_id_t id, const char *tag) {
    auto it = _attr_id_to_enum.find(id);
    if (it==_attr_id_to_enum.end()) return -1;

    auto eit = _enum_map.find(it->second.c_str());
    if (eit == _enum_map.end()) return -1;
    return eit->second.value(tag);
}

bool cps_class_map_attr_type(cps_api_attr_id_t id, CPS_CLASS_DATA_TYPE_t *t) {
    const cps_class_map_node_details_int_t * it = cps_dict_find_by_id(id);
    if (it==nullptr) return false;
    *t = it->data_type;
    return true;
}

CPS_API_OBJECT_OWNER_TYPE_t cps_api_obj_get_ownership_type(cps_api_object_t obj) {
    std_mutex_simple_lock_guard lg(&lock);
    CPS_API_OBJECT_OWNER_TYPE_t *p = _key_storage_type.at(cps_api_object_key(obj),true);
    if (p==nullptr) return CPS_API_OBJECT_SERVICE;
    return *p;
}

void cps_api_obj_set_ownership_type(cps_api_key_t *key, CPS_API_OBJECT_OWNER_TYPE_t type) {
    std_mutex_simple_lock_guard lg(&lock);
    _key_storage_type.insert(key,type);
}
