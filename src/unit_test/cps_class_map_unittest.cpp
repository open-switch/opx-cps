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

/*
 * cps_class_map_unittest.cpp
 *
 *  Created on: Apr 20, 2015
 */


#include "cps_class_map.h"
#include "cps_api_operation.h"
#include "private/cps_class_map_query.h"

#include <gtest/gtest.h>

#include <vector>

#include "cps_class_ut_data.h"



#include <string.h>

TEST(cps_class_map,load) {
    std::vector<cps_api_attr_id_t> ids ;

    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
     for ( auto it : lst) {
         std::cout << "Searching " << it.details.name << std::endl;
         std::cout << "Found matching.. " << cps_attr_id_to_name(it.id) << " --- "  << cps_class_attr_name(&it._ids[0],it._ids.size()) << std::endl;
         char buff[1024];
         cps_api_key_t key;
         memset(&key,0,sizeof(key));
         cps_api_key_init_from_attr_array(&key,&it._ids[0],it._ids.size(),0);
         std::cout << "Key... " << cps_api_key_print(&key,buff,sizeof(buff)) << std::endl;

         ASSERT_TRUE(cps_class_attr_is_embedded(&it._ids[0],it._ids.size()) == it.details.embedded);

         // The raw key starts at offset 0
         ASSERT_TRUE(strcmp(cps_class_string_from_key(&key, 0),it.details.name)==0);


         cps_api_key_from_attr_with_qual(&key,it.id,cps_api_qualifier_TARGET);

         cps_api_key_t _key;
         memset(&_key,0,sizeof(_key));
         cps_api_key_init_from_attr_array(&_key,&it._ids[0],it._ids.size(),0);
         std::cout << "Key... " << cps_api_key_print(&_key,buff,sizeof(buff)) << std::endl;

         cps_api_key_remove_element(&key,0);
         std::cout << "Key... " << cps_api_key_print(&key,buff,sizeof(buff)) << std::endl;
         ASSERT_TRUE(cps_api_key_matches(&_key,&key,true)==0);

     }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

