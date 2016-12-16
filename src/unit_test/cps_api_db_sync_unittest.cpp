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

bool test_sync_cb(cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_response_t *res)
{
    std::cout << "Operation: " << params->opcode << std::endl;
    std::cout << "Source Node: " << params->src_node << std::endl;
    std::cout << "Dest Node: " << params->dest_node << std::endl;

    if(params->opcode == cps_api_oper_SET) {
        std::cout << "Source Object: " << std::endl;
        cps_api_object_print(params->object_src);
        std::cout << "Dest Object: " << std::endl;
        cps_api_object_print(params->object_dest);
    }
    else if(params->opcode == cps_api_oper_CREATE) {
        std::cout << "Source Object: " << std::endl;
        cps_api_object_print(params->object_src);
    }
    else if(params->opcode == cps_api_oper_DELETE) {
        std::cout << "Dest Object: " << std::endl;
        cps_api_object_print(params->object_dest);
    }
    return true;
}

bool test_sync_error_cb(cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_error_t *err)
{
    std::cout << "Error Code: " << err->err_code << std::endl;
    return true;
}

TEST(cps_api_db_sync, objsync) {
   __init_class_map();

   cps_api_node_ident ids[2] = { {"NODE1", "127.0.0.1:6379"}, {"NODE2","10.11.63.118:6379"} };
   cps_api_node_group_t _g;

   _g.id = "TestGroup";
   _g.addrs = ids;
   _g.addr_len = 2;
   _g.data_type = cps_api_node_data_NODAL;

   cps_api_set_node_group(&_g);

   cps_api_object_guard src_og(cps_api_object_create());
   cps_api_key_from_attr_with_qual(cps_api_object_key(src_og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
   cps_api_key_set_node( src_og.get(), "NODE2");

   cps_api_object_guard dst_og(cps_api_object_create());
   cps_api_key_from_attr_with_qual(cps_api_object_key(dst_og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
   cps_api_key_set_node( dst_og.get(), "NODE1");

   cps_api_return_code_t ret = cps_api_sync(dst_og.get(), src_og.get(), test_sync_cb , test_sync_error_cb);
   std::cout << ret << std::endl;

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



