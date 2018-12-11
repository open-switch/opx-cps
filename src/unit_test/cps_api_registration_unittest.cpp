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
 * cps_registeration_unittest.cpp
 *
 *  Created on: Oct 26, 2015
 */

#include "std_thread_tools.h"
#include "cps_class_map.h"
#include "std_time_tools.h"
#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_service.h"

#include <gtest/gtest.h>


static cps_api_key_t _key_1;
static cps_api_key_t _key_2;
static bool finished = false;

#include <vector>

static struct {
  std::vector<cps_api_attr_id_t> _ids;
  cps_class_map_node_details details;
} lst[] = {
{{19,1,4}, { "base-sflow/entry/direction","",false,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19,1,2}, { "base-sflow/entry/id","",false,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19}, { "base-sflow","",true,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19,1,1}, { "base-sflow/entry/oid","",false,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19,1}, { "base-sflow/entry","",true,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19,1,5}, { "base-sflow/entry/sampling-rate","",false,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
{{19,1,3}, { "base-sflow/entry/ifindex","",false,CPS_CLASS_ATTR_T_LEAF,CPS_CLASS_DATA_TYPE_T_BIN}},
};

static const size_t lst_len = sizeof(lst)/sizeof(*lst);


TEST(registration_test,init) {
   size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(ix,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
}

static cps_api_return_code_t _write_function_(void * context,
        cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    return cps_api_ret_code_OK;
}

void *_std_thread_function_(void * param) {
    cps_api_operation_handle_t h;
    cps_api_operation_subsystem_init(&h,4);

    cps_api_registration_functions_t f;
    memset(&f,0,sizeof(f));
    f.handle = h;
    f._write_function  = _write_function_;

    cps_api_key_copy(&f.key,&_key_1);
    cps_api_register(&f);

    std_usleep(MILLI_TO_MICRO(5000));

    cps_api_key_copy(&f.key,&_key_2);
    cps_api_register(&f);

    std_usleep(MILLI_TO_MICRO(1000));

    finished=true;
    while(true) std_usleep(10000);
    return NULL;
}

TEST(registration_test,keys) {

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&_key_1,5,cps_api_qualifier_TARGET));
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(&_key_2,3,cps_api_qualifier_TARGET));

    cps_api_event_service_handle_t h;
    ASSERT_TRUE(cps_api_event_client_connect(&h)==cps_api_ret_code_OK);

    cps_api_event_reg_t r;
    cps_api_key_t e_key;
    memset(&r,0,sizeof(r));
    r.number_of_objects = 1;
    r.objects = &e_key;

    cps_api_key_copy(&e_key,&_key_1);
    cps_api_key_insert_element(&e_key,0,cps_api_qualifier_REGISTRATION);
    cps_api_event_client_register(h,&r);

    cps_api_key_copy(&e_key,&_key_2);
    cps_api_key_insert_element(&e_key,0,cps_api_qualifier_REGISTRATION);
    cps_api_event_client_register(h,&r);

    std_thread_create_param_t p;
    std_thread_init_struct(&p);
    p.name = "Test thread";
    p.thread_function = _std_thread_function_;

    ASSERT_TRUE(std_thread_create(&p)==STD_ERR_OK);

    while (!cps_api_is_registered(&_key_1,NULL)) {
        std_usleep(MILLI_TO_MICRO(100));
    }

    cps_api_object_t obj = cps_api_object_create();

    size_t cnt = 0;

    while (cps_api_wait_for_event(h,obj)==cps_api_ret_code_OK) {
        cps_api_key_t *k = cps_api_object_key(obj);
        if (cps_api_key_element_at(k,0)==cps_api_qualifier_REGISTRATION) {
            cps_api_key_remove_element(k,0);

            cnt += cps_api_key_matches(k,&_key_1,true)==0 ? 1 : 0;
            cnt += cps_api_key_matches(k,&_key_2,true)==0 ? 1 : 0;
        }
        if (cnt==2) return;
        if (finished) {
            ASSERT_TRUE(false);
        }
    }

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  //unit test init...
  cps_api_event_service_init();
  cps_api_unittest_init();
  cps_api_services_start();

  return RUN_ALL_TESTS();
}
