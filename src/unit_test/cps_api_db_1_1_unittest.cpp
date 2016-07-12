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
    ASSERT_TRUE(og.get()!=nullptr);

    cps_api_node_ident ids[2] = { {"NODE1", "10.11.56.37:6379"}, {"NODE2","127.0.0.1:6379"} };
    cps_api_node_group_t _g;

    _g.id = "A";
    _g.addrs = ids;
    _g.addr_len = 2;
    _g.data_type = cps_api_node_data_1_PLUS_1_REDUNDENCY;

    ASSERT_EQ(cps_api_set_node_group(&_g),cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_set_master_node("A","NODE1"),cps_api_ret_code_OK);


    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_IFINDEX,1);
    cps_api_object_attr_add(obj,BASE_IP_IPV6_NAME,"Clifford",9);

    cps_api_key_set_group(obj,"A");

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, obj, 0, 200)==cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_delete_node_group("A"),cps_api_ret_code_OK);

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
