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

#include "cps_api_operation.h"



static cps_api_return_code_t db_read_function (void * context, cps_api_get_params_t * param, size_t key_ix) {
    STD_ASSERT(key_ix < param->key_count);
    cps_api_key_t * the_key = param->keys[key_ix];

    uint32_t inst = cps_api_key_element_at(the_key,CPS_OBJ_KEY_APP_INST_POS);

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_set_key(obj,the_key);
    cps_api_object_attr_add(obj,0,"Interface",strlen("Interface"));
    cps_api_object_attr_add(obj,inst,"Interface",strlen("Interface"));
    cps_api_object_attr_add_u16(obj,1,(uint16_t)inst);
    cps_api_object_attr_add_u32(obj,2,(uint32_t)inst);
    cps_api_object_attr_add_u64(obj,3,(uint64_t)inst);
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {

}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {

}

bool do_test_init() {
    cps_api_registration_functions_t funcs;

    funcs.context = (void*)"Cliff";
    funcs._read_function = db_read_function;
    funcs._write_function = db_write_function;
    funcs._rollback_function = db_rollback_function;

    cps_api_key_init(&funcs.key,0,cps_api_inst_TARGET,
            cps_api_obj_cat_INTERFACE,0);

    return (cps_api_register(&funcs)==cps_api_ret_code_OK);
}

bool do_test_get() {
    cps_api_get_params_t get_req;
    if (cps_api_get_request_init(&get_req)!=cps_api_ret_code_OK) return false;
    cps_api_key_t keys[3];

    cps_api_key_init(&keys[0],1,cps_api_inst_TARGET,
            cps_api_obj_cat_INTERFACE,0);

    cps_api_key_init(&keys[1],1,cps_api_inst_TARGET,
            cps_api_obj_cat_INTERFACE,0);

    cps_api_key_init(&keys[2],1,cps_api_inst_TARGET,
            cps_api_obj_cat_INTERFACE,0);

    cps_api_key_set(&keys[0],CPS_OBJ_KEY_APP_INST_POS,0);
    cps_api_key_set(&keys[1],CPS_OBJ_KEY_APP_INST_POS,1);
    cps_api_key_set(&keys[2],CPS_OBJ_KEY_APP_INST_POS,2);
    get_req.key_count = sizeof(keys)/sizeof(*keys);
    get_req.keys = keys;

    if (cps_api_get(&get_req)!=cps_api_ret_code_OK) return false;

    size_t len = cps_api_object_list_size(get_req.list);
    size_t ix = 0;
    for ( ; ix < len ; ++ix ) {
        cps_api_object_t obj =cps_api_object_list_get(get_req.list,ix);
        cps_api_object_attr_t it = cps_api_object_attr_start(obj);

        for ( ; it != CPS_API_ATTR_NULL ;
                it = cps_api_object_attr_next(obj,it)) {
            char buff[100];
            printf("Found attr %s \n",cps_api_object_attr_to_string(it,buff,sizeof(buff)));
        }
    }

    return ix == 3;
}

TEST(cps_api_object,ram_based) {
    ASSERT_TRUE(do_test_init());
    ASSERT_TRUE(do_test_get());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
