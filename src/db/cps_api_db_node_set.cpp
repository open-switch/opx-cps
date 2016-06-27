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
    if (!_nodes->group_addresses(group,lst)) {
        const char * __addr = _nodes->addr(group.c_str());
        if (__addr==nullptr) return false;
        lst.push_back(__addr);
    }
    return true;
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

