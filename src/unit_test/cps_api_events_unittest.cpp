/*
 * Copyright (c) 2019 Dell Inc.
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

/*
 * cps_api_events_unittest.cpp
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
#include "cps_api_object_tools.h"


#include "cps_api_core_utils.h"
#include "cps_api_select_utils.h"

//IP Meta data
#include "cps_class_ut_data.h"

#include "std_time_tools.h"
#include <pthread.h>
#include <sys/select.h>
#include <thread>
#include <mutex>
#include <inttypes.h>
#include <memory>

#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

cps_api_event_service_handle_t handle;

TEST(cps_api_events,seed_meta_data) {
    __init_class_map();
}

TEST(cps_api_events,initialize_event_system) {
    ASSERT_TRUE(cps_api_event_service_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_db::cps_api_db_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);    //testing to ensure that multiple calls to init succeed
}

const static auto &_rep_send_event = [] (size_t range) ->bool {
    size_t ix = 0;
    size_t mx = cps_api_object_list_size(Get1000());
    for ( ; ix < range ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(Get1000(),ix%mx);
        if (!cps_api_core_publish(o)) return false;
    }
    return true;
};

TEST(cps_api_events,simple_publish_events_10000) {
    ASSERT_TRUE(_rep_send_event(10000));
}

TEST(cps_api_events,epoll_trial_20000) {
    std::vector<cps_api_select_handle_t> _handles;
    size_t ix = 0;
    size_t mx = 1000;
    for ( ; ix < mx ; ++ix ) {
        _handles.push_back(cps_api_select_alloc_read());
    }
    for ( auto &it : _handles) {
        cps_api_select_dealloc(it);
    }
}

/*
 * This test is just meant to enable errors but be sure that the system overall continues
 * to run. This is done by enabling the UT flags before exection and then after
 * clearning the UT flag.  As the test runs, it should not crash or be stuck.
 *
 * Additionally, after clearing the flag, the rest of the tests should work properly
 * */

TEST(cps_api_events,test_event_sending_failure_recovery) {

    cps_api_event_service_handle_t handle;

    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t key;
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&key,BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    reg.objects = &key;
    reg.number_of_objects =1;

    ASSERT_TRUE(cps_api_event_client_register(handle,&reg)==cps_api_ret_code_OK) ;

    cps_api_object_guard og(cps_api_object_create());
    //wait for connection event...
    ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
    cps_api_set_library_flags("cps.events.failure-reboot.enable","0");
    cps_api_set_library_flags("cps.unit-test.event-ut.enable","1");

    int cnt=100;
    while(--cnt > 0) {
        og.set(cps_api_object_create());

        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),0,"This is a tag.........",strlen("This is a tag.........")+1);
        for (size_t cnt=0,mx=1000; cnt < mx ; ++cnt ) {
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        }
        printf("Sending...\n");
        cps_api_event_publish(handle,og.get());

        if(cps_api_timedwait_for_event(handle,og.get(),5000)!=cps_api_ret_code_OK) {
            printf("Missing events...\n");
            continue;
        }
        printf("Received...\n");
    }
    cps_api_event_client_disconnect(handle);

    cps_api_set_library_flags("cps.unit-test.event-ut.enable","0");
    cps_api_set_library_flags("cps.events.failure-reboot.enable","1");
}


TEST(cps_api_events,basic_clients_jumbo) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t key;
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&key,BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    reg.objects = &key;
    reg.number_of_objects =1;

    ASSERT_TRUE(cps_api_event_client_register(handle,&reg)==cps_api_ret_code_OK) ;

    cps_api_object_guard og(cps_api_object_create());
    //wait for connection event...
    ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
    sleep(1);
    int cnt=5;
    while(--cnt > 0) {
        og.set(cps_api_object_create());

        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),0,"This is a tag.........",strlen("This is a tag.........")+1);
        for (size_t cnt=0,mx=1000; cnt < mx ; ++cnt ) {
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        }
        printf("Sending...\n");
        cps_api_event_publish(handle,og.get());
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
        printf("Received...\n");
    }
    cps_api_event_client_disconnect(handle);
}

TEST(cps_api_events,basic_clients) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t key;
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&key,BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    reg.objects = &key;
    reg.number_of_objects =1;

    ASSERT_TRUE(cps_api_event_client_register(handle,&reg)==cps_api_ret_code_OK) ;


    int cnt=10;
    while(--cnt > 0) {
        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        for (size_t cnt=0,mx=1; cnt < mx ; ++cnt ) {
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
            cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        }

        printf("Sending...\n");
        cps_api_object_print(og.get());
        cps_api_event_publish(handle,og.get());
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
        printf("Received...\n");
        cps_api_object_print(og.get());
    }
    cps_api_event_client_disconnect(handle);

}

void _cps_api_publish_event(cps_api_object_t obj, std::function<void(cps_api_object_t)> changer, size_t count) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    for ( size_t _ix = 0; _ix < count ; ++_ix ) {
        cps_api_event_publish(handle,obj);
        changer(obj);
    }

    cps_api_event_client_disconnect(handle);
}

void _cps_api_publish_event_time(cps_api_object_t obj, std::function<void(cps_api_object_t)> changer, size_t time) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    size_t _uptime = std_get_uptime(nullptr);

    while ( !std_time_is_expired(_uptime,MILLI_TO_MICRO(time*1000)) ) {
        cps_api_event_publish(handle,obj);
        changer(obj);
    }

    cps_api_event_client_disconnect(handle);
}

cps_api_object_t _create_object() {
    cps_api_object_guard og(cps_api_object_create_clone(cps_api_object_list_get(Get100(),0)));
    cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
    cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
    return og.release();
}

void cps_api_events_perf(size_t secs) {
    std::mutex m1;
    std::mutex m2;

    m1.lock();
    m2.lock();

    std::thread th([&m1,&secs]() {
        cps_api_object_guard og(_create_object());
        m1.lock();
        _cps_api_publish_event_time(og.get(),[](cps_api_object_t o){
            static int _inc_value=0;
            cps_api_object_attr_delete(o,0);
            cps_api_object_attr_add_u64(o,0,++_inc_value);

        },secs);

    });

    std::thread th2([&m2,&secs]() {
        cps_api_object_guard og(_create_object());
        m2.lock();
        _cps_api_publish_event_time(og.get(),[](cps_api_object_t o){
            static int _inc_value=0;
            cps_api_object_attr_delete(o,0);
            cps_api_object_attr_add_u64(o,0,--_inc_value);

        },secs);
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET));
    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;


    cps_api_object_guard og(cps_api_object_create());
    size_t cnt = 0;
    cps_api_event_stats();

    m2.unlock();
    m1.unlock();
    sleep(2);

    size_t _uptime = std_get_uptime(nullptr);

    while ( !std_time_is_expired(_uptime,secs*1000*1000) ) {
        if (cps_api_timedwait_for_event(handle,og.get(),2000)==cps_api_ret_code_OK) ++cnt;
    }
    sleep(1);
    printf("Received %d events\n",(int)cnt);
    cps_api_event_stats();

    th.join();
    th2.join();
    cps_api_event_client_disconnect(handle);
}


TEST(cps_api_events,performance_publish_10k) {
    cps_api_object_guard og(_create_object());
    _cps_api_publish_event_time(og.get(),[](cps_api_object_t){},1);
    cps_api_event_stats();
}

TEST(cps_api_events,performance_5_secs) {
    cps_api_events_perf(5);
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int _rc = RUN_ALL_TESTS();
  UTDataClear();
  return _rc;
}
