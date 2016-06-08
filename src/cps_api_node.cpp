
#include "cps_api_node.h"
#include "cps_api_node_private.h"

#include "cps_api_db.h"

#include "cps_api_object.h"
#include "cps_class_map.h"

#include "dell-cps.h"

#include "event_log.h"

#include <iostream>


cps_api_return_code_t cps_api_delete_node_group(const char *grp) {
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_DETAILS, cps_api_qualifier_TARGET);

    cps_api_object_attr_add(og.get(),CPS_NODE_DETAILS_NAME,grp,strlen(grp)+1);

    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    cps_db::delete_object(b.get(),og.get());
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_create_node_group(cps_api_node_group_t *group) {
    cps_api_object_guard og(cps_api_object_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET);

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_NAME,group->id,strlen(group->id)+1);

    for ( size_t ix = 0; ix < group->addr_len; ++ix ) {
        cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_IP,group->addresses[ix],strlen(group->addresses[ix])+1);
    }

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_TYPE,&group->data_type,sizeof(group->data_type));

    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    if (!b.valid()) {
        return cps_api_ret_code_ERR;
    }
    cps_api_object_guard tmp(cps_api_object_create());

    if (!cps_api_object_clone(tmp.get(),og.get())) {
        return cps_api_ret_code_ERR;
    }

    bool changed = false;
    if (cps_db::get_object(b.get(),tmp.get())) {
        changed = true;
    }

    (void)changed;
    if (!cps_db::store_object(b.get(),og.get())) {
        return cps_api_ret_code_ERR;
    }

    //send out changed...

    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_update_node_group(cps_api_node_group_t *group) {
    return cps_api_create_node_group(group);
}

cps_api_return_code_t cps_api_set_identity(const char *name, const char **alias, size_t len) {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    if (!b.valid()) {
        return cps_api_ret_code_ERR;
    }

    cps_api_object_guard og(cps_api_object_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_DETAILS, cps_api_qualifier_TARGET);
    cps_api_object_attr_add(og.get(),CPS_NODE_DETAILS_ALIAS,name,strlen(name)+1);

    size_t ix = 0;
    for (; ix < len ; ++ix ) {
        cps_api_object_attr_add(og.get(),CPS_NODE_DETAILS_NAME,alias[ix],strlen(alias[ix])+1);
        if (!cps_db::store_object(b.get(),og.get())) {
            EV_LOG(ERR,DSAPI,0,"SET-IDENT","Failed to update identity setting for local node");
        }
        cps_api_object_attr_delete(og.get(),CPS_NODE_DETAILS_NAME);
    }
    return cps_api_ret_code_OK;
}

bool cps_api_nodes::address_list(const std::string &addr, std::vector<std::string> &addrs) {
    auto it = _groups.find(addr);
    if (it==_groups.end()) return false;

    addrs = it->second._addrs;
    return true;
}

size_t cps_api_nodes::gen_hash(_group_data &src) {
    size_t rc = 0;
    for ( auto group_elem : src ) {
        rc = std::hash<std::string>()(group_elem.first) ^ ( rc << 8 );
        for ( auto node_elem : group_elem.second._addrs ) {
            rc = std::hash<std::string>()(group_elem.first) ^ ( rc << 8 );
        }
    }
    return rc;
}

bool cps_api_nodes::load() {
    cps_api_object_guard og(cps_api_object_create());

    if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET)) {
        EV_LOG(ERR,DSAPI,0,"CPS-CLS-MAP","Meta data for cluster set is not loaded.");
        return false;
    }
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

    if (!b.valid()) {
        return false;
    }

    _group_data cpy;

    cps_api_object_list_guard lg(cps_api_object_list_create());
    if (!cps_db::get_objects(b.get(),og.get(),lg.get())) return false;

    for (size_t ix = 0,mx = cps_api_object_list_size(lg.get()); ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);

        const char *name = (const char*) cps_api_object_get_data(o,CPS_NODE_GROUP_NAME);
        _node_data nd;

        cps_api_node_data_type_t *_type = (cps_api_node_data_type_t*)cps_api_object_get_data(o,CPS_NODE_GROUP_TYPE);
        if (_type!=nullptr) nd.type = *_type;

        static const int MAX_EXPECTED_ADDRESSES=40;
        char *addrs[MAX_EXPECTED_ADDRESSES];
        size_t len = cps_api_object_get_list_data(o,CPS_NODE_GROUP_IP,(void**)addrs,MAX_EXPECTED_ADDRESSES);

        for (size_t _ix = 0; _ix < len ; ++_ix ) {
            nd._addrs.push_back(addrs[_ix]);
        }
        cpy[name] = nd;
    }

    size_t _new_hash = cps_api_nodes::gen_hash(_groups);

    if (_new_hash==_hash) return false;  ///TODO is this unique enough?

    std::swap(_groups,cpy);
    _hash = _new_hash;
    return true;
}

bool cps_api_nodes::part_of(const char *group, const char *addr) {
    auto it = _groups.find(group);
    if (it==_groups.end()) return false;
    auto ait = std::find(it->second._addrs.begin(),it->second._addrs.end(),addr);
    if (ait==it->second._addrs.end()) return false;
    return true;
}

bool cps_api_key_set_group(cps_api_object_t obj,const char *group) {
    cps_api_attr_id_t ids[] = {CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP};
    return cps_api_object_e_add(obj,ids,2,cps_api_object_ATTR_T_BIN,group,strlen(group)+1);
}

const char * cps_api_key_get_group(cps_api_object_t obj) {
    cps_api_attr_id_t ids[] = {CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_GROUP};
    const char * p = (const char*)cps_api_object_e_get_data(obj,ids,2);
    if (p==nullptr) return DEFAULT_REDIS_ADDR;
    return p;
}

const char * cps_api_key_get_node(cps_api_object_t obj) {
    cps_api_attr_id_t ids[] = {CPS_OBJECT_GROUP,CPS_OBJECT_GROUP_NODE};
    return (const char*)cps_api_object_e_get_data(obj,ids,2);
}

bool cps_api_node_alias::load() {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    if (!b.valid()) {
        return false;
    }

    std::unordered_map<std::string,std::string> _cpy;
    std::swap(_cpy,_alias_map);

    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_DETAILS, cps_api_qualifier_TARGET);

    cps_api_object_list_guard lg(cps_api_object_list_create());
    if (!cps_db::get_objects(b.get(),og.get(),lg.get())) return false;

    bool changed = false;
    for ( size_t ix = 0, mx = cps_api_object_list_size(lg.get()); ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
        const char *name = (const char*) cps_api_object_get_data(o,CPS_NODE_DETAILS_NAME);
        const char *alias = (const char*) cps_api_object_get_data(o,CPS_NODE_DETAILS_ALIAS);

        _alias_map[name] = alias;

        auto it = _cpy.find(name);
        if (it!=_cpy.end()) {
            if (it->second == _alias_map[name])
                continue;
        }

        changed = true;
    }
    return changed;
}

const char * cps_api_node_alias::addr(const char *addr) {
    auto it = _alias_map.find(addr);
    if (it==_alias_map.end()) return addr;
    return it->second.c_str();
}

