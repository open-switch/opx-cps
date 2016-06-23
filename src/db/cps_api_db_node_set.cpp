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


#include "cps_api_node_private.h"
#include "cps_api_node_set.h"
#include "cps_api_node.h"
#include "cps_api_db_connection.h"
#include "cps_api_db.h"

#include "std_time_tools.h"
#include "event_log.h"

#include <mutex>

static std::recursive_mutex _mutex;

static cps_api_nodes *_nodes = new cps_api_nodes;
static uint64_t _last_loaded = 0;


static bool load_groups() {
    std::lock_guard<std::recursive_mutex> lg(_mutex);
       if (std_time_is_expired(_last_loaded,5*1000*1000)) {
           _last_loaded = std_get_uptime(nullptr);
           return _nodes->load();
       }
       return false;
}

bool cps_api_db_get_node_group(const std::string &group,std::vector<std::string> &lst) {
    if (group.find(':')!=std::string::npos) {//case where a node is a group (or more that a node is a group)
        lst.push_back(group);
        return true;
    }

    std::lock_guard<std::recursive_mutex> lg(_mutex);
    (void)load_groups();

    cps_api_node_data_type_t type;
    if(!_nodes->get_group_type(group,type)){
        EV_LOGGING(DSAPI,ERR,"GET-NODE-GRUOP","Failed to get group type for %s",group.c_str());
        return cps_api_ret_code_ERR;
    }

    if (!_nodes->group_addresses(group,lst)) {
        const char * __addr = _nodes->addr(group.c_str());
        if (__addr==nullptr) return false;
        lst.push_back(__addr);
    }
    return true;
}

cps_api_return_code_t cps_api_set_master_node(cps_api_node_group_t *group,const char * node_name){
    std::lock_guard<std::recursive_mutex> lg(_mutex);
    (void)load_groups();

    cps_api_node_data_type_t type;
    if(!_nodes->get_group_type(std::string(group->id),type)){
        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to get group type for %s",group->id);
        return cps_api_ret_code_ERR;
    }

    if(type != cps_api_node_data_1_PLUS_1_REDUNDENCY){
        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Setting master for group type %d not supported",type);
        return cps_api_ret_code_ERR;
    }

    auto it = _nodes->_db_node_map.find(group->id);
    if( it == _nodes->_db_node_map.end()){
        EV_LOGGING(DSAPI,ERR,"CPS-DB","No group named %s found",group);
        return cps_api_ret_code_ERR;
    }
    bool found = false;
    std::string master_node;
    for ( auto node_it : it->second){
        if (strncmp(node_it._name.c_str(),node_name,strlen(node_name))==0){
            auto master_it = _nodes->_master.find(group->id);
            if(master_it != _nodes->_master.end()){
                if(master_it->second.compare(node_it._addr)){
                    return cps_api_ret_code_OK;
                }else{
                    cps_db::connection_request b(cps_db::ProcessDBCache(),node_it._addr.c_str());
                    if (!b.valid()) {
                        return cps_api_ret_code_ERR;
                    }

                    if (!cps_db::remove_slave(b.get())) {
                        EV_LOGGING(DSAPI,ERR,"SET-MASTER","Failed to remove node %s as slave",
                                    node_it._name.c_str());
                        return cps_api_ret_code_ERR;
                    }
                }

            }
            _nodes->_master[group->id]=node_it._addr;
            master_node = node_it._addr;
            found = true;
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


bool cps_api_node_set_iterate(const std::string &group_name,const std::function<void (const std::string &node, void*context)> &operation,
        void *context) {
    std::vector<std::string> lst;

    if (!cps_api_db_get_node_group(group_name,lst)) {
        EV_LOG(ERR,DSAPI,0,"CPS-DB-NODES","Failed to load db for group %s",group_name.c_str());
        return false;
    }

    for (auto node_it : lst ) {
        operation(node_it,context);
    }
    return true;
}

