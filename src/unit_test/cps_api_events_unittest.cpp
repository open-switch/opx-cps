/* OPENSOURCELICENSE */
/*
 * cps_api_events_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "cps_api_events.h"


#include <stdio.h>
#include <stdlib.h>

#include "gtest/gtest.h"

#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"

cps_api_event_service_handle_t handle;

size_t cnt =0;

bool _cps_api_event_thread_callback(cps_api_object_t object,void * context) {
     char buff[1024];
     printf("1 - Obj %s\n",cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     return true;
}

bool _cps_api_event_thread_callback_2(cps_api_object_t object,void * context) {
     char buff[1024];
     printf("2 - Obj %s\n",cps_api_object_to_string(object,buff,sizeof(buff)));
     ++cnt;
     return true;
}

bool init(void) {

     bool rc =  cps_api_event_service_init()==cps_api_ret_code_OK;

     if (!rc) return false;

     if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) return false;

     cps_api_event_reg_t reg;
     reg.priority = 0;

    cps_api_key_t keys[2];
    cps_api_key_init(&keys[0],cps_api_qualifier_TARGET,
            cps_api_obj_cat_INTERFACE,1,0);

    cps_api_key_init(&keys[1],cps_api_qualifier_TARGET,
            cps_api_obj_cat_ROUTE,1,0);

    reg.objects = keys;
    reg.number_of_objects = sizeof(keys)/sizeof(*keys);

    if (cps_api_event_client_register(handle,&reg)!=cps_api_ret_code_OK) return false;

    if (cps_api_event_thread_init()!=cps_api_ret_code_OK) return false;

    if (cps_api_event_thread_reg(&reg,
            _cps_api_event_thread_callback,NULL)!=cps_api_ret_code_OK)
        return false;

    if (cps_api_event_thread_reg(&reg,
            _cps_api_event_thread_callback_2,NULL)!=cps_api_ret_code_OK)
        return false;

    return true;
}

bool send_receive(void) {
    cps_api_object_t obj = cps_api_object_create();
    cps_api_key_init(cps_api_object_key(obj),cps_api_qualifier_TARGET,
                cps_api_obj_cat_INTERFACE,1,1,3);
    cps_api_object_attr_add(obj,1,"Cliff",6);

    if (cps_api_event_thread_publish(obj)!=cps_api_ret_code_OK) return false;
    while (cnt<2) sleep(1);

    if (cps_api_event_publish(handle,obj)!=cps_api_ret_code_OK) return false;
    while (cnt<4) sleep(1);

    const int MAX_OBJ_LEN=(1024);
    cps_api_object_t rec = cps_api_object_create();

    if (cps_api_object_reserve(rec,MAX_OBJ_LEN)==cps_api_ret_code_OK) return true;
    if (cps_api_wait_for_event(handle,rec)!=cps_api_ret_code_OK) return false;

    //compare the received objects for the unit test only.  Should really walk through the attributes
    //of the object and do something with them but.. this is a unit test with fake data..
    return memcmp(cps_api_object_array(obj),cps_api_object_array(rec),
            cps_api_object_to_array_len(obj))==0;
}

TEST(cps_api_events,init) {
    ASSERT_TRUE(init());

    ASSERT_TRUE(send_receive());

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
