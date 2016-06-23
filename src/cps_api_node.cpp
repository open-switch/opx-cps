
#include "cps_api_node.h"
#include "cps_api_node_private.h"

#include "cps_api_db.h"

#include "cps_api_object.h"
#include "cps_class_map.h"
#include "cps_api_object_key.h"
#include "cps_api_node_set.h"
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


static bool cps_api_find_local_node(cps_api_node_group_t *group,size_t &node_ix){
    for(size_t ix = 0 ; ix < group->addr_len ; ++ix){
        if(strncmp(group->addrs->addr,"127.0.0.1",strlen("127.0.0.1"))==0){
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
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

    if(local_node_ix >= group->addr_len){
        return cps_api_ret_code_ERR;
    }

    cps_api_transaction_params_t tr;
    if (cps_api_transaction_init(&tr)!=cps_api_ret_code_OK) {
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

    cps_api_transaction_guard tg(&tr);
    cps_api_object_t  db_obj = cps_api_object_create();

    if(db_obj == nullptr ) return cps_api_ret_code_ERR;

    cps_api_key_from_attr_with_qual(cps_api_object_key(db_obj),CPS_DB_INSTANCE_OBJ,
                                        cps_api_qualifier_TARGET);

    cps_api_object_attr_add(db_obj,CPS_DB_INSTANCE_GROUP,group->id,strlen(group->id)+1);

    if(cps_api_create(&tr,db_obj) != cps_api_ret_code_OK ){
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

    if(cps_api_commit(&tr) != cps_api_ret_code_OK ) {
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

    cps_api_object_t ret_obj = cps_api_object_list_get(tr.change_list,0);
    if (ret_obj == nullptr ){
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

    // Get the port where new db instance was started
    const char * db_port = (const char*) cps_api_object_get_data(ret_obj,CPS_DB_INSTANCE_PORT);
    std::string _db_port = db_port;

    if(cps_api_transaction_close(&tr) != cps_api_ret_code_OK ){
        /*@TODO add an error log */
        return cps_api_ret_code_ERR;
    }

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

    std::string _failed_nodes = "";
    bool result = false;

    if (!cps_api_node_set_iterate(group->id,[&db_node,&result,&_failed_nodes](const std::string &name,void *c){
            cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());

                result |= cps_db::store_object(r.get(),db_node.get());
                if (!result) _failed_nodes+=name+",";
                return;

        },nullptr)) {
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_set_node_group(cps_api_node_group_t *group) {

    cps_api_object_guard og(cps_api_object_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_NODE_GROUP, cps_api_qualifier_TARGET);

    cps_api_object_attr_add(og.get(),CPS_NODE_GROUP_NAME,group->id,strlen(group->id)+1);

    for ( size_t ix = 0; ix < group->addr_len; ++ix ) {
        cps_api_attr_id_t _ip[]={CPS_NODE_GROUP_NODE,ix,CPS_NODE_GROUP_NODE_IP};
        cps_api_object_e_add(og.get(),_ip,sizeof(_ip)/sizeof(*_ip),cps_api_object_ATTR_T_BIN,
                group->addrs[ix].addr,strlen(group->addrs[ix].addr)+1);
        cps_api_attr_id_t _alias[]={CPS_NODE_GROUP_NODE,ix,CPS_NODE_GROUP_NODE_NAME};
        cps_api_object_e_add(og.get(),_alias,sizeof(_alias)/sizeof(*_alias),cps_api_object_ATTR_T_BIN,
                group->addrs[ix].node_name,strlen(group->addrs[ix].node_name)+1);
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
    if(group->data_type == cps_api_node_data_1_PLUS_1_REDUNDENCY){
        if(!cps_api_create_global_instance(group)){
            return cps_api_ret_code_ERR;
        }
    }

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
        rc = std::hash<std::string>()(group_elem.first) ^ ( rc << 8 );
        for ( auto node_elem : group_elem.second._addrs ) {
            rc = std::hash<std::string>()(group_elem.first) ^ ( rc << 8 );
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
    }
    return true;
}

bool cps_api_nodes::add_db_node(const char *group,const char *ip,_db_node_data & db_node){
    std::string old_ip = ip;
    std::size_t ip_pos = old_ip.find(':');
    old_ip = old_ip.substr(0,ip_pos);
    db_node._addr = old_ip + db_node._addr;
    auto it = _db_node_map.find(group);
    if( it == _db_node_map.end()){
        std::vector<_db_node_data> v;
        v.push_back(std::move(db_node));
        _db_node_map[group] = std::move(v);
    }else{
        _db_node_map[group].push_back(std::move(db_node));
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

    for (size_t ix = 0,mx = cps_api_object_list_size(lg.get()); ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);

        const char *name = (const char*) cps_api_object_get_data(o,CPS_NODE_GROUP_NAME);
        _node_data nd;
        _db_node_data db_node;

        cps_api_node_data_type_t *_type = (cps_api_node_data_type_t*)cps_api_object_get_data(o,CPS_NODE_GROUP_TYPE);
        if (_type==nullptr){
            EV_LOGGING(DSAPI,ERR,"NODE-LOAD","Could not get the group type for group %s",name);
            return false;
        }
        nd.type = *_type;
        cps_api_object_attr_t _node_group = cps_api_object_attr_get(o,CPS_NODE_GROUP_NODE);
        if (_node_group==nullptr) continue; ///TODO log

        cps_api_object_it_t it;
        cps_api_object_it_from_attr(_node_group,&it);
        cps_api_object_it_inside(&it);

        while (cps_api_object_it_valid(&it)) {
            cps_api_attr_id_t id = cps_api_object_attr_id(it.attr);
            (void)id;
            cps_api_object_it_t elem = it;

            cps_api_object_it_inside(&elem);

            if (!cps_api_object_it_valid(&elem)) continue;
            cps_api_object_attr_t _ip =cps_api_object_it_find(&elem,CPS_NODE_GROUP_NODE_IP);
            cps_api_object_attr_t _name =cps_api_object_it_find(&elem,CPS_NODE_GROUP_NODE_NAME);
            if (_ip==nullptr || _name==nullptr) continue;
            const char *__ip = (const char*)cps_api_object_attr_data_bin(_ip);
            const char *__name =(const char*)cps_api_object_attr_data_bin(_name);

            if(nd.type == cps_api_node_data_1_PLUS_1_REDUNDENCY){
                db_node._name = __name;
                if(!get_port_info(name,&db_node)){
                    EV_LOGGING(DSAPI,ERR,"CPS-DB","Failed to get port info for group %s and node %s",
                       name,__name);
                    return false;
                }

                add_db_node(name,__ip,db_node);
            }
            _alias_map[__name] = __ip;

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

bool cps_api_key_set_group(cps_api_object_t obj,const char *group) {
    return cps_api_object_attr_add(obj,CPS_OBJECT_GROUP_GROUP,group,strlen(group)+1);
}

const char * cps_api_key_get_group(cps_api_object_t obj) {
    const char *p = (const char*) cps_api_object_get_data(obj,CPS_OBJECT_GROUP_GROUP);
    if (p==nullptr) return DEFAULT_REDIS_ADDR;
    return p;
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
