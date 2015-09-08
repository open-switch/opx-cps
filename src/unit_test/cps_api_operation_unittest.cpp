/* OPENSOURCELICENSE */
/*
 * cps_api_operation_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */



#include "gtest/gtest.h"

#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"

#include "cps_class_map.h"
#include "cps_api_object_key.h"

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

TEST(cps_api_object,test_init) {
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
    STD_ASSERT(key_ix < param->key_count);
    cps_api_key_t * the_key = &param->keys[key_ix];

    uint32_t inst = cps_api_key_element_at(the_key,CPS_OBJ_KEY_APP_INST_POS);
    size_t ix = 0;
    size_t mx = OBJECTS_IN_GET;
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t obj = cps_api_object_create();
        cps_api_object_set_key(obj,the_key);
        cps_api_object_attr_add(obj,0,"Interface",strlen("Interface"));
        cps_api_object_attr_add(obj,inst,"Interface",strlen("Interface"));
        cps_api_object_attr_add_u64(obj,3,(uint64_t)inst);
        if (!cps_api_object_list_append(param->list,obj)) {
            cps_api_object_delete(obj);
            return cps_api_ret_code_ERR;
        }
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    cps_api_object_t obj = cps_api_object_list_get(param->change_list,index_of_element_being_updated);
    STD_ASSERT(obj!=NULL);

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    for ( ; cps_api_object_it_valid(&it) ; cps_api_object_it_next(&it) ) {
        char buff[100];
        printf("Set... Found attr %s \n",cps_api_object_attr_to_string(it.attr,buff,sizeof(buff)));
    }

    cps_api_object_attr_t attr = cps_api_object_attr_get(obj,4);
    if (attr!=NULL) {
        return cps_api_ret_code_ERR;
    }

    if (index_of_element_being_updated > 0) return cps_api_ret_code_ERR;

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {

    return cps_api_ret_code_OK;
}
static cps_api_operation_handle_t _serv_handle;

TEST(cps_api_object,test_reg) {
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

TEST(cps_api_object,test_get) {
    cps_api_get_params_t get_req;

    ASSERT_TRUE(cps_api_get_request_init(&get_req)==cps_api_ret_code_OK);
    cps_api_key_t keys[2];
    char buff[1024];

    cps_api_key_from_attr_with_qual(&keys[0],ID_START,cps_api_qualifier_TARGET);
    printf("Key - %s\n",cps_api_key_print(&keys[0],buff,sizeof(buff)));

    cps_api_key_from_attr_with_qual(&keys[1],ID_START+5,cps_api_qualifier_TARGET);
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

TEST(cps_api_object,test_set) {

    cps_api_get_params_t get_req;
    ASSERT_TRUE (cps_api_get_request_init(&get_req)==cps_api_ret_code_OK);

    cps_api_key_t keys;
    cps_api_key_from_attr_with_qual(&keys,ID_START,cps_api_qualifier_TARGET);

    get_req.keys = &keys;
    get_req.key_count = 1;

    ASSERT_TRUE (cps_api_get(&get_req)==cps_api_ret_code_OK);

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

    obj = cps_api_object_create();
    uint32_t inst = cps_api_key_element_at(&keys,CPS_OBJ_KEY_APP_INST_POS);
    cps_api_object_set_key(obj,&keys);
    cps_api_object_attr_add(obj,0,"Interface",strlen("Interface"));
    cps_api_object_attr_add(obj,1,"Interface",strlen("Interface"));
    cps_api_object_attr_add_u64(obj,3,(uint64_t)inst);

    cps_api_set(&trans,obj);

    cloned = cps_api_object_create();
    cps_api_object_clone(cloned,obj);
    cps_api_object_attr_add_u64(cloned,4,true);

    cps_api_action(&trans,cloned);

    ASSERT_TRUE(cps_api_commit(&trans)!=cps_api_ret_code_OK);

    //transaction complete
    cps_api_transaction_close(&trans);

}

#include "cps_api_operation_tools.h"
#include "cps_api_object_tools.h"


TEST(cps_api_object,test_tool_obj_test) {

    cps_api_object_t obj = cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true);
    cps_api_object_guard og1(obj);
    ASSERT_TRUE(og1.valid());

    obj = cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START*10,true);
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


TEST(cps_api_object,test_tool_op_test) {
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),4,100)==cps_api_ret_code_OK);

    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START*10,true));
    ASSERT_FALSE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,100)==cps_api_ret_code_OK);
    }
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
    cps_api_object_list_guard olg(cps_api_object_list_create());
    ASSERT_TRUE(cps_api_get_objs(og.get(), olg.get(), 3,100)==cps_api_ret_code_OK);
    }
    {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START,true));
    cps_api_object_list_guard olg(cps_api_object_list_create());
    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,ID_START*10,true));
    ASSERT_FALSE(cps_api_get_objs(og.get(), olg.get(), 3,1000)==cps_api_ret_code_OK);
    }
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int rc =  RUN_ALL_TESTS();
  cps_api_list_debug();
  return rc;
}
