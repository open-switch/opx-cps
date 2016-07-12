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

#include "cps_api_metadata_import.h"
#include "cps_class_map.h"


#include "cps_class_map_query.h"

#include "cps_string_utils.h"

#include "std_config_node.h"
#include "std_envvar.h"
#include "std_directory.h"
#include "std_error_codes.h"
#include "event_log.h"


#include <unordered_map>
#include <functional>

#include <string.h>
#include <map>
#include <stdlib.h>

namespace {

bool __process_key(std::vector<cps_api_attr_id_t> &ids, const char * str_key, cps_api_attr_id_t _elem_id, const cps_class_map_node_details & details) {

    std::vector<std::string> _key_fields = cps_string::split(str_key,".");
    cps_api_attr_id_t id = 0;

    for ( auto _field : _key_fields ) {
        if (_field[0]=='[') {
            _field.erase(0,1);
            _field.erase(_field.size()-1);
            if (_field == details.name) {
                id = _elem_id;
            } else {
                cps_api_attr_id_t *_id = cps_api_attr_name_to_id(_field.c_str());
                if (_id==nullptr) return false;
                id = *_id;
            }
        } else {
            id = (cps_api_attr_id_t) strtoull(_field.c_str(),nullptr,0);
        }

        ids.push_back(id);
    }

    return true;
}
void __process_ownership(std_config_node_t node, void *user_data) {
    const char * id = std_config_attr_get(node,"id");
    const char * qualifiers = std_config_attr_get(node,"qualifiers");
    const char * owner_type = std_config_attr_get(node,"owner-type");

    if (id == nullptr || owner_type==nullptr || qualifiers==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Can't parse entry - missing data %s",id!=nullptr ? id : "nullptr",owner_type!=nullptr?owner_type:"nullptr");
        return ;
    }
    const CPS_API_OBJECT_OWNER_TYPE_t *_owner_type = cps_class_owner_type_from_string(owner_type);
    if (_owner_type==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Can't parse entry - owner type failed %s",owner_type);
        return;
    }
    cps_api_attr_id_t *_id = cps_api_attr_name_to_id(id);
    if (_id==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Can't parse entry - convert id from %s",id);
        return;
    }

    auto _qual_list = cps_string::split(qualifiers,",");

    for ( auto &_qual : _qual_list ) {
        const cps_api_qualifier_t *p = cps_class_qual_from_string(_qual.c_str());
        if (p==nullptr) {
            EV_LOG(TRACE,DSAPI,0,"CPS-META","Can't parse entry - convert from %s to qualifier failed",_qual.c_str());
            continue;
        }
        cps_api_key_t key;
        if (!cps_api_key_from_attr_with_qual(&key,*_id,*p)) {
            EV_LOG(ERR,DSAPI,0,"CPS-META","Can't convert ID to key %s",id);
            return ;
        }
        cps_api_obj_set_ownership_type(&key,*_owner_type);
    }

}

void __process_node(std_config_node_t node, void *user_data) {
    const char * key_path = std_config_attr_get(node,"key");
    const char * id = std_config_attr_get(node,"id");
    const char * name = std_config_attr_get(node,"name");
    const char * desc = std_config_attr_get(node,"desc");
    const char * embedded = std_config_attr_get(node,"embedded");
    const char * node_type = std_config_attr_get(node,"node-type");
    const char * data_type = std_config_attr_get(node,"data-type");

    if (data_type==nullptr) data_type = "bin";

    if (embedded==nullptr) {
        embedded = (strstr(node_type,"leaf")==nullptr) ? "true" : "false";
    }

    if (id==nullptr || key_path==nullptr || embedded==nullptr || node_type==nullptr || data_type==nullptr) {
        EV_LOG(TRACE,DSAPI,0,"CPS-META","Can't parse entry - missing data %s",name);
        return;
    }

    const CPS_CLASS_DATA_TYPE_t *_data_type = cps_class_data_type_from_string(data_type);
    const CPS_CLASS_ATTR_TYPES_t *_node_type = cps_class_attr_type_from_string(node_type);

    if(_data_type==nullptr || _node_type==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Can't parse entry - invalid data %s (%s:%s)",name,data_type,node_type);
        return;
    }

    cps_api_attr_id_t _id = (cps_api_attr_id_t) strtoull(id,nullptr,0);

    cps_class_map_node_details details;
    details.desc = desc;
    details.name = name;
    details.embedded = strcasecmp(embedded,"true")==0 ;
    details.attr_type = *_node_type;
    details.data_type = *_data_type;

    //keys
    std::vector<cps_api_attr_id_t> _key_path;
    if (!__process_key(_key_path, key_path,_id, details)) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Can't parse key - invalid data %s %s)",key_path,name);
        return;
    }

    if (cps_class_map_init(_id,&_key_path[0],_key_path.size(),&details)!=cps_api_ret_code_OK) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","CPS Class metadata could not be loaded for %s",name);
    }

}

using _xml_lambda = std::function<void (std_config_node_t,void*)> ;

void _fun(std_config_node_t node, void *user_data) {
    _xml_lambda *_fn = (_xml_lambda*)user_data;
    _fn->operator ()(node,user_data);
}

void __process_enum(std_config_node_t node, void *user_data) {

    const char * _enum_name = std_config_attr_get(node,"name");
    if (_enum_name==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META-LOADER","Enum entry missing critical details \"name\" attribte is missing.");
        return;
    }
    _xml_lambda l = [_enum_name] (std_config_node_t node, void *user_data) -> void {
        const char * _name = std_config_attr_get(node,"name");
        const char * _value = std_config_attr_get(node,"value");
        if (_name==nullptr || _value==nullptr) {
            ///TODO switch to trace later
            EV_LOG(ERR,DSAPI,0,"CPS-META-LOADER","Enum entry missing critical details [name:%s] [value:%s]",
                    _name==nullptr ? "Missing" : "Ok" , _value==nullptr ? "Missing" : "Ok");

            return ;
        }
        const char * _description = std_config_attr_get(node,"description");
        if (_description==nullptr) _description = "";
        cps_class_map_enum_reg(_enum_name,_name,strtol(_value,nullptr,0),_description);
    };
    std_config_for_each_node(node,_fun,&l);
}

void __process_enum_assoc(std_config_node_t node, void *user_data) {
    const char * _name = std_config_attr_get(node,"name");
    const char * _value = std_config_attr_get(node,"enum");

    if(_name==nullptr || _value==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Enum assication is invalid. (name:%s) (value:%mapped_enum)",_name==nullptr?"Missing" : "Ok",
                _value==nullptr ?"Missing":"Ok");
        return;
    }

    cps_api_attr_id_t *id = cps_api_attr_name_to_id(_name);
    if (id==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-META","Enum assication is invalid. Attrib %s is unknown",_name);
        return;
    }
    cps_class_map_enum_associate(*id,_value);
}

void __process_file(std_config_node_t node, void *user_data) {
    const char * _node_name = std_config_name_get(node);

    if (strcmp(_node_name,"metadata_entry")==0) {
        __process_node(node,user_data);
    }
    if (strcmp(_node_name,"class_ownership")==0) {
        __process_ownership(node,user_data);
    }
    if (strcmp(_node_name,"enum_entry")==0) {
        __process_enum(node,user_data);
    }
    if (strcmp(_node_name,"enum_association")==0) {
        __process_enum_assoc(node,user_data);
    }

}

using _prioritized_file_list = std::map<double,std::vector<std::string>>;

}

void __process_single_file(const char * file) {
    std_config_hdl_t h = std_config_load(file);
    if (h==nullptr) {
        EV_LOG(TRACE,DSAPI,0,"CPS-META","Invalid file contents %s",file);
        return ;
    }

    std_config_node_t node = std_config_get_root(h);
    if (node!=nullptr) {
        do {
            const char * _node_name = std_config_name_get(node);
            if (_node_name==nullptr || (strcmp(_node_name,"cps-class-map")!=0)) {
                break;
            }
            std_config_for_each_node(node,__process_file,nullptr);
        } while (0);
    }
    std_config_unload(h);

}

namespace {
bool __cb(const char *name, std_dir_file_TYPE_t type,void *context) {
    if (type != std_dir_file_T_FILE && type!= std_dir_file_T_LINK)  {
        return true;
    }
    auto _path_elements = cps_string::split(name,"/");
    if (_path_elements.size() == 0) return true;

    static constexpr const char * _MATCHING_FILENAME = CPS_DEF_CLASS_XML_SUFFIX;
    static constexpr size_t _MATCHING_FILENAME_LEN = strlen(_MATCHING_FILENAME);

    std::string _filename = _path_elements[_path_elements.size()-1];
    auto _ix = _filename.find(_MATCHING_FILENAME);
    if (_ix==std::string::npos) return true;
    if ((_ix + _MATCHING_FILENAME_LEN)!=_filename.size()) return true;


    _prioritized_file_list *files = (_prioritized_file_list*)context;

    //expected file name is position-name-cpsmetadata.xml"
    //if no position.. then starting is at 0
    auto filename_elements = cps_string::split(_filename,"-");

    //for the actual.. 1.1.1
    double _position = 0.0;
    size_t iteration = 0;

    if (filename_elements.size()>2) {
        const static int VER_POS = 0;
        auto _version_elements = cps_string::split(filename_elements[VER_POS],".");
        for (auto & num : _version_elements) {
            double _elem = (double)strtoll(num.c_str(),nullptr,0);
            if (iteration>0) {
                _elem /= (iteration*10);
            }
            _position+=_elem;
            iteration+=4;//decimal places to shift each time
        }
    }

    (*files)[_position].push_back(name);

    return true;
}
}

void cps_api_metadata_import(void) {
    _prioritized_file_list _files;

    const char *_dir = std_getenv(CPS_API_METADATA_ENV);

    //iterate over any environment directories
    if (_dir!=nullptr) {
        auto list_of_metadata_dirs = cps_string::split(_dir,":");
        for (auto &__dir : list_of_metadata_dirs) {
            std_dir_iterate(__dir.c_str(),__cb,&_files,true);
        }
    }

    ///TODO erg.. in a transition where the software will be moving to SONiC soon - and then this should be /etc/sonic/cpsmetadata
    auto _search_path = cps_string::split(CPS_DEF_META_SEARCH_PATHS,":");
    for (auto &__dir : _search_path ) {
        std_dir_iterate((__dir+ "/cpsmetadata").c_str(),__cb,&_files,true);
    }

    for (_prioritized_file_list::reverse_iterator it = _files.rbegin(), end=_files.rend() ; it!=end;++it  ) {
        for (auto &file : it->second ) {
            __process_single_file(file.c_str());
        }
    }
}
