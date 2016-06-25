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



#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "std_event_service.h"
#include "cps_api_service.h"
#include "cps_class_map.h"
#include "cps_api_node.h"

//IP Meta data
#include "cps_class_ut_data.h"

#include <pthread.h>
#include <sys/select.h>
#include <thread>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"


static size_t cnt=0;
static bool _first_time = false;

bool _cps_api_event_term2(cps_api_object_t object,void * context) {
     char buff[1024];
     _first_time=true;
     //uint32_t* data = (uint32_t*)cps_api_object_get_data(object,1);
     printf("2 ---- %d(%d) - Obj %s\n",0,(int)cnt,cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     return true;
}

bool _cps_api_event_term1(cps_api_object_t object,void * context) {
    char buff[1024];
    _first_time=true;
    //uint32_t* data = (uint32_t*)cps_api_object_get_data(object,1);
    printf("1 ---- %d(%d) - Obj %s\n",0,(int)cnt,cps_api_object_to_string(object,buff,sizeof(buff)));
    ++cnt;
    return true;
}

TEST(cps_api_events,init) {
    ASSERT_TRUE(cps_db::cps_api_db_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_event_service_init()==cps_api_ret_code_OK);
}

TEST(cps_api_events,seed_meta_data) {
    std::vector<cps_api_attr_id_t> ids ;
    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
}

void fire_event(cps_api_attr_id_t id, size_t cnt) {
    cps_api_object_guard og(cps_api_object_create());
    ASSERT_TRUE(og.valid());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),id,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(og.get(),1,cnt);
    cps_api_event_thread_publish(og.get());
}

TEST(cps_api_events,event_thread) {

    cps_api_attr_id_t ids[] = {BASE_IP_IPV6, BASE_IP_IPV4};
    cps_api_object_list_guard lg(cps_api_object_list_create());
    size_t ix = 0;
    size_t mx = sizeof(ids)/sizeof(*ids);

    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_create_obj_and_append(lg.get());
        ASSERT_TRUE(o!=nullptr);
        cps_api_key_set_len(cps_api_object_key(o),1);
        cps_api_key_set(cps_api_object_key(o),0,ix);
        cps_api_key_from_attr_with_qual(cps_api_object_key(o),ids[ix],cps_api_qualifier_TARGET);
    }

    cps_api_event_thread_reg_object(lg.get(),_cps_api_event_term1,nullptr);
    cps_api_event_thread_reg_object(lg.get(),_cps_api_event_term2,nullptr);

    sleep(1);

    static const size_t CALLBACKS = 2;

    size_t _mx =1;
    for ( ; ix < mx ; ++ix ) {
        cps_api_attr_id_t ids[] = {BASE_IP_IPV4, BASE_IP_IPV6};

        size_t _ix = 0;
        size_t _mx = sizeof(ids)/sizeof(*ids);
        for ( ; _ix < _mx ; ++_ix ) {
            cps_api_object_guard og(cps_api_object_create());
            ASSERT_TRUE(og.valid());
            cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),ids[_ix],cps_api_qualifier_TARGET);
            cps_api_object_attr_add_u32(og.get(),1,ix);cps_api_key_set_group(og.get(),"a");
            cps_api_event_thread_publish(og.get());
        }
    }
    time_t now = time(NULL);
    while (cnt < (mx*CALLBACKS)) {
        sleep(1);
        if ((time(NULL) - now)> 30) break;
    }
    ASSERT_EQ(cnt,(mx*_mx*CALLBACKS));

}


TEST(cps_api_events,event_waiting) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    cps_api_key_set_group(obj,"A");
    cps_api_key_set_len(cps_api_object_key(obj),1);
    cps_api_key_set(cps_api_object_key(obj),0,2);

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;

    event_reg.set(cps_api_object_list_create());

    cps_api_object_guard og(cps_api_object_create());
    obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    cps_api_key_set_group(obj,"A");
    cps_api_key_set_len(cps_api_object_key(obj),1);
    cps_api_key_set(cps_api_object_key(obj),0,1);

    while (true) {
        char buff[1024];
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
        printf("1 ---- %d(%d) - Obj %s\n",0,(int)cnt,cps_api_object_to_string(og.get(),buff,sizeof(buff)));
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
