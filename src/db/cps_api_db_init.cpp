
#include "cps_api_node_private.h"

#include "cps_api_db.h"

#include "std_time_tools.h"

cps_api_return_code_t cps_db::cps_api_db_init() {
    const char *addr = DEFAULT_REDIS_ADDR;
    cps_api_node_group_t _group;
    _group.id = CPS_API_NODE_LOCAL_GROUP;
    _group.addr_len=1;
    _group.addresses=&addr;
    _group.data_type = cps_api_node_data_NODAL;

    static const ssize_t MAX_RETRY_ATTEMPT=20;
    static const ssize_t RETRY_DELAY=1000*1000*2;

    for ( size_t ix = 0; ix < MAX_RETRY_ATTEMPT ; ++ix ) {
        if (cps_api_create_node_group(&_group)!=cps_api_ret_code_OK) {
            std_usleep(RETRY_DELAY);
        } else break;
    }

    return cps_api_ret_code_OK;
}
