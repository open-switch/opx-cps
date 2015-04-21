/** OPENSOURCELICENSE */
/*
 * cps_class_map_unittest.cpp
 *
 *  Created on: Apr 20, 2015
 */


#include "cps_class_map.h"
#include "cps_api_operation.h"

#include <gtest/gtest.h>


#include "cps_class_map.h"

#include <vector>

static struct {
  std::vector<cps_api_attr_id_t> _ids;
  cps_class_map_node_details details;
} lst[] = {
{{19,1,4}, { "base-sflow/entry/direction","",false,cps_api_object_ATTR_T_BIN}},
{{19,1,2}, { "base-sflow/entry/id","",false,cps_api_object_ATTR_T_BIN}},
{{19}, { "base-sflow","",true,cps_api_object_ATTR_T_BIN}},
{{19,1,1}, { "base-sflow/entry/oid","",false,cps_api_object_ATTR_T_BIN}},
{{19,1}, { "base-sflow/entry","",true,cps_api_object_ATTR_T_BIN}},
{{19,1,5}, { "base-sflow/entry/sampling-rate","",false,cps_api_object_ATTR_T_BIN}},
{{19,1,3}, { "base-sflow/entry/ifindex","",false,cps_api_object_ATTR_T_BIN}},
};

static const size_t lst_len = sizeof(lst)/sizeof(*lst);

TEST(cps_class_map,load) {
    ASSERT_TRUE(cps_class_objs_load("workspace/debian/jessie/x86_64/sysroot/opt/ngos/lib","libcpsclass-"));
}

TEST(cps_class_map,keys) {
   size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
    cps_api_key_t k;
    cps_api_key_init(&k,cps_api_qualifier_TARGET,
                19,1,0,0);
    cps_api_attr_id_t ids[]= {1};
    size_t ids_len = 1;

    ASSERT_TRUE(cps_class_attr_is_valid(&k,ids,ids_len));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

