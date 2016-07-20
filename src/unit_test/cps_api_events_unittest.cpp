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
 * cps_api_events_unittest.cpp
 */


#include "cps_api_events.h"
#include "private/db/cps_api_db.h"

#include <stdio.h>
#include <stdlib.h>


#include "gtest/gtest.h"


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
#include <inttypes.h>

cps_api_event_service_handle_t handle;

bool _cps_api_event_thread_callback(cps_api_object_t object,void * context) {
     char buff[1024];
     static int cnt=0;
     printf("1(%d)- Obj %s\n",cnt,cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     return true;
}

bool _cps_api_event_thread_callback_2(cps_api_object_t object,void * context) {
     char buff[1024];
     static int cnt=0;
     printf("2(%d) - Obj %s\n",cnt,cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     return true;
}


bool threaded_client_test() {

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t keys[5];
    cps_api_key_init(&keys[0],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_INTERFACE,1,0);

    cps_api_key_init(&keys[1],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_ROUTE,1,0);

    cps_api_key_init(&keys[2],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_ROUTE,1,1,1);

    cps_api_key_init(&keys[3],cps_api_qualifier_TARGET,
            cps_api_obj_cat_ROUTE,1,0);

    cps_api_key_init(&keys[4],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,0);

    reg.objects = keys;
    reg.number_of_objects = 5 ; //sizeof(keys)/sizeof(*keys);

    if (cps_api_event_thread_reg(&reg,
            _cps_api_event_thread_callback,NULL)!=cps_api_ret_code_OK)
        return false;

    cps_api_key_set_len(keys,1);
    reg.number_of_objects = 1;
    if (cps_api_event_thread_reg(&reg,
            _cps_api_event_thread_callback_2,NULL)!=cps_api_ret_code_OK)
        return false;
    return true;
}

bool push_running = true;
void * push_client_messages(void *) {

    size_t ix = 0;
    size_t mx = 10000;

    cps_api_event_service_handle_t handle;
    if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) return false;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_attr_add(obj,1,"Cliff",6);
    cps_api_qualifier_t q[2] ={ cps_api_qualifier_TARGET, cps_api_qualifier_OBSERVED};
    cps_api_object_category_types_t c[3]={cps_api_obj_cat_INTERFACE,
            cps_api_obj_cat_ROUTE,cps_api_obj_cat_QOS
    };

    for ( ; ix < mx ; ++ix ) {
        if ((ix %10)==0) {
            if (cps_api_event_client_disconnect(handle)!=cps_api_ret_code_OK) return false;
            if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) return false;
        }
        cps_api_key_init(cps_api_object_key(obj),q[ix%2],c[ix%3],1,2,ix%3,ix);
        if (cps_api_event_publish(handle,obj)!=cps_api_ret_code_OK) exit(1);
    }
    push_running = false;
    return NULL;
}

bool simple_client_use() {
    cps_api_event_service_handle_t handle;
    if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) return false;

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t keys[5];
    cps_api_key_init(&keys[0],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_INTERFACE,1,0);

    cps_api_key_init(&keys[1],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_ROUTE,1,0);

    cps_api_key_init(&keys[2],cps_api_qualifier_OBSERVED,
            cps_api_obj_cat_ROUTE,1,1,1);

    cps_api_key_init(&keys[3],cps_api_qualifier_TARGET,
            cps_api_obj_cat_ROUTE,1,0);

    cps_api_key_init(&keys[4],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,0);


    reg.objects = keys;
    reg.number_of_objects =5; //sizeof(keys)/sizeof(*keys);

    if (cps_api_event_client_register(handle,&reg)!=cps_api_ret_code_OK) return false;

    cps_api_object_t rec = cps_api_object_create();
    pthread_t id;
    pthread_create(&id,NULL,push_client_messages,NULL);
    char buff[1024];
    int cnt=0;
    while(true) {
        if (cps_api_wait_for_event(handle,rec)!=cps_api_ret_code_OK) return false;
        printf("3(%d) -  %s\n",cnt,cps_api_object_to_string(rec,buff,sizeof(buff)));
        ++cnt;
    }
    return true;
}

bool _cps_api_event_term(cps_api_object_t object,void * context) {
     char buff[1024];
     static int cnt=0;
     printf("%d(%d) - Obj %s\n",__LINE__,cnt,cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     if (cnt==(10000-1)) exit(0);
     return true;
}

bool full_reg() {

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t key[2];

    cps_api_key_init(&key[0],cps_api_qualifier_OBSERVED,0,0,0);
    cps_api_key_init(&key[1],cps_api_qualifier_TARGET,0,0,0);

    reg.objects = key;
    reg.number_of_objects = 2 ;

    if (cps_api_event_thread_reg(&reg, _cps_api_event_term,NULL)!=cps_api_ret_code_OK)
        return false;

    if (cps_api_event_thread_reg(&reg,_cps_api_event_term,NULL)!=cps_api_ret_code_OK)
        return false;

    return true;
}

bool test_init() {
    static std_event_server_handle_t _handle=NULL;
    std_event_server_init(&_handle,CPS_API_EVENT_CHANNEL_NAME,CPS_API_EVENT_THREADS );
    return (cps_api_event_service_init()==cps_api_ret_code_OK);
}

TEST(cps_api_events,seed_meta_data) {
    std::vector<cps_api_attr_id_t> ids ;

    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
}

TEST(cps_api_events,init) {
    ASSERT_TRUE(test_init());
    ASSERT_TRUE(cps_db::cps_api_db_init()==cps_api_ret_code_OK);
}

TEST(cps_api_events,basic_clients) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t keys;
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&keys,BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    reg.objects = &keys;
    reg.number_of_objects =1;

    ASSERT_TRUE(cps_api_event_client_register(handle,&reg)==cps_api_ret_code_OK) ;

    char buff[1024];
    int cnt=10;
    while(--cnt > 0) {
        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);

        cps_api_event_publish(handle,og.get());
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
        printf("3(%d) -  %s\n",cnt,cps_api_object_to_string(og.get(),buff,sizeof(buff)));
    }
    cps_api_event_client_disconnect(handle);
}

TEST(cps_api_events,performance_10000) {
    size_t count = 10000;
    std::mutex m1;
    std::mutex m2;


    m1.lock();
    m2.lock();

    std::thread th([&m1,count]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m1.lock();

        size_t ix = 0;
        size_t mx = count;
        for ( ; ix < mx ; ++ix ) {
            cps_api_event_publish(handle,og.get());

        }
    });

    std::thread th2([&m2,count]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m2.lock();
        size_t ix = 0;
        size_t mx = count;
        for ( ; ix < mx ; ++ix ) {
            cps_api_event_publish(handle,og.get());

        }
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;


    m2.unlock();
    m1.unlock();
    size_t _total = count * 2;
    cps_api_object_guard og(cps_api_object_create());
    while (_total-- > 0) {
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);
    }


    th.join();
    th2.join();
}

TEST(cps_api_events,filtered_events) {

    std::mutex m1;

    m1.lock();

    std::thread th([&m1]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m1.lock();

        size_t ix = 0;
        time_t now = time(nullptr);
        for ( ; (time(nullptr) - now) < (10) ; ++ix ) {
            cps_api_object_attr_delete(og.get(),0);
            cps_api_object_attr_add_u64(og.get(),0,ix);
            cps_api_object_attr_delete(og.get(),1);
            cps_api_object_attr_add_u64(og.get(),1,ix%2);
            cps_api_event_publish(handle,og.get());
        }
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
    cps_api_object_attr_add_u64(obj,1,1);
    cps_api_event_object_exact_match(obj,true);

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;

    m1.unlock();

    cps_api_object_guard og(cps_api_object_create());
    time_t now = time(nullptr);
    size_t cnt = 0;

    for ( ; (time(nullptr) - now) < (10) ; ++cnt) {
        if (cps_api_timedwait_for_event(handle,og.get(),5000)!=cps_api_ret_code_OK) break;
        uint64_t *matched = (uint64_t*)cps_api_object_get_data(og.get(),1);
        if (matched!=nullptr) {
            ASSERT_EQ(*matched,1);
        }

    }
    printf("Received %d events\n",(int)cnt);

    th.join();
}


TEST(cps_api_events,performance_1min) {

    std::mutex m1;

    m1.lock();

    std::thread th([&m1]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m1.lock();

        size_t ix = 0;
        time_t now = time(nullptr);
        for ( ; (time(nullptr) - now) < (60) ; ++ix ) {
            cps_api_object_attr_delete(og.get(),0);
            cps_api_object_attr_add_u64(og.get(),0,ix);
            cps_api_event_publish(handle,og.get());
        }
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;

    m1.unlock();

    cps_api_object_guard og(cps_api_object_create());
    time_t now = time(nullptr);
    size_t cnt = 0;
    size_t _prev = 0;
    for ( ; (time(nullptr) - now) < (60) ; ++cnt) {
        if (cps_api_timedwait_for_event(handle,og.get(),5000)!=cps_api_ret_code_OK) break;
        uint64_t *cnt = (uint64_t *)cps_api_object_get_data(og.get(),0);
        if (cnt!=nullptr) {
            if (_prev!=(*cnt-1)) {
                printf("Sequence issue...%" PRIu64 " instead of %" PRIu64 "\n",*cnt,_prev);
            }
            _prev = *cnt;
        }
        if (cnt==nullptr) {
            printf("No sequence number\n");
        }
    }
    printf("Received %d events\n",(int)cnt);

    th.join();
}


TEST(cps_api_events,performance_1min_2_senders) {
    std::mutex m1;
    std::mutex m2;

    m1.lock();
    m2.lock();

    std::thread th([&m1]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m1.lock();

        size_t ix = 0;
        time_t now = time(nullptr);
        for ( ; (time(nullptr) - now) < (60) ; ++ix ) {
            cps_api_object_attr_delete(og.get(),0);
            cps_api_object_attr_add_u64(og.get(),0,ix);
            cps_api_event_publish(handle,og.get());
        }
    });

    std::thread th2([&m2]() {
        cps_api_event_service_handle_t handle;
        ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

        cps_api_object_guard og(cps_api_object_create());
        ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);
        m2.lock();
        time_t now = time(nullptr);
        for ( ; (time(nullptr) - now) < (60) ; ) {
            ASSERT_TRUE(cps_api_event_publish(handle,og.get())==cps_api_ret_code_OK);
        }
    });

    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_object_list_guard event_reg(cps_api_object_list_create());
    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(event_reg.get());
    ASSERT_TRUE(obj!=nullptr);

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET));

    ASSERT_TRUE(cps_api_event_client_register_object(handle,event_reg.get())==cps_api_ret_code_OK) ;


    m2.unlock();
    m1.unlock();

    cps_api_object_guard og(cps_api_object_create());
    time_t now = time(nullptr);
    size_t cnt = 0;
    for ( ; (time(nullptr) - now) < (60) ; ++cnt) {
        if (cps_api_timedwait_for_event(handle,og.get(),5000)!=cps_api_ret_code_OK) break;

        usleep(100000);
    }
    printf("Received %d events\n",(int)cnt);

    th.join();
    th2.join();
}


TEST(cps_api_events,full_test) {
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(full_reg());
    ASSERT_TRUE(threaded_client_test());
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
