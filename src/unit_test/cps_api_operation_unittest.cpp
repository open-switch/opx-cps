/*
 * Copyright (c) 2018 Dell Inc.
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
 * cps_api_operation_unittest.cpp
 */


#include "cps_class_ut_data.h"


#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"

#include "cps_class_map.h"
#include "cps_api_object_key.h"

#include "cps_api_object_tools.h"

#include "cps_api_operation_tools.h"

#include "private/cps_string_utils.h"
#include "private/cps_ns.h" //IPC testing
#include "private/cps_api_client_utils.h"

#include "std_socket_tools.h"
#include "std_time_tools.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <cstdio>
#include <memory>
#include <pthread.h>
#include <thread>
#include <unistd.h>

#include <mutex>


#include "gtest/gtest.h"

enum local_tlvs : cps_api_attr_id_t {
    ID_FAIL_REQ,
    ID_PRINT_OBJ,
    ID_GET_NUMBER,
    ID_SLEEP_TIME,
    ID_FAIL_RESP,
};

#define OBJECTS_IN_GET (10000)

int fds[2];


std::mutex __m;

size_t __get_request = 0;
size_t __set_request = 0;
size_t __amount_of_get_objs=0;

TEST(cps_api_operation_unittest,test_init) {
    __init_class_map();
    /**
     * Service startup... internal to API not needed for others
     */
    cps_api_services_start();
    cps_api_ns_startup();
}

static cps_api_return_code_t db_read_function (void * context, cps_api_get_params_t * param, size_t key_ix) {
    STD_ASSERT(param->filters!=nullptr);
    STD_ASSERT(key_ix < param->key_count);

    cps_api_object_t obj = cps_api_object_list_get(param->filters,key_ix);
    STD_ASSERT(obj!=nullptr);

    {
    std::lock_guard<std::mutex> g(__m);
    __get_request++;
    }

    size_t sleep_time = 0;
    uint32_t *_sleep_time = (uint32_t*) cps_api_object_get_data(obj,local_tlvs::ID_SLEEP_TIME);
    if (_sleep_time!=nullptr) sleep_time = *_sleep_time;

    if (sleep_time!=0) {
        printf("Received request... %d (sleep %d) (%d)\n",(int)__get_request,(int)sleep_time,(int)pthread_self());
        sleep(sleep_time);
    }

    size_t _list_len = OBJECTS_IN_GET;
    uint32_t *_get_size = (uint32_t*) cps_api_object_get_data(obj,ID_GET_NUMBER);
    if (_get_size!=nullptr) _list_len = *_get_size;

    auto _should_fail = cps_api_object_get_data(obj,ID_FAIL_REQ);
    if (_should_fail!=nullptr) {
        cps_api_object_attr_add(obj,ID_FAIL_RESP,"asdadasdasd",strlen("asdadasdasd"));
        return cps_api_ret_code_ERR;
    }

    cps_api_object_list_guard _the_list(cps_api_object_list_clone(GetFromCount(_list_len),true));

    cps_api_object_list_merge(param->list,_the_list.get());
    {
        std::lock_guard<std::mutex> g(__m);
        __amount_of_get_objs+=_list_len;
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context,
        cps_api_transaction_params_t * param, size_t key_ix) {
    STD_ASSERT(param->change_list!=nullptr);
    STD_ASSERT(key_ix < cps_api_object_list_size(param->change_list));

    {
    std::lock_guard<std::mutex> g(__m);
    __set_request++;
    }

    cps_api_object_t obj = cps_api_object_list_get(param->change_list,key_ix);
    STD_ASSERT(obj!=NULL);

    auto _debug_flag = cps_api_object_get_data(obj,ID_PRINT_OBJ);
    if (_debug_flag!=nullptr) cps_api_object_print(obj);

    auto _should_fail = cps_api_object_get_data(obj,ID_FAIL_REQ);
    if (_should_fail!=nullptr) {
    	cps_api_set_object_return_attrs(obj,cps_api_ret_code_ERR,"Someone was here %d=%s",1,"cliff");
        return cps_api_ret_code_ERR;
    }
    //std_usleep(50);
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    return cps_api_ret_code_OK;
}

static cps_api_operation_handle_t _serv_handle;

TEST(cps_api_operation_unittest,register_objects) {
    /**
     * Create a operation object handle for use with future registrations.
     */
    ASSERT_TRUE(cps_api_operation_subsystem_init(&_serv_handle,1)==cps_api_ret_code_OK);

    cps_api_attr_id_t _ids[] = {BASE_IP_IPV6,BASE_IP_IPV4,BASE_IP_IPV4_ADDRESS,BASE_IP_IPV6_ADDRESS};

    for (auto &it : _ids ) {
        cps_api_registration_functions_t funcs;
        funcs.handle = _serv_handle;
        funcs.context = (void*)"Cliff";
        funcs._read_function = db_read_function;
        funcs._write_function = db_write_function;
        funcs._rollback_function = db_rollback_function;

        cps_api_key_from_attr_with_qual(&funcs.key,it,cps_api_qualifier_TARGET);

        ASSERT_TRUE(cps_api_register(&funcs)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_operation_unittest,test_error_on_get_one) {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));
    cps_api_object_attr_add_u32(og.get(),ID_FAIL_REQ,true);

    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_api_get_objs(og.get(),lg.get(),1,0)==cps_api_ret_code_ERR);
}

TEST(cps_api_operation_unittest,simple_test_get_filtered) {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));

    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,10);
    cps_api_object_exact_match(og.get(),true);

    cps_api_object_list_guard lg(cps_api_object_list_create());
    bool _rc = (cps_api_get_objs(og.get(),lg.get(),0,0)==cps_api_ret_code_OK) && cps_api_object_list_size(lg.get())==1;
    ASSERT_TRUE(_rc==true);
}


bool _get_amount(size_t amount) {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));
    cps_api_object_attr_add_u32(og.get(),ID_GET_NUMBER,amount);

    cps_api_object_list_guard lg(cps_api_object_list_create());
    return (cps_api_get_objs(og.get(),lg.get(),0,0)==cps_api_ret_code_OK) && cps_api_object_list_size(lg.get())==amount;
}

TEST(cps_api_operation_unittest,simple_test_get_1) {
    ASSERT_TRUE(_get_amount(1));
}

TEST(cps_api_operation_unittest,simple_test_get_10) {
    ASSERT_TRUE(_get_amount(10));
}

TEST(cps_api_operation_unittest,simple_test_get_1000) {
    ASSERT_TRUE(_get_amount(1000));
}

TEST(cps_api_operation_unittest,simple_test_get_100000) {
    ASSERT_TRUE(_get_amount(100000));
}

TEST(cps_api_operation_unittest,simple_test_get_1000_100) {
    for (size_t ix = 0; ix < 1000 ; ++ix ) {
        ASSERT_TRUE(_get_amount(100));
    }
}

TEST(cps_api_operation_unittest,simple_test_get_100_1000) {
    for (size_t ix = 0; ix < 100; ++ix ) {
        ASSERT_TRUE(_get_amount(1000));
    }
}

TEST(cps_api_operation_unittest,simple_test_create_fail) {
	cps_api_object_guard og (cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));

    cps_api_object_attr_add_u32(og.get(),ID_FAIL_REQ,true);

    auto _start=__set_request;
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,0)==cps_api_ret_code_ERR);
    auto _end=__set_request;

    const char * _err_str = cps_api_object_return_string(og.get());
    const cps_api_return_code_t* _rc = cps_api_object_return_code(og.get());
    ASSERT_TRUE(_err_str!=nullptr);
    ASSERT_TRUE(_rc!=nullptr);
    ASSERT_EQ((_start+1),_end);
}

TEST(cps_api_operation_unittest,simple_test_create) {
	cps_api_object_guard og (cps_api_object_create());
	cps_api_object_clone(og.get(),cps_api_object_list_get(Get1000(),0));
	auto _start=__set_request;
   ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,0)==cps_api_ret_code_OK);
   auto _end=__set_request;
   ASSERT_EQ((_start+1),_end);
}

TEST(cps_api_operation_unittest,simple_test_set) {
    auto _start=__set_request;
    cps_api_object_guard og (cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_SET,og.get(),1,0)==cps_api_ret_code_OK);
    auto _end=__set_request;
    ASSERT_TRUE((_start+1)==_end);
}

TEST(cps_api_operation_unittest,simple_test_del) {
    auto _start=__set_request;
    cps_api_object_guard og (cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_DELETE,og.get(),1,0)==cps_api_ret_code_OK);
    auto _end=__set_request;
    ASSERT_TRUE((_start+1)==_end);
}

TEST(cps_api_operation_unittest,simple_test_action) {
    auto _start=__set_request;
    cps_api_object_guard og (cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_ACTION,og.get(),1,0)==cps_api_ret_code_OK);
    auto _end=__set_request;
    ASSERT_TRUE((_start+1)==_end);
}

bool handle_operation(size_t count) {
    cps_api_transaction_params_t tr;
    cps_api_return_code_t _rc = cps_api_ret_code_OK;

    if ((_rc=cps_api_transaction_init(&tr))!=cps_api_ret_code_OK) {
        return _rc;
    }
    cps_api_transaction_guard tr_g(&tr);

    for ( size_t ix = 0; ix < count ; ++ix ) {
        cps_api_object_guard og(nullptr);

        og.set(cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));
        cps_api_create(&tr,og.get());
        og.release();

        og.set(cps_api_object_create());
        cps_api_object_clone(og.get(),cps_api_object_list_get(Get1000(),0));
        cps_api_set(&tr,og.get());
        og.release();

        og.set(cps_api_object_create());
        cps_api_object_clone(og.get(),cps_api_object_list_get(Get1000(),0));
        cps_api_delete(&tr,og.get());
        og.release();

        og.set(cps_api_object_create_clone(cps_api_object_list_get(Get1000(),0)));
        cps_api_action(&tr,og.get());
        og.release();
    }
    _rc = cps_api_commit(&tr);
    if (_rc!=cps_api_ret_code_OK) return false;
    return true;
}

TEST(cps_api_operation_unittest,simple_operations_1) {
    handle_operation(1);
}

TEST(cps_api_operation_unittest,simple_operations_100) {
    handle_operation(100);
}

TEST(cps_api_operation_unittest,simple_operations_1000) {
    handle_operation(1000);
}

TEST(cps_api_operation_unittest,simple_operations_10000) {
    handle_operation(10000);
}

TEST(cps_api_operation_unittest,simple_operations_100000) {
    handle_operation(100000);
}

#if 0
TEST(cps_api_operation_unittest,test_get) {


    cps_api_get_params_t get_req;

    ASSERT_TRUE(cps_api_get_request_init(&get_req)==cps_api_ret_code_OK);
    cps_api_key_t keys[2];
    char buff[1024];

    cps_api_key_from_attr_with_qual(&keys[0],BASE_IP_IPV6,cps_api_qualifier_TARGET);
    printf("Key - %s\n",cps_api_key_print(&keys[0],buff,sizeof(buff)));

    cps_api_key_from_attr_with_qual(&keys[1],BASE_IP_IPV4,cps_api_qualifier_TARGET);
    printf("Key - %s\n",cps_api_key_print(&keys[1],buff,sizeof(buff)));

    get_req.key_count = sizeof(keys)/sizeof(*keys);
    get_req.keys = keys;

    ASSERT_TRUE(cps_api_get(&get_req)==cps_api_ret_code_OK);

    size_t expected = get_req.key_count * OBJECTS_IN_GET;

    size_t len = cps_api_object_list_size(get_req.list);
    size_t ix = 0;
    for ( ; ix < len ; ++ix ) {
        cps_api_object_t obj =cps_api_object_list_get(get_req.list,ix);

        cps_api_object_it_t it;
        cps_api_object_it_begin(obj,&it);


        for ( ; cps_api_object_it_valid(&it) ; cps_api_object_it_next(&it) ) {
            char buff[100];
            printf("Found attr %s \n",cps_api_object_attr_to_string(it.attr,buff,sizeof(buff)));
        }
    }
    cps_api_get_request_close(&get_req);
    ASSERT_TRUE(ix == expected);
}

TEST(cps_api_operation_unittest,test_set) {

    cps_api_get_params_t get_req;
    ASSERT_TRUE (cps_api_get_request_init(&get_req)==cps_api_ret_code_OK);

    cps_api_key_t keys;
    cps_api_key_from_attr_with_qual(&keys,BASE_IP_IPV6,cps_api_qualifier_TARGET);

    get_req.keys = &keys;
    get_req.key_count = 1;

    ASSERT_TRUE (cps_api_get(&get_req)==cps_api_ret_code_OK);

    return ;
    cps_api_object_t obj;
    bool create = false;
    if (cps_api_object_list_size(get_req.list)>0) {
        obj = cps_api_object_list_get(get_req.list,0);
        STD_ASSERT(obj!=NULL);
        cps_api_object_list_remove(get_req.list,0);
    } else {
        obj = cps_api_object_create();
        uint32_t inst = cps_api_key_element_at(&keys,CPS_OBJ_KEY_APP_INST_POS);
        cps_api_object_set_key(obj,&keys);
        cps_api_object_attr_add(obj,0,"Interface",strlen("Interface"));
        cps_api_object_attr_add(obj,1,"Interface",strlen("Interface"));
        cps_api_object_attr_add_u64(obj,3,(uint64_t)inst);
        create =true;
    }
    cps_api_get_request_close(&get_req);

    cps_api_transaction_params_t trans;
    ASSERT_TRUE (cps_api_transaction_init(&trans)==cps_api_ret_code_OK);

    ASSERT_TRUE(cps_api_object_attr_add(obj,6,"Cliff",6));

    cps_api_object_t cloned = cps_api_object_create();
    cps_api_object_clone(cloned,obj);


    if (create) {
        ASSERT_TRUE(cps_api_create(&trans,obj)==cps_api_ret_code_OK);
    } else {
        ASSERT_TRUE (cps_api_set(&trans,obj)==cps_api_ret_code_OK);
    }

    if (create) {
        ASSERT_TRUE (cps_api_create(&trans,cloned)==cps_api_ret_code_OK) ;
    } else {
        ASSERT_TRUE (cps_api_set(&trans,cloned)==cps_api_ret_code_OK) ;
    }
    size_t ix = 0;
    size_t mx = cps_api_object_list_size(trans.change_list);
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(trans.change_list,ix);
        if (ix==0) ASSERT_TRUE(ix==0 && o==obj);
        if (ix==1) ASSERT_TRUE(ix==1 && o==cloned);
    }

    ASSERT_TRUE(cps_api_commit(&trans)==cps_api_ret_code_OK);
    cps_api_transaction_close(&trans);

    /**
     * Transaction initialize...
     */
    ASSERT_TRUE(cps_api_transaction_init(&trans)==cps_api_ret_code_OK);

    cps_api_object_guard og(cps_api_object_create());
    ASSERT_TRUE(og.get()!=nullptr);

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_set_key(obj,&keys);
    cps_api_object_attr_add(og.get(),0,"Interface",strlen("Interface"));
    cps_api_object_attr_add(og.get(),1,"Interface",strlen("Interface"));
    cps_api_object_attr_add_u64(og.get(),3,(uint64_t)cps_api_qualifier_TARGET);


    cps_api_object_guard _og2(cps_api_object_create());
    ASSERT_TRUE(_og2.get()!=nullptr);
    cps_api_object_clone(_og2.get(),og.get());

    cps_api_set(&trans,og.get());
    og.release();
    cps_api_object_attr_add_u64(cloned,4,true);
    cps_api_action(&trans,_og2.get());
    _og2.release();

    ASSERT_TRUE(cps_api_commit(&trans)!=cps_api_ret_code_OK);

    //transaction complete
    cps_api_transaction_close(&trans);
    og.set(cps_api_object_create());
    cps_api_object_stats(&keys, og.get());

    memset(&keys,0,sizeof(keys));
    cps_api_object_stats(&keys, og.get());
}

#include "cps_api_operation_tools.h"
#include "cps_api_object_tools.h"


TEST(cps_api_operation_unittest,test_tool_obj_test) {

    cps_api_object_t obj = cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true);
    cps_api_object_guard og1(obj);
    ASSERT_TRUE(og1.valid());

    obj = cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true);
    cps_api_object_guard og2(obj);
    ASSERT_TRUE(og2.valid());

    cps_api_object_attr_add(og1.get(),0,"Interface",strlen("Interface"));

    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),false));

    cps_api_object_attr_add(og2.get(),0,"Interface",strlen("Interface"));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),false));

    cps_api_object_attr_add(og2.get(),1,"Interface=",strlen("Interface="));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),false));
    ASSERT_FALSE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));

    cps_api_object_attr_add(og1.get(),1,"Interface=",strlen("Interface="));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));

    cps_api_object_attr_add_u32(og1.get(),2,10);
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));

    cps_api_object_attr_add_u32(og2.get(),2,20);
    ASSERT_FALSE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));
    ASSERT_FALSE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),false));
    cps_api_object_attr_delete(og2.get(),2);


    uint64_t val = 10;
    cps_api_set_key_data_uint(og2.get(),3,&val,sizeof(val));
    val = 4;
    cps_api_set_key_data_uint(og2.get(),4,&val,sizeof(val));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),false));
    ASSERT_FALSE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));


    val = 10;
    cps_api_set_key_data_uint(og1.get(),3,&val,sizeof(val));
    val = 4;
    cps_api_set_key_data_uint(og1.get(),4,&val,sizeof(val));
    ASSERT_TRUE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));

    cps_api_set_key_data_uint(og1.get(),5,&val,sizeof(val));
    val *= 2;
    cps_api_set_key_data_uint(og2.get(),5,&val,sizeof(val));
    ASSERT_FALSE(cps_api_obj_tool_matches_filter(og2.get(),og1.get(),true));

}


TEST(cps_api_operation_unittest,test_tool_op_test) {
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),4,100)==cps_api_ret_code_OK);

    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    ASSERT_FALSE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,100)==cps_api_ret_code_OK);
    }
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    cps_api_object_list_guard olg(cps_api_object_list_create());
    ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    cps_api_object_list_guard olg(cps_api_object_list_create());
    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    ASSERT_FALSE(cps_api_get_objs(og.get(), olg.get(), 3,1000)==cps_api_ret_code_OK);
    }
}
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int rc =  RUN_ALL_TESTS();
  UTDataClear();
  cps_api_list_debug();
  return rc;
}
