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

#include "cps_api_db.h"

#include "std_time_tools.h"

#define CPS_API_NODE_LOCAL_GROUP "localhost"

cps_api_return_code_t cps_db::cps_api_db_init() {

    cps_api_node_ident id={"local",DEFAULT_REDIS_ADDR };
    cps_api_node_group_t _group;
    _group.id = CPS_API_NODE_LOCAL_GROUP;
    _group.addr_len=1;
    _group.addrs=&id;
    _group.data_type = cps_api_node_data_NODAL;

    static const ssize_t MAX_RETRY_ATTEMPT=20;
    static const ssize_t RETRY_DELAY=1000*1000*2;

    for ( size_t ix = 0; ix < MAX_RETRY_ATTEMPT ; ++ix ) {
        if (cps_api_set_node_group(&_group)!=cps_api_ret_code_OK) {
            std_usleep(RETRY_DELAY);
        } else break;
    }

    return cps_api_ret_code_OK;
}
