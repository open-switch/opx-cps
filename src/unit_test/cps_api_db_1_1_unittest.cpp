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

#include "cps_api_events.h"


#include <stdio.h>
#include <stdlib.h>


#include "gtest/gtest.h"

#include "cps_api_node.h"
#include "cps_api_node_private.h"

#include "cps_api_db.h"
#include "cps_string_utils.h"

#include "cps_class_map.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "std_event_service.h"

#include "cps_api_service.h"

#include "cps_api_operation_tools.h"
#include "cps_dictionary.h"

#include <pthread.h>
#include <sys/select.h>

#include <vector>
#include "cps_class_ut_data.h"


TEST(cps_api_db,db_node_list) {

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    cps_api_object_list_t list =cps_api_object_list_create();
    cps_api_object_list_guard lg(list);

    cps_api_node_ident ids[2] = { {"NODE1", "127.0.0.1:6379"}, {"NODE2","10.11.63.197:6379"} };
    cps_api_node_group_t _g;

    _g.id = "A";
    _g.addrs = ids;
    _g.addr_len = 2;
    _g.data_type = cps_api_node_data_1_PLUS_1_REDUNDENCY;

    cps_api_set_node_group(&_g);
    cps_api_set_master_node("A","NODE1");

    cps_api_nodes nodes;
    ASSERT_TRUE(nodes.load());

    ASSERT_TRUE(nodes.part_of("A","127.0.0.1:6379"));
    ASSERT_TRUE(nodes.part_of("A","172.17.0.6:6380"));
    ASSERT_TRUE(!nodes.part_of("A","372.17.0.6:6379"));
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
