/* OPENSOURCELICENSE */
/*
 * cps_api_key_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
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
    cps_api_key_set(&key,0,0);
    cps_api_key_set(&key,1,1);
    cps_api_key_set(&key,2,2);
    cps_api_key_set_len(&key,3);

    cps_api_key_t key2;
    memset(&key2,0,sizeof(key2));
    cps_api_key_set(&key2,0,0);
    cps_api_key_set(&key2,1,1);
    cps_api_key_set(&key2,2,2);
    cps_api_key_set_len(&key2,3);

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
