/*
 * cps_api_node_private.h
 *
 *  Created on: May 24, 2016
 *      Author: cwichmann
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
    using _group_data = std::unordered_map<std::string,_node_data>;
    _group_data _groups;
    size_t _hash;

    static size_t gen_hash(_group_data &src);
public:
    bool address_list(const std::string &addr, std::vector<std::string> &addrs);
    bool load();
    bool part_of(const char *group, const char *addr);
};

class cps_api_node_alias {
    std::unordered_map<std::string,std::string> _alias_map;
public:
    const char * addr(const char *addr);
    const char * addr(const std::string &str) { return addr(str.c_str()); }
    bool load();
};


#endif /* CPS_API_INC_PRIVATE_CPS_API_NODE_PRIVATE_H_ */
