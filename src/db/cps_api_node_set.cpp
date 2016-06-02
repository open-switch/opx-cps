

#include "cps_api_node_private.h"
#include "cps_api_node_set.h"

#include "std_time_tools.h"

#include <mutex>

static std::mutex _mutex;
static cps_api_node_alias *_aliases = new cps_api_node_alias;
static cps_api_nodes *_nodes = new cps_api_nodes;


bool cps_api_db_get_node_group(const std::string &group,std::vector<std::string> &lst) {
    std::lock_guard<std::mutex> lg(_mutex);
    _nodes->load();
    if (!_nodes->address_list(group,lst)) return false;
    return true;
}

bool cps_api_node_set_iterate(const std::string &group_name,const std::function<void (const std::string &node, void*context)> &operation,
        void *context) {
    std::vector<std::string> lst;

    if (!cps_api_db_get_node_group(group_name,lst)) return false;

    for (auto node_it : lst ) {
        operation(node_it,context);
    }
    return true;
}

