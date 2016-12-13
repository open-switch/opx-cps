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


#include "cps_api_node_private.h"
#include "cps_api_node_set.h"
#include "cps_api_node.h"
#include "cps_api_db_connection.h"
#include "cps_api_db.h"
#include "cps_class_map.h"
#include "cps_api_object_key.h"
#include "dell-cps.h"

#include "std_time_tools.h"
#include "event_log.h"

#include <mutex>
#include <unordered_map>

static cps_api_nodes *_nodes = new cps_api_nodes;
static uint64_t _last_loaded = 0;
static std::unordered_map<std::string,std::string> _ip_to_node_map;


static bool load_groups() {
    std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());
       if (std_time_is_expired(_last_loaded,5*1000*1000)) {
           _last_loaded = std_get_uptime(nullptr);
           return _nodes->load();
       }
       return false;
}

static bool cps_api_clean_db_instance(const char *group){
    cps_api_transaction_params_t tr;
    if (cps_api_transaction_init(&tr)!=cps_api_ret_code_OK) {
        return false;
    }

    cps_api_transaction_guard tg(&tr);
    cps_api_object_t  db_obj = cps_api_object_create();

    if(db_obj == nullptr ) return false;

    cps_api_key_from_attr_with_qual(cps_api_object_key(db_obj),CPS_DB_INSTANCE_OBJ,
                                    cps_api_qualifier_TARGET);

    cps_api_object_attr_add(db_obj,CPS_DB_INSTANCE_GROUP,group,strlen(group)+1);

    if(cps_api_delete(&tr,db_obj) != cps_api_ret_code_OK ){
        return false;
    }

    if(cps_api_commit(&tr) != cps_api_ret_code_OK ) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB","Failed to delete db instance for group %s",group);
        return false;
    }

    return true;
}


bool cps_api_db_get_group_config(const char * group,  std::unordered_set<std::string> & node_list){
    std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());

    if(_nodes->get_group_info(std::string(group),node_list)){
        return true;
    }
    EV_LOGGING(DSAPI,INFO,"GET-GROUP","Group %s does not exist",group);
    return false;
}


bool cps_api_db_set_group_config(const char * group,  std::unordered_set<std::string> & node_list){
    std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());
    _nodes->add_group_info(std::string(group),node_list);
    return true;
}

bool cps_api_db_get_node_from_ip(const std::string & ip, std::string &name){
    return _nodes->ip_to_name(ip.c_str(),name);
}

bool cps_api_db_del_node_group(const char *group){
     std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());

     cps_api_node_data_type_t type;
     if(!_nodes->get_group_type(std::string(group),type)){
         EV_LOGGING(DSAPI,ERR,"DEL-MASTER","Failed to get group type for %s",group);
         return false;
     }

     if(type != cps_api_node_data_1_PLUS_1_REDUNDENCY){
         return true;
     }

     auto it = _nodes->_db_node_map.find(group);
     if( it == _nodes->_db_node_map.end()){
         EV_LOGGING(DSAPI,ERR,"CPS-DB","No group named %s found",group);
         return false;
     }

     for ( auto node_it : it->second){
         cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
         if (!b.valid()) {
             return false;
         }
         cps_api_object_guard og(cps_api_object_create());

         if (!cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),CPS_DB_INSTANCE_OBJ,
                                                cps_api_qualifier_TARGET)) {
             EV_LOGGING(DSAPI,ERR,"CPS-CLS-MAP","Meta data for cluster set is not loaded.");
             return false;
         }

         cps_api_set_key_data(og.get(),CPS_DB_INSTANCE_GROUP,cps_api_object_ATTR_T_BIN,group,strlen(group)+1);
         cps_api_set_key_data(og.get(),CPS_DB_INSTANCE_NODE_ID,cps_api_object_ATTR_T_BIN,node_it._name.c_str(),
                                 strlen(node_it._name.c_str())+1);

        cps_db::delete_object(b.get(),og.get());
     }

     _nodes->_master.erase(std::string(group));
     _nodes->_db_node_map.erase(std::string(group));
     _nodes->remove_master_set(std::string(group));

     return cps_api_clean_db_instance(group);

}

bool cps_api_db_get_node_group(const std::string &group,std::vector<std::string> &lst) {
    if (group.find(':')!=std::string::npos) {//case where a node is a group (or more that a node is a group)
        lst.push_back(group);
        return true;
    }

    std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());
    (void)load_groups();

    cps_api_node_data_type_t type;
    if(!_nodes->get_group_type(group,type)){
        const char * _alias = _nodes->addr(group);
        if (_alias!=nullptr) {
            lst.push_back(_alias);
            return true;
        }
        EV_LOGGING(DSAPI,ERR,"GET-NODE-GRUOP","Failed to get group type for %s",group.c_str());
        return false;
    }

    if((type == cps_api_node_data_1_PLUS_1_REDUNDENCY) && (_nodes->is_master_set(group))){

        auto it = _nodes->_master.find(group);
        if(it == _nodes->_master.end()){
             EV_LOGGING(DSAPI,ERR,"GET-NODE-GRUOP","Master not set for %s",group.c_str());
             return false;
        }
        lst.push_back(it->second);
        return true;
    }

    if (!_nodes->group_addresses(group,lst)) {
        const char * __addr = _nodes->addr(group.c_str());
        if (__addr==nullptr) return false;
        lst.push_back(__addr);
    }
    return true;
}


static bool cps_api_remove_slave(std::string & addr){
     cps_db::connection_request b(cps_db::ProcessDBCache(),addr.c_str());
     if (!b.valid()) {
         return false;
     }

    if (!cps_db::remove_slave(b.get())) {
        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to remove node %s as slave",
                    addr.c_str());
        return false;
    }

    return true;
}

cps_api_return_code_t cps_api_set_master_node(const char *group,const char * node_name){
    std::lock_guard<std::recursive_mutex> lg(_nodes->get_lock());
    (void)load_groups();

    cps_api_node_data_type_t type;
    if(!_nodes->get_group_type(std::string(group),type)){
        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to get group type for %s",group);
        return cps_api_ret_code_ERR;
    }

    if(type != cps_api_node_data_1_PLUS_1_REDUNDENCY){
        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Setting master for group type %d not supported",type);
        return cps_api_ret_code_ERR;
    }

    auto it = _nodes->_db_node_map.find(group);
    if( it == _nodes->_db_node_map.end()){
        EV_LOGGING(DSAPI,ERR,"CPS-DB","No group named %s found",group);
        return cps_api_ret_code_ERR;
    }
    bool found = false;
    std::string master_node;
    for ( auto node_it : it->second){
        if (strncmp(node_it._name.c_str(),node_name,strlen(node_name))==0){
            auto master_it = _nodes->_master.find(group);
            if(master_it != _nodes->_master.end()){
                if(master_it->second.compare(node_it._addr) == 0){
                    return cps_api_ret_code_OK;
                }else{
                    if(!cps_api_remove_slave(node_it._addr)){
                        return cps_api_ret_code_ERR;
                    }
                }
            }
            _nodes->_master[group]=node_it._addr;
            _nodes->mark_master_set(std::string(group));
            master_node = node_it._addr;
            if(strncmp(node_it._addr.c_str(),"127.0.0.1",strlen("127.0.0.1"))==0){
                EV_LOGGING(DSAPI,DEBUG,"SET-MASTER","Setting local node %s master for group %s",node_name,group);
                return cps_api_remove_slave(node_it._addr) ? cps_api_ret_code_OK : cps_api_ret_code_ERR;
            }
            found = true;
            break;
        }
    }

    if(!found){
        EV_LOGGING(DSAPI,ERR,"CPS-DB","Failed to find a node named %s in group %s",node_name,group);
        return cps_api_ret_code_ERR;
    }

    for ( auto node_it : it->second){
        if (strncmp(node_it._name.c_str(),node_name,strlen(node_name))){
            cps_db::connection_request b(cps_db::ProcessDBCache(),node_it._addr.c_str());
            if (!b.valid()) {
                return cps_api_ret_code_ERR;
            }

            if (!cps_db::make_slave(b.get(),master_node)) {
                EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to make %s slave of %s",
                        node_it._name.c_str(),master_node.c_str());
                return cps_api_ret_code_ERR;
            }
        }
    }

    return cps_api_ret_code_OK;
}

bool cps_api_node_set_iterate(const std::string &group_name,
        const std::function<bool (const std::string &node)> &operation) {
    std::vector<std::string> lst;

    if (!cps_api_db_get_node_group(group_name,lst)) {
        EV_LOG(ERR,DSAPI,0,"CPS-DB-NODES","Failed to load db for group %s",group_name.c_str());
        return false;
    }

    for (auto node_it : lst ) {
        if (!operation(node_it)) break;    //it is perfectly good to terminate the loop early
    }
    return true;
}

