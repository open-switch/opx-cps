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

#include "cps_api_node.h"
#include "cps_api_operation_tools.h"
#include "cps_api_object.h"
#include "cps_class_map.h"
#include "cps_string_utils.h"
#include "cps_api_db_interface.h"
#include "cps_api_object_tools.h"
#include "cps_api_db_operations.h"
#include "cps_class_ut_data.h"
#include "dell-cps.h"
#include "gtest/gtest.h"

void fill_db(int start) {
    __init_class_map();
    static int count = 0;

    cps_api_object_list_guard _3(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    size_t mx = start+3;
    for ( size_t ix = start; ix < mx ; ++ix ) {
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);

        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,ix);

        std::string _elem = cps_string::sprintf("%s-%d","Cliff",((int)ix+count));
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
        cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_elem.c_str(),_elem.size());


        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,ix);
        cps_api_object_t o = cps_api_object_list_create_obj_and_append(_3.get());
        cps_api_object_clone(o,og.get());
    }

    ++count;
    cps_api_db_commit_bulk_t bulk;
    bulk.objects = _3.get();
    bulk.node_group = nullptr;
    bulk.publish = false;
    bulk.op = cps_api_oper_CREATE;
    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);

}

TEST(cps_api_db_sync, set3_objs_from3) {
    fill_db(3);
}

TEST(cps_api_db_sync, set3_objs_from5) {
    fill_db(5);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


