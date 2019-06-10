/*
 * Copyright (c) 2019 Dell Inc.
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


#ifndef CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_
#define CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_

#include "cps_api_node.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <mutex>

///@TODO Refactor this file entirely... simplify caching and decouple db vs master
class cps_api_nodes {
    struct _node_data {
        //the address and the db instance is selected here
        std::vector<std::string> _addrs;
        cps_api_node_data_type_t type;
    };

    struct _db_node_data{
        std::string _addr;
        std::string _name;
    };

    using alias_map_t = std::unordered_map<std::string,std::string> ;
    using group_data_t = std::unordered_map<std::string,_node_data>;
    using group_master_data_t = std::unordered_map<std::string,std::string>;
    using db_node_data_t = std::unordered_map<std::string,std::vector<_db_node_data>>;

    std::recursive_mutex _mutex;
    std::unordered_map<std::string,std::string> _ip_to_name_map;
    std::unordered_set<std::string> _master_set;
    group_data_t _groups;
    size_t _hash=0;

    alias_map_t _alias_map;


    static size_t gen_hash(group_data_t &src);
    bool load_groups();
    bool update_slaves(const char * group);
public:
    db_node_data_t _db_node_map;
    group_master_data_t _master;

    bool is_master_set(std::string group);
    bool group_exists(const std::string &group);
    void mark_master_set(std::string group){ _master_set.insert(group); }
    void remove_master_set(std::string group) { _master_set.erase(group);}
    bool get_port_info(const char *name,_db_node_data *nd);
    const char * addr(const char *addr);
    const char * addr(const std::string &str) { return addr(str.c_str()); }
    bool get_group_type(const std::string & group,cps_api_node_data_type_t &type);
    bool group_addresses(const std::string &group, std::vector<std::string> &addrs);

    bool load();

    bool part_of(const char *group, const char *addr);


    bool ip_to_name(const char *ip, std::string &name);
    std::recursive_mutex &get_lock() {
        return _mutex;
    }
};

bool cps_api_key_is_local_group(const char *node_name);

#endif /* CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_ */
