/* OPENSOURCELICENSE */
/*
 * cps_api_operation_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#include "private/cps_ns.h"
#include "cps_api_operation.h"



static cps_api_return_code_t db_read_function (void * context, cps_api_get_params_t * param, size_t key_ix) {
    STD_ASSERT(key_ix < param->key_count);
    cps_api_key_t * the_key = &param->keys[key_ix];

    uint32_t inst = cps_api_key_element_at(the_key,CPS_OBJ_KEY_APP_INST_POS);

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_set_key(obj,the_key);
    cps_api_object_attr_add(obj,0,"Interface",strlen("Interface"));
    cps_api_object_attr_add(obj,inst,"Interface",strlen("Interface"));
    cps_api_object_attr_add_u64(obj,3,(uint64_t)inst);
    cps_api_object_list_append(param->list,obj);

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    cps_api_object_t obj = cps_api_object_list_get(param->change_list,index_of_element_being_updated);
    STD_ASSERT(obj!=NULL);

    cps_api_object_attr_t it = cps_api_object_attr_first(obj);

    for ( ; it != CPS_API_ATTR_NULL ;
            it = cps_api_object_attr_next(obj,it)) {
        char buff[100];
        printf("Set... Found attr %s \n",cps_api_object_attr_to_string(it,buff,sizeof(buff)));
    }
    cps_api_object_t old = cps_api_object_create();
    if (!cps_api_object_clone(old,obj)) return cps_api_ret_code_ERR;
    if (!cps_api_object_list_append(param->prev,old)) {
        cps_api_object_delete(old);
        return cps_api_ret_code_ERR;
    }
    if (index_of_element_being_updated > 0) return cps_api_ret_code_ERR;

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {

    return cps_api_ret_code_OK;
}
static cps_api_operation_handle_t _serv_handle;

bool do_test_init(void) {
    /**
     * Service startup... internal to API not needed for others
     */
    if (cps_api_ns_startup()!=cps_api_ret_code_OK) {
        return false;
    }

    /**
     * Create a operation object handle for use with future registrations.
     */
    if (cps_api_operation_subsystem_init(&_serv_handle,1)!=cps_api_ret_code_OK) return false;

    cps_api_registration_functions_t funcs;
    funcs.handle = _serv_handle;
    funcs.context = (void*)"Cliff";
    funcs._read_function = db_read_function;
    funcs._write_function = db_write_function;
    funcs._rollback_function = db_rollback_function;

    cps_api_key_init(&funcs.key,cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,0,0);

    return (cps_api_register(&funcs)==cps_api_ret_code_OK);
}

bool do_test_get(void) {
    cps_api_get_params_t get_req;
    if (cps_api_get_request_init(&get_req)!=cps_api_ret_code_OK) return false;
    cps_api_key_t keys[3];
    char buff[1024];

    cps_api_key_init(&keys[0],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,1,1);

    printf("Key - %s\n",cps_api_key_print(&keys[0],buff,sizeof(buff)));


    cps_api_key_init(&keys[1],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,1,2);
    printf("Key - %s\n",cps_api_key_print(&keys[1],buff,sizeof(buff)));

    cps_api_key_init(&keys[2],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,1,3);
    printf("Key - %s\n",cps_api_key_print(&keys[2],buff,sizeof(buff)));


    get_req.key_count = sizeof(keys)/sizeof(*keys);
    get_req.keys = keys;

    if (cps_api_get(&get_req)!=cps_api_ret_code_OK) return false;

    size_t len = cps_api_object_list_size(get_req.list);
    size_t ix = 0;
    for ( ; ix < len ; ++ix ) {
        cps_api_object_t obj =cps_api_object_list_get(get_req.list,ix);
        cps_api_object_attr_t it = cps_api_object_attr_first(obj);

        for ( ; it != CPS_API_ATTR_NULL ;
                it = cps_api_object_attr_next(obj,it)) {
            char buff[100];
            printf("Found attr %s \n",cps_api_object_attr_to_string(it,buff,sizeof(buff)));
        }
    }
    cps_api_get_request_close(&get_req);
    return ix == 3;
}

bool test_set(void) {
    cps_api_transaction_params_t trans;
    if (cps_api_transaction_init(&trans)!=cps_api_ret_code_OK) return false;

    cps_api_get_params_t get_req;
    if (cps_api_get_request_init(&get_req)!=cps_api_ret_code_OK) return false;
    cps_api_key_t keys;

    cps_api_key_init(&keys,cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,1,1);

    cps_api_key_set(&keys,CPS_OBJ_KEY_APP_INST_POS,0);
    get_req.key_count = 1;
    get_req.keys = &keys;

    if (cps_api_get(&get_req)!=cps_api_ret_code_OK) return false;
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

    if (!cps_api_object_attr_add(obj,6,"Cliff",6)) return false;

    if (create) {
        if (cps_api_create(&trans,obj)!=cps_api_ret_code_OK) return false;
    } else {
        if (cps_api_set(&trans,obj)!=cps_api_ret_code_OK) return false;
    }

    if (create) {
        if (cps_api_create(&trans,obj)!=cps_api_ret_code_OK) return false;
    } else {
        if (cps_api_set(&trans,obj)!=cps_api_ret_code_OK) return false;
    }

    if (cps_api_commit(&trans)!=cps_api_ret_code_OK) return false;
    return true;
}

TEST(cps_api_object,ram_based) {
    ASSERT_TRUE(do_test_init());
    ASSERT_TRUE(do_test_get());
    ASSERT_TRUE(test_set());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
