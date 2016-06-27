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


#ifndef CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_
#define CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_

#include "cps_api_node.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>


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

    group_data_t _groups;
    size_t _hash;

    alias_map_t _alias_map;
    static size_t gen_hash(group_data_t &src);
    bool load_groups();
    bool load_aliases();
public:
    db_node_data_t _db_node_map;
    group_master_data_t _master;

    bool get_port_info(const char *name,_db_node_data *nd);
    const char * addr(const char *addr);
    const char * addr(const std::string &str) { return addr(str.c_str()); }
    bool get_group_type(const std::string & group,cps_api_node_data_type_t &type);
    bool group_addresses(const std::string &group, std::vector<std::string> &addrs);

    bool load();

    bool part_of(const char *group, const char *addr);
};


#endif /* CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_ */
