
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


#include "cps_api_db_interface.h"

#include "cps_class_ut_data.h"

#include "cps_api_object_tools.h"

#include <gtest/gtest.h>

/**
 * Test setup and configure the object for BASE_IP_IPV6 to be stored only in the DB
 */
TEST(cps_api_db_direct,setup) {
    std::vector<cps_api_attr_id_t> ids ;

    __init_class_map();
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);
}

static void _perf_loop(size_t count, cps_api_object_list_t objs) {
    size_t ix = 0;
    size_t mx = count;
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_list_guard lg(cps_api_object_list_clone(objs,true));

        cps_api_db_commit_bulk_t bulk;
        bulk.objects = lg.get();
        bulk.op =cps_api_oper_CREATE;
        bulk.node_group = nullptr;
        bulk.publish = false;
        (void) cps_api_db_commit_bulk(&bulk);

        cps_api_db_get_bulk(lg.get(),nullptr);

        bulk.objects = lg.get();
        bulk.op =cps_api_oper_DELETE;
        (void)cps_api_db_commit_bulk(&bulk);
    }
}

TEST(cps_api_db_direct,100x1000_commits) {
    cps_api_object_list_guard lg(cps_api_object_list_clone(Get1000(),true));

    cps_api_db_commit_bulk_t bulk;
    bulk.objects = lg.get();
    bulk.op =cps_api_oper_CREATE;
    bulk.node_group = nullptr;
    bulk.publish = false;

    size_t ix = 0;
    size_t mx = 100;
    for (; ix < mx ; ++ix ) {
        (void) cps_api_db_commit_bulk(&bulk);
    }
}

TEST(cps_api_db_direct,100x1000_gets) {
    cps_api_object_list_guard lg(cps_api_object_list_clone(Get1000(),true));

    size_t ix = 0;
    size_t mx = 100;
    for (; ix < mx ; ++ix ) {
        lg.set(cps_api_object_list_clone(Get1000(),true));
        cps_api_db_get_bulk(lg.get(),nullptr);
    }
}

TEST(cps_api_db_direct,100x1000_deletes) {
    cps_api_object_list_guard lg(cps_api_object_list_clone(Get1000(),true));

    cps_api_db_commit_bulk_t bulk;
    bulk.objects = lg.get();
    bulk.op =cps_api_oper_DELETE;
    bulk.node_group = nullptr;
    bulk.publish = false;

    size_t ix = 0;
    size_t mx = 100;
    for (; ix < mx ; ++ix ) {
        (void) cps_api_db_commit_bulk(&bulk);
    }
}


TEST(cps_api_db_direct,1x100) {
    _perf_loop(1,Get100());
}

TEST(cps_api_db_direct,10x100) {
    _perf_loop(10,Get100());
}

TEST(cps_api_db_direct,100x100) {
    _perf_loop(100,Get100());
}

TEST(cps_api_db_direct,10x1000) {
    _perf_loop(10,Get1000());
}

TEST(cps_api_db_direct,100x1000) {
    _perf_loop(100,Get1000());
}

TEST(cps_api_db_direct,1000x1000) {
    _perf_loop(1000,Get1000());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
