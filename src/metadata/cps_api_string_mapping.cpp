
#include "cps_class_map_query.h"
#include "cps_class_map.h"

#include <unordered_map>
#include <algorithm>

using cps_class_map_qual_to_string = std::unordered_map<int,std::string>;


static const std::unordered_map<std::string,CPS_CLASS_ATTR_TYPES_t> _attr_types = {
        {"leaf",CPS_CLASS_ATTR_T_LEAF},
        {"leaf-list",CPS_CLASS_ATTR_T_LEAF_LIST },
        {"container",CPS_CLASS_ATTR_T_CONTAINER },
        {"subsystem",CPS_CLASS_ATTR_T_SUBSYSTEM },
        {"list",CPS_CLASS_ATTR_T_LIST },
};

static const std::unordered_map<std::string,CPS_CLASS_DATA_TYPE_t> _data_types = {
        {"uint8_t",CPS_CLASS_DATA_TYPE_T_UINT8},
        {"uint16_t",CPS_CLASS_DATA_TYPE_T_UINT16},
        {"uint32_t",CPS_CLASS_DATA_TYPE_T_UINT32},
        {"uint64_t",CPS_CLASS_DATA_TYPE_T_UINT64},
        {"int8_t",CPS_CLASS_DATA_TYPE_T_INT8},
        {"int16_t",CPS_CLASS_DATA_TYPE_T_INT16},
        {"int32_t",CPS_CLASS_DATA_TYPE_T_INT32},
        {"int64_t",CPS_CLASS_DATA_TYPE_T_INT64},
        {"string",CPS_CLASS_DATA_TYPE_T_STRING},
        {"enum",CPS_CLASS_DATA_TYPE_T_ENUM},
        {"bool",CPS_CLASS_DATA_TYPE_T_BOOL},
        {"obj-id",CPS_CLASS_DATA_TYPE_T_OBJ_ID},
        {"date",CPS_CLASS_DATA_TYPE_T_DATE},
        {"ipv4",CPS_CLASS_DATA_TYPE_T_IPV4},
        {"ipv6",CPS_CLASS_DATA_TYPE_T_IPV6},
        {"ip",CPS_CLASS_DATA_TYPE_T_IP},
        {"bin",CPS_CLASS_DATA_TYPE_T_BIN},
        {"double",CPS_CLASS_DATA_TYPE_T_DOUBLE},
        {"embedded",CPS_CLASS_DATA_TYPE_T_EMBEDDED},
        {"key",CPS_CLASS_DATA_TYPE_T_KEY},
};

static std::unordered_map<std::string,CPS_API_OBJECT_OWNER_TYPE_t> _owner_type_to_string = {
        { "service", CPS_API_OBJECT_SERVICE },
        { "service-cache",CPS_API_OBJECT_SERVICE_CACHE },
        { "db",CPS_API_OBJECT_DB }
};

static const cps_class_map_qual_to_string _qual_to_string = {
        {cps_api_qualifier_TARGET, "target"},
        {cps_api_qualifier_OBSERVED, "observed"},
        {cps_api_qualifier_PROPOSED, "proposed"},
        {cps_api_qualifier_REALTIME, "realtime"},
        {cps_api_qualifier_REGISTRATION, "registration"}
};

static std::unordered_map<std::string,cps_api_operation_types_t> _op_types = {
        { "delete",cps_api_oper_DELETE},
        { "create",cps_api_oper_CREATE},
        { "set",cps_api_oper_SET},
        { "action",cps_api_oper_ACTION}
};

static std::unordered_map<std::string,cps_api_node_data_type_t> _node_types = {
        { "nodal",cps_api_node_data_NODAL},
        { "1+1",cps_api_node_data_1_PLUS_1_REDUNDENCY},
};

const cps_api_node_data_type_t* cps_node_type_from_string(const char *str) {
    auto it = _node_types.find(str);
    if (it!=_node_types.end()) return &it->second;
    return nullptr;
}

const cps_api_operation_types_t* cps_operation_type_from_string(const char *str) {
    auto it = _op_types.find(str);
    if (it!=_op_types.end()) return &it->second;
    return nullptr;
}

const CPS_API_OBJECT_OWNER_TYPE_t *cps_class_owner_type_from_string(const char *str) {
    auto it = _owner_type_to_string.find(str);
    if (it!=_owner_type_to_string.end()) return &it->second;
    return nullptr;
}

const CPS_CLASS_DATA_TYPE_t *cps_class_data_type_from_string(const char *str) {
    auto it = _data_types.find(str);
    if (it!=_data_types.end()) return &it->second;
    return nullptr;
}

const CPS_CLASS_ATTR_TYPES_t *cps_class_attr_type_from_string(const char *str) {
    auto it = _attr_types.find(str);
    if (it!=_attr_types.end()) return &it->second;
    return nullptr;
}

struct _attr_type_match {
    uint32_t _val;
    template <typename T> bool operator()(T &it) const {
        return (uint32_t)it->second == _val;
    }
};

const cps_api_qualifier_t *cps_class_qual_from_string(const char *str) {
    auto it = std::find_if(_qual_to_string.begin(),_qual_to_string.end(),[str](const cps_class_map_qual_to_string::value_type &it) {
        return strcmp(it.second.c_str(),str)==0;
    });
    if (it!=_qual_to_string.end()) return (const cps_api_qualifier_t *)&it->first;
    return nullptr;
}

const char * cps_operation_type_to_string(cps_api_operation_types_t data) {
    auto it = std::find_if(_op_types.begin(),_op_types.end(),[data]
            (const std::unordered_map<std::string,cps_api_operation_types_t>::value_type &it){
                return (it.second == data);
            });
    if (it != _op_types.end()) return it->first.c_str();
    return nullptr;
}

const char * cps_class_owner_type_to_string(CPS_API_OBJECT_OWNER_TYPE_t data) {
    auto it = std::find_if(_owner_type_to_string.begin(),_owner_type_to_string.end(),[data]
            (const std::unordered_map<std::string,CPS_API_OBJECT_OWNER_TYPE_t>::value_type &it){
                return (it.second == data);
            });
    if (it != _owner_type_to_string.end()) return it->first.c_str();
    return nullptr;
}

const char * cps_class_attr_type_to_string(CPS_CLASS_ATTR_TYPES_t data) {
    auto it = std::find_if(_attr_types.begin(),_attr_types.end(),[data]
            (const std::unordered_map<std::string,CPS_CLASS_ATTR_TYPES_t>::value_type &it){
                return (it.second == data);
            });
    if (it != _attr_types.end()) return it->first.c_str();
    return nullptr;
}

const char * cps_class_data_type_to_string(CPS_CLASS_DATA_TYPE_t data) {
    auto it = std::find_if(_data_types.begin(),_data_types.end(),[data]
            (const std::unordered_map<std::string,CPS_CLASS_DATA_TYPE_t>::value_type &it){
                return (it.second == data);
            });
    if (it != _data_types.end()) return it->first.c_str();
    return nullptr;
}

const char * cps_class_qual_to_string(cps_api_qualifier_t qual) {
    auto it = _qual_to_string.find((cps_api_qualifier_t)qual);
    if (it == _qual_to_string.end()) return nullptr;
    return it->second.c_str();
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

