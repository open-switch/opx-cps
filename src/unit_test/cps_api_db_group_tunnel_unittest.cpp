/*
 * cps_api_db_group_tunnel_unittest.cpp
 *
 *  Created on: Jul 29, 2016
 *      Author: vraiyani
 */

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
#include "cps_class_map.h"
#include "cps_api_db.h"
#include "cps_string_utils.h"

#include "cps_api_object_tools.h"

#include "cps_api_operation_tools.h"
#include "cps_dictionary.h"

#include "cps_class_ut_data.h"
#include "dell-cps.h"

#include "gtest/gtest.h"

#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>



TEST(cps_api_db,db_node_list) {

     cps_api_node_ident ids[3] = { {"NODE1", "127.0.0.1:6379"}, {"NODE2","172.17.0.6:6379"},{"NODE3","10.11.56.36:6379"} };
     cps_api_node_group_t _g;

    _g.id = "A";
    _g.addrs = ids;
    _g.addr_len = 3;
    _g.data_type = cps_api_node_data_NODAL;

    cps_api_set_node_group(&_g);

    cps_api_node_ident new_ids[2] = { {"NODE1", "127.0.0.1:6379"}, {"NODE2","172.17.0.6:6379"} };
    _g.addrs = new_ids;
    _g.addr_len = 2;
    cps_api_set_node_group(&_g);

    new_ids[1] = {"NODE2","11.10.5.6:6379"};
    cps_api_set_node_group(&_g);
    cps_api_delete_node_group(_g.id);


}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}



