/** OPENSOURCELICENSE */

/* OPENSOURCELICENSE */
/*
 * cps_api_timings_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */



#include "gtest/gtest.h"

#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"

#include "cps_class_map.h"
#include "cps_api_object_key.h"
#include "cps_api_operation_tools.h"
#include "cps_api_object_tools.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <cstdio>
#include <memory>

#define OBJECTS_IN_GET (10000)

static cps_api_attr_id_t ID_START=10;

cps_api_attr_id_t ids[][6] = {
        {ID_START+1,ID_START+3 ,ID_START+4,ID_START,ID_START+5 },
        {ID_START*10+1,ID_START*10+3 ,ID_START*10+4,ID_START*10,ID_START*10+5 },
};

TEST(cps_api_timer,test_init) {
    /**
     * Service startup... internal to API not needed for others
     */
    cps_api_services_start();
    cps_api_ns_startup();


    cps_class_map_node_details details;
    details.attr_type = CPS_CLASS_ATTR_T_LEAF;
    details.data_type = CPS_CLASS_DATA_TYPE_T_UINT64;
    details.desc = "Some descr";
    details.embedded = false;

    size_t ix = 0;
    size_t mx = sizeof(ids)/sizeof(*ids);
    for ( ; ix < mx ; ++ix ) {
        size_t jx = 0;
        size_t jmx = sizeof(ids[0])/sizeof(ids[0][1]);
        for ( ; jx < jmx ; ++jx ) {
            size_t len = std::snprintf(nullptr,0,"generic/id-%d",(int)ids[ix][jx])+1;
            std::unique_ptr<char[]> p (new char[len]);
            ASSERT_TRUE(p.get()!=nullptr);
            std::snprintf(p.get(),len,"generic/id-%d",(int)ids[ix][jx]);
            details.name = p.get();
            cps_class_map_init(ids[ix][jx],ids[ix],jx,&details);
        }
    }
}


static cps_api_return_code_t db_read_function (void * context, cps_api_get_params_t * param, size_t key_ix) {
    cps_api_object_t obj = cps_api_object_list_get(param->filters,key_ix);

    size_t mx = 1;
    cps_api_object_attr_t attr = cps_api_object_attr_get(obj,0);
    if (attr!=nullptr) {
        mx = cps_api_object_attr_data_u32(attr);
    }

    for ( size_t ix = 0; ix < mx ; ++ix ) {
        obj = cps_api_object_list_create_obj_and_append(param->list);
        STD_ASSERT(obj!=nullptr);
        //fill in some attributes and set key..
        cps_api_object_attr_add(obj,1,"Interface",strlen("Interface"));
        cps_api_object_attr_add_u64(obj,2,(uint64_t)ix);
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    cps_api_object_t obj = cps_api_object_list_get(param->change_list,index_of_element_being_updated);
    STD_ASSERT(obj!=NULL);
    //set the data..
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {

    return cps_api_ret_code_OK;
}
static cps_api_operation_handle_t _serv_handle;

TEST(cps_api_timer,test_reg) {
    /**
     * Create a operation object handle for use with future registrations.
     */
    ASSERT_TRUE(cps_api_operation_subsystem_init(&_serv_handle,1)==cps_api_ret_code_OK);

    cps_api_registration_functions_t funcs;
    funcs.handle = _serv_handle;
    funcs.context = (void*)"Cliff";
    funcs._read_function = db_read_function;
    funcs._write_function = db_write_function;
    funcs._rollback_function = db_rollback_function;

    cps_api_key_from_attr_with_qual(&funcs.key,ID_START,cps_api_qualifier_TARGET);

    ASSERT_TRUE(cps_api_register(&funcs)==cps_api_ret_code_OK);
}

TEST(cps_api_timer,test_benchmark_getobj_10000x1_test) {
    for ( size_t ix = 0 , mx = 10000; ix < mx ; ++ix ) {
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        cps_api_object_list_guard olg(cps_api_object_list_create());
        ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_timer,test_benchmark_getobj_10000x10_test) {
    for ( size_t ix = 0 , mx = 10000; ix < mx ; ++ix ) {
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        cps_api_object_attr_add_u64(og.get(),0,(uint64_t)10);
        cps_api_object_list_guard olg(cps_api_object_list_create());
        ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_timer,test_benchmark_getobj_1000x1000_test) {
    for ( size_t ix = 0 , mx = 1000; ix < mx ; ++ix ) {
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        cps_api_object_attr_add_u64(og.get(),0,(uint64_t)1000);
        cps_api_object_list_guard olg(cps_api_object_list_create());
        ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_timer,test_benchmark_getobj_20x100000_test) {
    for ( size_t ix = 0 , mx = 20; ix < mx ; ++ix ) {
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        cps_api_object_attr_add_u64(og.get(),0,(uint64_t)100000);
        cps_api_object_list_guard olg(cps_api_object_list_create());
        ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_timer,test_benchmark_getobj_1x1000000_test) {
    for ( size_t ix = 0 , mx = 1; ix < mx ; ++ix ) {
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        cps_api_object_attr_add_u64(og.get(),0,(uint64_t)1000000);
        cps_api_object_list_guard olg(cps_api_object_list_create());
        ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
}

TEST(cps_api_timer,test_benchmark_set_10000x1) {
    for ( size_t ix = 0, mx = 10000; ix < mx ; ++ix ){
        cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
        ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),4,100)==cps_api_ret_code_OK);
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int rc =  RUN_ALL_TESTS();
  cps_api_list_debug();
  return rc;
}
