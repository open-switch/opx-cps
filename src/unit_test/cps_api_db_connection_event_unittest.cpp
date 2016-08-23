
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
#include "private/db/cps_api_db.h"

#include <stdio.h>
#include <stdlib.h>


#include "gtest/gtest.h"

#include "dell-cps.h"
#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "std_event_service.h"
#include "cps_api_service.h"
#include "cps_class_map.h"
#include "cps_api_node.h"
#include "cps_api_object_tools.h"

//IP Meta data
#include "cps_class_ut_data.h"

#include <pthread.h>
#include <sys/select.h>
#include <thread>
#include <mutex>
#include <inttypes.h>
#include <memory>



bool connection_event_test() {

        cps_api_event_service_handle_t handle=NULL;

        if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) {
            return false;
        }

        cps_api_object_list_guard lg(cps_api_object_list_create());
        cps_api_object_guard obj(cps_api_object_create());
        cps_api_key_from_attr_with_qual(cps_api_object_key(obj.get()), CPS_CONNECTION_ENTRY,
                                               cps_api_qualifier_TARGET);
        cps_api_object_attr_add(obj.get(),CPS_OBJECT_GROUP_GROUP, "A",strlen("A")+1);

        if (!cps_api_object_list_append(lg.get(),obj.get())) {
              return false;
        }

        if(cps_api_event_client_register_object(handle,lg.get()) != cps_api_ret_code_OK){
            return false;
        }

        cps_api_object_t rec = cps_api_object_create();

        while(true) {
               if (cps_api_wait_for_event(handle,rec)!=cps_api_ret_code_OK) {
                   cps_api_object_delete(rec);
                   return false;
               }
               cps_api_object_attr_t ip = cps_api_object_attr_get(rec,CPS_CONNECTION_ENTRY_IP);
               cps_api_object_attr_t name = cps_api_object_attr_get(rec,CPS_CONNECTION_ENTRY_NAME);
               cps_api_object_attr_t state = cps_api_object_attr_get(rec,CPS_CONNECTION_ENTRY_CONNECTION_STATE);
               cps_api_object_attr_t group = cps_api_object_attr_get(rec,CPS_CONNECTION_ENTRY_GROUP);

               const char *_ip, *_name, *_group;

               bool _state;
               if(group){
                   _group = (const char *)cps_api_object_attr_data_bin(group);
                   printf(" Group %s\n",_group);
                                           }
               if(ip){
                   _ip =  (const char *) cps_api_object_attr_data_bin(ip);
                   printf(" IP %s\n",_ip);
               }
               if(name){
                   _name = (const char *) cps_api_object_attr_data_bin(name);
                   printf(" Name %s\n",_name);
               }
               if(state){
                   _state = cps_api_object_attr_data_u32(state);
                   printf(" State %d\n",_state);
               }

        }
        cps_api_object_delete(rec);
        return true;
}


TEST(cps_api_events,initialize_event_system) {
    ASSERT_TRUE(cps_api_event_service_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_db::cps_api_db_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);    //testing to ensure that multiple calls to init succeed
}



TEST(cps_api_events,full_test) {
   ASSERT_TRUE(connection_event_test());

}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}





