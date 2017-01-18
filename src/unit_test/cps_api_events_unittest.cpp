/*
 * Copyright (c) 2016 Dell Inc.
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
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_IP,"10.10.10.10",12);
        cps_api_object_attr_add( og.get(),BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,"24",3);

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



#if 0
//################################################################
TEST(cps_api_events,performance_30_secs) {
	cps_api_events_perf(30);
}


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

    cps_api_key_t keys[4];
    cps_api_key_from_attr_with_qual(&keys[0],BASE_IP_IPV6,cps_api_qualifier_OBSERVED);
    cps_api_key_from_attr_with_qual(&keys[1],BASE_IP_IPV4,cps_api_qualifier_OBSERVED);
    cps_api_key_from_attr_with_qual(&keys[2],BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_key_from_attr_with_qual(&keys[3],BASE_IP_IPV4,cps_api_qualifier_TARGET);


    reg.objects = keys;
    reg.number_of_objects = 4 ; //sizeof(keys)/sizeof(*keys);

    if (cps_api_event_thread_reg(&reg,_cps_api_event_thread_callback,NULL)!=cps_api_ret_code_OK)
        return false;

    cps_api_key_set_len(&keys[0],1);
    reg.number_of_objects = 1;
    if (cps_api_event_thread_reg(&reg, _cps_api_event_thread_callback_2,NULL)!=cps_api_ret_code_OK)
        return false;
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

struct _event_reg_handler {
    cps_api_event_reg_t _reg;
    std::unique_ptr<cps_api_key_t[]> _keys;
};

std::unique_ptr<_event_reg_handler> _setup_key_reg(cps_api_attr_id_t *attr, cps_api_qualifier_t*quals,size_t len, bool use_defaults) {
    auto reg = std::unique_ptr<_event_reg_handler>(new _event_reg_handler);
    reg->_keys = std::unique_ptr<cps_api_key_t[]>(new cps_api_key_t[len]);
    for (size_t ix =0; ix  < len ; ++ix ) {
        auto &it = reg->_keys[ix];
        if (!cps_api_key_from_attr_with_qual(&it,attr[ix],quals[ix])) {
            return nullptr;
        }
    }
    reg->_reg.number_of_objects = len;
    reg->_reg.objects = reg->_keys.get();
    return reg;
}

std::unique_ptr<_event_reg_handler> _setup_key_reg(cps_api_attr_id_t attr, cps_api_qualifier_t qual=cps_api_qualifier_TARGET, bool use_defaults=true) {
    return _setup_key_reg(&attr,&qual,1,use_defaults);
}

size_t _cnt_test_1=0;
std::mutex m;

bool __cps_api_event_thread_callback_t_test_1(cps_api_object_t object,void * context) {
    char buff[2000];
    static bool _first_time=true;
    printf("Test 1 - cnt:(%d) - Obj %s\n",(int)_cnt_test_1++,cps_api_object_to_string(object,buff,sizeof(buff)));
    if (_first_time) {
        auto reg = _setup_key_reg(BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH);
        printf("Registering again... = %d\n",cps_api_event_thread_reg(&reg->_reg,&__cps_api_event_thread_callback_t_test_1,nullptr)==cps_api_ret_code_OK);
        _first_time = false;
    }
    m.unlock();
    return true;
}



TEST(cps_api_events,basic_clients_10000) {
    cps_api_event_service_handle_t handle;
    ASSERT_TRUE(cps_api_event_client_connect(&handle)==cps_api_ret_code_OK);

    cps_api_event_reg_t reg;
    reg.priority = 0;

    cps_api_key_t keys;
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&keys,BASE_IP_IPV6,cps_api_qualifier_TARGET));

    reg.objects = &keys;
    reg.number_of_objects =1;

    ASSERT_TRUE(cps_api_event_client_register(handle,&reg)==cps_api_ret_code_OK) ;

    printf("Sending.... \n");
    size_t ix = 0;
    size_t mx = 10000;
    for ( ; ix < mx ; ++ix ) {
    	cps_api_object_t _obj = cps_api_object_list_get(Get1000(),ix%1000);
    	cps_api_event_publish(handle,_obj);
    }

    printf("Receiving.... \n");
    cps_api_object_guard og(cps_api_object_create());
    for ( ix = 0 ; ix < mx ; ++ix ) {
        ASSERT_TRUE(cps_api_wait_for_event(handle,og.get())==cps_api_ret_code_OK);

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



TEST(cps_api_events,full_test) {
    ASSERT_TRUE(cps_api_event_thread_init()==cps_api_ret_code_OK);
    ASSERT_TRUE(full_reg());
    ASSERT_TRUE(threaded_client_test());
}

TEST(cps_api_events,event_thread_tests) {
    auto reg = _setup_key_reg(BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH);
    ASSERT_TRUE(cps_api_event_thread_reg(&reg->_reg,&__cps_api_event_thread_callback_t_test_1,nullptr)==cps_api_ret_code_OK);

    m.lock();
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV4_ADDRESS_PREFIX_LENGTH,true));

    cps_api_event_thread_publish(og.get());
    m.lock();

    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6_ADDRESS_PREFIX_LENGTH,true));

    cps_api_event_thread_publish(og.get());
    m.lock();
}

#endif
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int _rc = RUN_ALL_TESTS();
  __init_cleanup();
  return _rc;
}
