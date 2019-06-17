/*
 * Copyright (c) 2019 Dell Inc.
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

#include "private/db/cps_api_db.h"



#include "cps_api_operation.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"

#include "cps_api_service.h"
#include "cps_class_map.h"
#include "cps_api_node.h"

#include "cps_api_object.h"
#include "cps_string_utils.h"

#include "cps_class_ut_data.h"


#include <pthread.h>
#include <sys/select.h>
#include <thread>
#include <mutex>

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "gtest/gtest.h"

TEST(cps_api_db,init) {
    __init_class_map();

}

TEST(cps_api_db,db_clear_all) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    ASSERT_TRUE(cps_db::delete_object_list(con,lg.get()));
}

TEST(cps_api_db,set_100_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::store_objects(con,Get100()));
}

TEST(cps_api_db,get_100_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    printf("Retrieved %d objects\n",(int)cps_api_object_list_size(lg.get()));
}

TEST(cps_api_db,del_100_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    ASSERT_TRUE(cps_db::delete_object_list(con,Get100()));
}

TEST(cps_api_db,set_1000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::store_objects(con,Get1000()));
}

TEST(cps_api_db,get_1000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    printf("Retrieved %d objects\n",(int)cps_api_object_list_size(lg.get()));
}

TEST(cps_api_db,del_1000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::delete_object_list(con,Get1000()));
}

TEST(cps_api_db,set_10000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::store_objects(con,Get10000()));
}

TEST(cps_api_db,get_10000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    printf("Retrieved %d objects\n",(int)cps_api_object_list_size(lg.get()));
}

TEST(cps_api_db,del_10000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::delete_object_list(con,Get10000()));
}

TEST(cps_api_db,set_100000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::store_objects(con,Get100000()));
}

TEST(cps_api_db,get_100000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    printf("Retrieved %d objects\n",(int)cps_api_object_list_size(lg.get()));
}

TEST(cps_api_db,del_100000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::delete_object_list(con,Get100000()));
}

TEST(cps_api_db,set_1000000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::store_objects(con,Get1000000()));
}

TEST(cps_api_db,get_1000000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_list_guard lg(cps_api_object_list_create());
    ASSERT_TRUE(cps_db::get_objects(con,og.get(),lg.get()));
    printf("Retrieved %d objects\n",(int)cps_api_object_list_size(lg.get()));
}

TEST(cps_api_db,del_1000000_pieces_of_data) {
    cps_db::connection con;
    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));
    ASSERT_TRUE(cps_db::delete_object_list(con,Get1000000()));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
