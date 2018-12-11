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
 * cps_api_key_unittest.cpp
 */


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "gtest/gtest.h"
#include "cps_api_key.h"




TEST(cps_api_key,key_create) {
    cps_api_key_t key;
    memset(&key,0,sizeof(key));
    cps_api_key_set(&key,0,0);
    cps_api_key_set(&key,1,1);
    cps_api_key_set(&key,2,2);
    cps_api_key_set_len(&key,3);

    ASSERT_TRUE(cps_api_key_get_len(&key)==3);
    ASSERT_TRUE(cps_api_key_element_at(&key,0)==0);
    ASSERT_TRUE(cps_api_key_element_at(&key,1)==1);
    ASSERT_TRUE(cps_api_key_element_at(&key,2)==2);

}

TEST(cps_api_key,key_compare) {
    char buff1[1024];
    char buff2[1024];

    cps_api_key_t key;
    memset(&key,0,sizeof(key));
    cps_api_key_set(&key,0,1);
    cps_api_key_set(&key,1,3);
    cps_api_key_set(&key,2,1);
    cps_api_key_set(&key,3,1);
    cps_api_key_set_len(&key,4);

    cps_api_key_t key2;
    memset(&key2,0,sizeof(key2));
    cps_api_key_set(&key2,0,1);
    cps_api_key_set(&key2,1,3);
    cps_api_key_set(&key2,2,1);
    cps_api_key_set(&key2,3,1);
    cps_api_key_set_len(&key2,4);
    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)==0);

    cps_api_key_set(&key2,1,4);
    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)!=0);

    cps_api_key_set(&key2,1,3);
    cps_api_key_set(&key2,4,2);
    cps_api_key_set_len(&key2,3);
    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)==0);

    cps_api_key_set(&key2,1,2);
    cps_api_key_set_len(&key2,4);
    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)!=0);

    cps_api_key_t key3;
    memset(&key3,0,sizeof(key3));
    cps_api_key_set(&key3,0,1);
    cps_api_key_set(&key3,1,3);
    cps_api_key_set(&key3,2,1);
    cps_api_key_set_len(&key3,3);

    ASSERT_TRUE(cps_api_key_matches(&key,&key3,true)!=0);
    cps_api_key_set_len(&key3,1);
    ASSERT_TRUE(cps_api_key_matches(&key,&key3,false)==0);
    cps_api_key_set_len(&key3,2);
    ASSERT_TRUE(cps_api_key_matches(&key,&key3,false)==0);

    cps_api_key_set(&key2,1,3);
    ASSERT_TRUE(cps_api_key_matches(&key,&key2,true)==0);
    printf("Key 1 %s, Key 2 %s\n",cps_api_key_print(&key,buff1,sizeof(buff1)),
            cps_api_key_print(&key2,buff2,sizeof(buff2)));

    cps_api_key_set_len(&key2,2);
    cps_api_key_set(&key2,2,4);

    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)==0);
    printf("Key 1 %s, Key 2 %s\n",cps_api_key_print(&key,buff1,sizeof(buff1)),
            cps_api_key_print(&key2,buff2,sizeof(buff2)));

    ASSERT_TRUE(cps_api_key_matches(&key2,&key,false)!=0);

    cps_api_key_set(&key2,2,2);
    cps_api_key_set(&key,2,0);

    ASSERT_TRUE(cps_api_key_matches(&key,&key2,false)==0);

    printf("Key 1 %s, Key 2 %s\n",cps_api_key_print(&key,buff1,sizeof(buff1)),
            cps_api_key_print(&key2,buff2,sizeof(buff2)));

    memset(&key2,0,sizeof(key2));
    cps_api_key_copy(&key2,&key);

    ASSERT_TRUE(cps_api_key_matches(&key,&key2,true)==0);

    printf("Final\nKey 1 %s, Key 2 %s\n",cps_api_key_print(&key,buff1,sizeof(buff1)),
            cps_api_key_print(&key2,buff2,sizeof(buff2)));
}

TEST(cps_api_key,key_insert) {
    cps_api_key_t key;
    memset(&key,0,sizeof(key));
    cps_api_key_set(&key,0,0);
    cps_api_key_set(&key,1,1);
    cps_api_key_set(&key,2,2);
    cps_api_key_set_len(&key,3);

    cps_api_key_t new_key ;
    cps_api_key_copy(&new_key,&key);


    cps_api_key_remove_element(&new_key,0);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,0)== 1);
    ASSERT_TRUE(cps_api_key_get_len(&new_key)==2);

    cps_api_key_insert_element(&new_key,0,5);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,0)== 5);
    ASSERT_TRUE(cps_api_key_get_len(&new_key)==3);

    cps_api_key_insert_element(&new_key,3,5);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,3)== 5);
    ASSERT_TRUE(cps_api_key_get_len(&new_key)==4);

    cps_api_key_insert_element(&new_key,2,5);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,2)== 5);
    ASSERT_TRUE(cps_api_key_get_len(&new_key)==5);

    ASSERT_TRUE(cps_api_key_element_at(&new_key,0)== 5);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,2)== 5);
    ASSERT_TRUE(cps_api_key_element_at(&new_key,4)== 5);

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
