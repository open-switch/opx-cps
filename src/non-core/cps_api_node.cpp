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

#include "cps_api_node.h"
#include "cps_api_node_private.h"
#include "cps_api_node_set.h"
#include "cps_api_db.h"
#include "cps_api_db_connection.h"
#include "cps_api_object.h"
#include "cps_api_object_tools.h"
#include "cps_api_operation_tools.h"
#include "cps_class_map.h"
#include "cps_api_object_key.h"
#include "cps_api_node_set.h"
#include "cps_string_utils.h"
#include "cps_api_vector_utils.h"
#include "dell-cps.h"
#include "std_utils.h"
#include "event_log.h"

#include <mutex>
#include <string>
#include <cstring>
#include <unordered_map>
#include <iostream>

namespace cps_db {
/**
 * A simple hash function template that will work for various int types
 */
template <typename T>
struct vector_hash {
    std::size_t operator() (const std::vector<T> &c) const {
        std::size_t rc;
        for ( auto it : c ) {
            size_t _cur = std::hash<size_t>(it);
            rc =  rc ^ (_cur << 3);
        }
        return rc;
    }
};

}

constexpr static const char * __get_local_ip() {
    return "127.0.0.1";
}
constexpr static size_t __get_local_ip_len() {
    return strlen(__get_local_ip());
}

#define MAX_IP_LEN 64


static cps_api_return_code_t cps_api_del_node_tunnel(const char * group, const char * node) {
    cps_api_transaction_params_t trans;
    cps_api_key_t keys;

    if (cps_api_transaction_init(&trans) != cps_api_ret_code_OK)
        return cps_api_ret_code_ERR;

    cps_api_object_t p_trans_obj = cps_api_object_create();
    cps_api_transaction_guard tgd(&trans);

    cps_api_key_from_attr_with_qual(&keys, CPS_TUNNEL_OBJ, cps_api_qualifier_TARGET);
    cps_api_object_set_type_operation(&keys, cps_api_oper_DELETE);
    cps_api_object_set_key(p_trans_obj, &keys);

    // Add attributes
    cps_api_object_attr_add(p_trans_obj, CPS_TUNNEL_GROUP, group,strlen(group)+1);
    cps_api_object_attr_add(p_trans_obj, CPS_TUNNEL_NODE_ID, node,strlen(node)+1);

    cps_api_object_list_append(trans.change_list, p_trans_obj);
    if (cps_api_commit(&trans) != cps_api_ret_code_OK) // Add logs
        return cps_api_ret_code_ERR;

    return cps_api_ret_code_OK;
}


cps_api_return_code_t cps_api_delete_node_group(const char *grp) {
    cps_api_db_del_node_group(grp);

    std::unordered_set<std::string>  node_list;
    if(!cps_api_db_get_group_config(grp,node_list)){
        EV_LOGGING(DSAPI,ERR,"DELETE-GLOBAL","Failed to get group information %s",grp);
        return cps_api_ret_code_OK;
    }

    for ( auto it : node_list ){
        cps_api_del_node_tunnel(grp, it.c_str());
    }

    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET);

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_NAME,grp,strlen(grp)+1);

    cps_db::connection_request b(cps_db::ProcessDBEvents(),DEFAULT_REDIS_ADDR);
    if (!b.valid()) return cps_api_ret_code_ERR;
    bool rc = false;
    if ((rc=cps_db::delete_object(b.get(),og.get()))) {
        cps_api_object_set_type_operation(cps_api_object_key(og.get()),cps_api_oper_DELETE);
        rc = cps_db::publish(b.get(),og.get());
    }
    if (!rc) {
        EV_LOGGING(DSAPI,ERR,"CPS-NODE-GROUP","Delete or publish failed for %s due to db connection issue",grp);
    }

    return rc ? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}


static bool cps_api_find_local_node(cps_api_node_group_t *group,size_t &node_ix){
    const size_t _len = __get_local_ip_len();
    const char * _name = __get_local_ip();
    for(size_t ix = 0 ; ix < group->addr_len ; ++ix){
        if(strncmp(group->addrs[ix].addr,_name,_len)==0){
            node_ix = ix;
            return true;
        }
    }
    return false;
}


cps_api_return_code_t cps_api_create_global_instance(cps_api_node_group_t *group){
    if(group == nullptr) return cps_api_ret_code_ERR;

    size_t local_node_ix;
    if(!cps_api_find_local_node(group,local_node_ix)){
        ///TODO this is a little stange - should see about removing this check - it is not likely to be needed but
        /// it would be better not to maintain "local" ip addresses
        EV_LOGGING(DSAPI,ERR,"SET-GLOBAL","Failed to find local node in group %s",group);
        return cps_api_ret_code_ERR;
    }

    if(local_node_ix >= group->addr_len){
        return cps_api_ret_code_ERR;
    }

    cps_api_object_guard og(cps_api_object_create());
    if(og.get()== nullptr ) return cps_api_ret_code_ERR;

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_DB_INSTANCE_OBJ,
                                        cps_api_qualifier_TARGET);
    cps_api_object_attr_add(og.get(),CPS_DB_INSTANCE_GROUP,group->id,strlen(group->id)+1);
    if (cps_api_commit_one(cps_api_oper_CREATE,og.get(),4,200)!=cps_api_ret_code_OK) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-TUN","Failed to create tunnel for group %s",group);
        return cps_api_ret_code_ERR;
    }

    cps_api_object_t ret_obj = og.get();

    // Get the port where new db instance was started
    const char * db_port = (const char*) cps_api_object_get_data(ret_obj,CPS_DB_INSTANCE_PORT);
    std::string _db_port = db_port;

    /*
     * Push the group name local node id and its port to local/remote DB.
     */
    cps_api_node_ident & node = group->addrs[local_node_ix];
    cps_api_object_guard db_node(cps_api_object_create());
    if (db_node.get() == nullptr){
        return cps_api_ret_code_ERR;
    }

    cps_api_key_from_attr_with_qual(cps_api_object_key(db_node.get()),CPS_DB_INSTANCE_OBJ, cps_api_qualifier_TARGET);

    cps_api_set_key_data(db_node.get(),CPS_DB_INSTANCE_GROUP,cps_api_object_ATTR_T_BIN,group->id,strlen(group->id)+1);
    cps_api_set_key_data(db_node.get(),CPS_DB_INSTANCE_NODE_ID,cps_api_object_ATTR_T_BIN,node.node_name,strlen(node.node_name)+1);
    cps_api_object_attr_add(db_node.get(),CPS_DB_INSTANCE_PORT,_db_port.c_str(),strlen(_db_port.c_str())+1);

///TODO not sure we want a global db created in this way - one node seems to own the provisioning of all nodes
/// just not sure maybe we should require each node to do its own global db config
    if (!cps_api_node_set_iterate(group->id,[&db_node](const std::string &name) -> bool {
            cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
            if (!r.valid()) return true;
            if (!cps_db::store_object(r.get(),db_node.get())) {
                EV_LOGGING(DSAPI,ERR,"DB-GROUP-UPATE","Failed to update node %s data",name.c_str());
            }
            return true;
        })) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}


static bool cps_api_get_tunnel_port(cps_api_node_group_t *group, size_t ix, char *tunnel_port, size_t len) {
    cps_api_transaction_params_t trans;
    cps_api_key_t keys;

    if (cps_api_transaction_init(&trans) != cps_api_ret_code_OK)
        return false;

    cps_api_object_t p_trans_obj = cps_api_object_create();
    cps_api_transaction_guard tgd(&trans);

    cps_api_key_from_attr_with_qual(&keys, CPS_TUNNEL_OBJ, cps_api_qualifier_TARGET);
    cps_api_object_set_type_operation(&keys, cps_api_oper_CREATE);
    cps_api_object_set_key(p_trans_obj, &keys);

    // Add attributes
    cps_api_object_attr_add(p_trans_obj, CPS_TUNNEL_GROUP, group->id,strlen(group->id)+1);
    cps_api_object_attr_add(p_trans_obj, CPS_TUNNEL_NODE_ID, group->addrs[ix].node_name,strlen(group->addrs[ix].node_name)+1);
    cps_api_object_attr_add(p_trans_obj, CPS_TUNNEL_IP,  group->addrs[ix].addr,strlen(group->addrs[ix].addr)+1);

    cps_api_object_list_append(trans.change_list, p_trans_obj);
    if (cps_api_commit(&trans) != cps_api_ret_code_OK)
        return false;

    cps_api_object_t p_ret_obj = cps_api_object_list_get(trans.change_list,0);
    if (p_ret_obj==nullptr)
        return false;

    const char *port = (const char *)cps_api_object_get_data(p_ret_obj, CPS_TUNNEL_PORT);
    strncpy(tunnel_port,port,len-1);

    return true;
}

static bool cps_api_set_compare_group(cps_api_node_group_t *new_grp,  std::unordered_set<std::string> & node_list){

    std::unordered_set<std::string>del_nodes;
    for(auto it : node_list){
        bool exist = false;
        for(unsigned int new_grp_ix = 0 ; new_grp_ix < new_grp->addr_len ; ++ new_grp_ix){
            if(strncmp(it.c_str(),new_grp->addrs[new_grp_ix].node_name,
                    strlen(it.c_str()))==0){
                exist = true;
                break;
            }
        }
        if(!exist){
            del_nodes.insert(it);
        }
    }

    for(auto node_it : del_nodes){
        cps_api_del_node_tunnel(new_grp->id,node_it.c_str());
    }

    return true;
}

cps_api_return_code_t cps_api_set_node_group(cps_api_node_group_t *group) {

    cps_api_object_guard og(cps_api_object_create());

    std::unordered_set<std::string>  node_list;

    bool created = cps_api_db_get_group_config(group->id,node_list);

    if(created){
        cps_api_set_compare_group(group,node_list);
    }

    for(unsigned int ix = 0 ; ix < group->addr_len ; ++ix){
        node_list.insert(std::string(group->addrs[ix].node_name));
    }

    cps_api_db_set_group_config(group->id,node_list);

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET);

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_NAME,group->id,strlen(group->id)+1);

    for ( size_t ix = 0; ix < group->addr_len; ++ix ) {
        cps_api_attr_id_t _ip[]={CPS_NODE_GROUP_NODE,ix,CPS_NODE_GROUP_NODE_IP};
        cps_api_object_e_add(og.get(),_ip,sizeof(_ip)/sizeof(*_ip),cps_api_object_ATTR_T_BIN,
                group->addrs[ix].addr,strlen(group->addrs[ix].addr)+1);
        cps_api_attr_id_t _alias[]={CPS_NODE_GROUP_NODE,ix,CPS_NODE_GROUP_NODE_NAME};
        cps_api_object_e_add(og.get(),_alias,sizeof(_alias)/sizeof(*_alias),cps_api_object_ATTR_T_BIN,
                group->addrs[ix].node_name,strlen(group->addrs[ix].node_name)+1);

        cps_api_attr_id_t _tunnel_ip[]={CPS_NODE_GROUP_NODE,ix,CPS_NODE_GROUP_NODE_TUNNEL_IP};
        if (strstr(group->addrs[ix].addr, __get_local_ip()) )
            cps_api_object_e_add(og.get(),_tunnel_ip,sizeof(_tunnel_ip)/sizeof(*_tunnel_ip),
                                 cps_api_object_ATTR_T_BIN,
                                 group->addrs[ix].addr,strlen(group->addrs[ix].addr)+1);
        else {
           // Do a transaction on cps/tunnel object and get the stunnel port
           char tunnel_port[MAX_IP_LEN], tunnel_addr[MAX_IP_LEN];

           if(! cps_api_get_tunnel_port(group, ix, tunnel_port, sizeof(tunnel_port)))
               return cps_api_ret_code_ERR;

           // Stunnel IP address will be "loopback:tunnel_port"
           strncpy(tunnel_addr,"127.0.0.1:", MAX_IP_LEN-1);
           strncat(tunnel_addr, tunnel_port, MAX_IP_LEN-1);
           cps_api_object_e_add(og.get(),_tunnel_ip,sizeof(_tunnel_ip)/sizeof(*_tunnel_ip),
                            cps_api_object_ATTR_T_BIN, tunnel_addr, strlen(tunnel_addr)+1);
        }
    }

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_TYPE,&group->data_type,sizeof(group->data_type));

    cps_db::connection_request b(cps_db::ProcessDBEvents(),DEFAULT_REDIS_ADDR);
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

    if(group->data_type == cps_api_node_data_1_PLUS_1_REDUNDENCY){
        return cps_api_create_global_instance(group);
    }

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),created? cps_api_oper_CREATE : cps_api_oper_SET);
    cps_db::publish(b.get(),og.get());

    return cps_api_ret_code_OK;
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

bool cps_api_nodes::group_addresses(const std::string &addr, std::vector<std::string> &addrs) {
    auto it = _groups.find(addr);
    if (it==_groups.end()) return false;

    addrs = it->second._addrs;

    return true;
}

size_t cps_api_nodes::gen_hash(group_data_t &src) {
    size_t rc = 0;
    for ( auto group_elem : src ) {
        rc = std::hash<std::string>()(group_elem.first) + ( rc );
        for ( auto node_elem : group_elem.second._addrs ) {
            rc = std::hash<std::string>()(node_elem) + ( rc  );
        }
    }
    return rc;
}

bool cps_api_nodes::get_port_info(const char *group, _db_node_data *nd){
    cps_api_object_guard og(cps_api_object_create());

    if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_DB_INSTANCE_OBJ,
                                        cps_api_qualifier_TARGET)) {
        EV_LOGGING(DSAPI,ERR,"CPS-CLS-MAP","Meta data for cluster set is not loaded.");
        return false;
    }

    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

    if (!b.valid()) {
        return false;
    }

    cps_api_set_key_data(og.get(),CPS_DB_INSTANCE_GROUP,cps_api_object_ATTR_T_BIN,group,strlen(group)+1);
    cps_api_set_key_data(og.get(),CPS_DB_INSTANCE_NODE_ID,cps_api_object_ATTR_T_BIN,nd->_name.c_str(),strlen(nd->_name.c_str())+1);

    if (cps_db::get_object(b.get(),og.get())) {

        const char *port = (const char*) cps_api_object_get_data(og.get(),CPS_DB_INSTANCE_PORT);
        if(port == nullptr){
            return false;
        }
        nd->_addr = port;
        return true;
    }
    return false;
}

bool cps_api_nodes::update_slaves(const char *group) {

    // Check if master exist, if not then return as master is not set
    auto master_it = _master.find(group);
    if( master_it == _master.end()){
        return true;
    }


    auto it = _db_node_map.find(group);
    if( it == _db_node_map.end()){
        return false;
    }

    std::string master_node = master_it->second;

    // If master is local node, no need to update
    if(strncmp(master_node.c_str(),"127.0.0.1",strlen("127.0.0.1"))==0){
        return true;
    }

    // Iterate through the other nodes of the group and make them slave of master
    for ( auto node_it : it->second){
        if (strncmp(node_it._addr.c_str(),master_node.c_str(),strlen(master_node.c_str()))){
            cps_db::connection_request b(cps_db::ProcessDBCache(),node_it._addr.c_str());
            if (!b.valid()) {
                return false;
            }

            if (!cps_db::make_slave(b.get(),master_node)) {
                EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to make %s slave of %s",
                        node_it._name.c_str(),master_node.c_str());
                return false;
            }
        }
    }
    return true;
}

bool cps_api_nodes::load_groups() {
    cps_api_object_guard og(cps_api_object_create());

    if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET)) {
        EV_LOG(ERR,DSAPI,0,"CPS-CLS-MAP","Meta data for cluster set is not loaded.");
        return false;
    }
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

    if (!b.valid()) {
        return false;
    }

    group_data_t cpy;

    cps_api_object_list_guard lg(cps_api_object_list_create());
    if (!cps_db::get_objects(b.get(),og.get(),lg.get())) return false;

    _ip_to_name_map.clear();

    for (size_t ix = 0,mx = cps_api_object_list_size(lg.get()); ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
        std::vector<_db_node_data> v;
        const char *name = (const char*) cps_api_object_get_data(o,CPS_NODE_GROUP_NAME);
        _node_data nd;

        cps_api_node_data_type_t *_type = (cps_api_node_data_type_t*)cps_api_object_get_data(o,CPS_NODE_GROUP_TYPE);
        if (_type==nullptr){
            EV_LOGGING(DSAPI,ERR,"NODE-LOAD","Could not get the group type for group %s",name);
            return false;
        }
        nd.type = *_type;
        cps_api_object_attr_t _node_group = cps_api_object_attr_get(o,CPS_NODE_GROUP_NODE);
        if (_node_group==nullptr) {
            cpy[name] = nd;
            EV_LOGGING(DSAPI, DEBUG, "NODE-LOAD", "Empty node list for group %s", name);
            continue;
        }

        cps_api_object_it_t it;
        cps_api_object_it_from_attr(_node_group,&it);
        cps_api_object_it_inside(&it);

        while (cps_api_object_it_valid(&it)) {
            cps_api_attr_id_t id = cps_api_object_attr_id(it.attr);
            (void)id;
            cps_api_object_it_t elem = it;

            cps_api_object_it_inside(&elem);

            if (!cps_api_object_it_valid(&elem)) continue;

            _db_node_data db_node;
            cps_api_object_attr_t _ip =cps_api_object_it_find(&elem,CPS_NODE_GROUP_NODE_TUNNEL_IP);
            cps_api_object_attr_t _name =cps_api_object_it_find(&elem,CPS_NODE_GROUP_NODE_NAME);

            if (_ip==nullptr || _name==nullptr) continue;
            const char *__ip = (const char*)cps_api_object_attr_data_bin(_ip);
            const char *__name =(const char*)cps_api_object_attr_data_bin(_name);

            //setup reverse mapping
            /*@TODO need to revist if there is same node with different name but same ip in
             * different group
             */
            _ip_to_name_map[__ip] = __name;
            _alias_map[__name] = __ip;

            if(nd.type == cps_api_node_data_1_PLUS_1_REDUNDENCY){
                db_node._name = __name;
                if(get_port_info(name,&db_node)){
                    auto lst = cps_string::split(std::string(__ip),":");
                    if (lst.size()!=2) {
                        return false;
                    }
                    db_node._addr = lst[0] + ':'+ db_node._addr;
                    v.push_back(std::move(db_node));
                }
                else{
                    EV_LOGGING(DSAPI,ERR,"CPS-DB","Failed to get port info for group %s and node %s",
                                       name,__name);
                }
            }

            if(v.size()>0){
                _db_node_map[name] = v;
                update_slaves(name);
            }

            const char * _alias = this->addr(__ip);
            if (_alias!=nullptr) __ip = _alias;
            nd._addrs.push_back(__ip);
            cps_api_object_it_next(&it);
        }
        cpy[name] = nd;
    }

    size_t _new_hash = cps_api_nodes::gen_hash(cpy);

    if (_new_hash==_hash) return false;  ///TODO is this unique enough?

    std::swap(_groups,cpy);
    _hash = _new_hash;
    return true;

}

bool cps_api_nodes::ip_to_name(const char *ip, std::string &name) {
    std::lock_guard<std::recursive_mutex> lg(_mutex);
    auto it = _ip_to_name_map.find(ip);
    if(it == _ip_to_name_map.end()){
        return false;
    }

    name = it->second;
    return true;
}

bool cps_api_nodes::load_aliases() {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    if (!b.valid()) {
        return false;
    }

    alias_map_t _cpy;

    cps_api_object_guard og(cps_api_object_create());
    if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_DETAILS, cps_api_qualifier_TARGET)) {
        return false;
    }

    cps_api_object_list_guard lg(cps_api_object_list_create());
    if (!cps_db::get_objects(b.get(),og.get(),lg.get())) return false;

    for ( size_t ix = 0, mx = cps_api_object_list_size(lg.get()); ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
        const char *name = (const char*) cps_api_object_get_data(o,CPS_NODE_DETAILS_NAME);
        const char *alias = (const char*) cps_api_object_get_data(o,CPS_NODE_DETAILS_ALIAS);

        _cpy[name] = alias;
    }
    std::swap(_cpy,_alias_map);

    return true;
}

bool cps_api_nodes::load() {
    load_aliases();
    return load_groups();
}

bool cps_api_nodes::part_of(const char *group, const char *addr) {
    auto it = _groups.find(group);
    if (it==_groups.end()) return false;
    auto ait = std::find(it->second._addrs.begin(),it->second._addrs.end(),addr);
    return (ait!=it->second._addrs.end());
}

const char * cps_api_nodes::addr(const char *addr) {
    const char *_ret = nullptr;

    do {
        auto it = _alias_map.find(addr);
        if (it==_alias_map.end()) break;
        _ret = it->second.c_str();
        addr = _ret;
        continue;
    } while (0);
    return _ret;
}

void cps_api_key_del_node_attrs(cps_api_object_t obj) {
    while (cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_GROUP)) ;
    while (cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_NODE)) ;
}

bool cps_api_key_set_group(cps_api_object_t obj,const char *group) {
	cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_GROUP);
    return cps_api_object_attr_add(obj,CPS_OBJECT_GROUP_GROUP,group,strlen(group)+1);
}

bool cps_api_key_is_local_group(const char *node_name) {
    return strcmp(DEFAULT_REDIS_ADDR,node_name)==0;
}

const char * cps_api_key_get_group(cps_api_object_t obj) {
    const char *p = (const char*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_GROUP);
    if (p==nullptr) return DEFAULT_REDIS_ADDR;
    return p;
}

bool cps_api_key_set_node(cps_api_object_t obj, const char *node) {
	cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_NODE);
    return cps_api_object_attr_add(obj,CPS_OBJECT_GROUP_NODE,node,strlen(node)+1);
}

const char * cps_api_key_get_node(cps_api_object_t obj) {
    return (const char*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_NODE);
}

bool cps_api_nodes::get_group_type(const std::string &group,cps_api_node_data_type_t &type){
    auto it = _groups.find(group);
    if(it != _groups.end()){
        type = it->second.type;
        return true;
    }
    return false;
}

bool cps_api_nodes::is_master_set(std::string group){
    auto it = _master_set.find(group);
    if(it != _master_set.end()){
        return true;
    }
    return false;
}

bool cps_api_nodes::add_group_info(std::string group,std::unordered_set<std::string> & node_list){
    _group_node_map[group] = std::move(node_list);
    return true;
}

bool cps_api_nodes::get_group_info(std::string group,std::unordered_set<std::string> & node_list){
    auto it = _group_node_map.find(group);
    if(it != _group_node_map.end()){
        node_list = it->second;
        return true;
    }

    return false;
}
