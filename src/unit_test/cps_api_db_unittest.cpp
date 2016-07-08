/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */


#include "cps_api_node.h"
#include "cps_api_node_private.h"
#include "cps_class_map.h"
#include "cps_api_db.h"
#include "cps_string_utils.h"

#include "cps_api_object_tools.h"

#include "cps_api_operation_tools.h"
#include "cps_dictionary.h"

#include "cps_class_ut_data.h"
#include "dell-cps.h"

#include "gtest/gtest.h"

#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>



TEST(cps_api_db,init) {
    std::vector<cps_api_attr_id_t> ids ;

    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);
}

TEST(cps_api_db,db_ping) {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

    ASSERT_TRUE(cps_db::ping(b.get()));
}


TEST(cps_api_db,db_key) {
    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);

    std::vector<char> _buff;
    cps_db::dbkey_from_class_key(_buff,cps_api_object_key(obj));
    std::cout << "DB key data " << cps_string::tostring(&_buff[0],_buff.size());

    _buff.clear();
    cps_db::dbkey_from_instance_key(_buff,(obj));
    std::cout << "DB instance data " << cps_string::tostring(&_buff[0],_buff.size());

    _buff.clear();
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_IFINDEX,1);
    cps_db::dbkey_from_instance_key(_buff,obj);
    std::cout << "DB instance data " << cps_string::tostring(&_buff[0],_buff.size());
}

TEST(cps_api_db,db_inc_cntr) {

    cps_db::connection con;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    std::vector<char> _buff;
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_IFINDEX,1);

    cps_db::dbkey_from_instance_key(_buff,obj);
    std::cout << "DB instance data " << cps_string::tostring(&_buff[0],_buff.size());

    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0",false));


    ssize_t cntr=0;

    cps_db::delete_object(con,_buff);

    for ( size_t ix = 0; ix < 10 ; ++ix ) {
        cps_db::get_sequence(con,_buff,cntr);
        ASSERT_TRUE(cntr==((ssize_t)ix+1));
    }
    cps_db::delete_object(con,_buff);
}

TEST(cps_api_db,db_set_obj) {
    cps_db::connection con;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    std::vector<char> _buff;
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_IFINDEX,1);
    cps_api_object_attr_add(obj,BASE_IP_IPV6_NAME,"Clifford",9);

    cps_api_object_t clone = cps_api_object_create();
    cps_api_object_guard og2(clone);
    cps_api_object_clone(clone,obj);

    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0",false));

    cps_db::delete_object(con,obj);

    ASSERT_TRUE(cps_db::store_object(con,obj));
    ASSERT_TRUE(cps_db::get_object(con,clone));
    ASSERT_TRUE(memcmp(cps_api_object_array(obj),cps_api_object_array(clone),cps_api_object_to_array_len(clone))==0);

    cps_db::delete_object(con,obj);
    ASSERT_FALSE(cps_db::get_object(con,clone));
}

TEST(cps_api_db,db_set_list_obj) {
    cps_db::connection con;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    cps_api_object_list_t list =cps_api_object_list_create();
    cps_api_object_list_guard lg(list);

    std::vector<char> _buff;
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);

    size_t ix = 0;
    static const size_t mx = 1000;
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_create_obj_and_append(list);
        cps_api_object_clone(o,obj);
        cps_api_object_attr_add_u32(o,BASE_IP_IPV6_IFINDEX,ix);
        std::string _s = cps_string::sprintf("Cliff%d",(int)ix);
        cps_api_object_attr_add(obj,BASE_IP_IPV6_NAME,_s.c_str(),_s.size()+1);
    }

    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    ASSERT_TRUE(b.valid());


    ASSERT_TRUE(cps_db::delete_objects(b.get(),list));

    ASSERT_TRUE(cps_db::store_objects(b.get(),list));

    cps_api_object_list_t newl=cps_api_object_list_create();
    cps_api_object_list_guard lg2(newl);

    ASSERT_TRUE(cps_db::get_objects(b.get(),obj,newl));

    ASSERT_EQ(cps_api_object_list_size(newl),cps_api_object_list_size(list));
}

TEST(cps_api_db,db_node_list) {

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    cps_api_object_list_t list =cps_api_object_list_create();
    cps_api_object_list_guard lg(list);

    cps_api_node_ident ids[2] = { {"NODE1", "127.0.0.1:6379"}, {"NODE2","172.17.0.6:6380"} };
    cps_api_node_group_t _g;

    _g.id = "A";
    _g.addrs = ids;
    _g.addr_len = 2;
    _g.data_type = cps_api_node_data_NODAL;

    cps_api_set_node_group(&_g);

    cps_api_nodes nodes;
    ASSERT_TRUE(nodes.load());

    ASSERT_TRUE(nodes.part_of("A","127.0.0.1:6379"));
    ASSERT_TRUE(nodes.part_of("A","172.17.0.6:6380"));
    ASSERT_TRUE(!nodes.part_of("A","372.17.0.6:6379"));
}

TEST(cps_api_db,db_node_alias) {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

    const char *lst[]={"cliff","this","local" };
    ASSERT_TRUE(cps_api_set_identity(DEFAULT_REDIS_ADDR,lst,sizeof(lst)/sizeof(*lst))==cps_api_ret_code_OK);

    cps_api_nodes n;
    ASSERT_TRUE(n.load());

    for (size_t ix = 0, mx = sizeof(lst)/sizeof(*lst) ; ix < mx ; ++ix ) {
        const char *addr = n.addr(lst[ix]);
        ASSERT_TRUE(strcmp(addr,DEFAULT_REDIS_ADDR)==0);
    }
}

TEST(cps_api_db,db_node_cleanup) {
    cps_api_object_guard og(cps_api_object_create());
    ASSERT_TRUE(og.get()!=nullptr);
    cps_api_object_t obj = og.get();
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_key_set_group(obj,"A");
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_DELETE, obj, 0, 200)==cps_api_ret_code_OK);
}

cps_api_object_guard _tempate(cps_api_object_create());

TEST(cps_api_db,db_node_create) {
    //create one element
    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(_tempate.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET));
    cps_api_object_attr_add_u32(_tempate.get(),BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(_tempate.get(),BASE_IP_IPV6_IFINDEX,1);
    cps_api_object_attr_add(_tempate.get(),BASE_IP_IPV6_NAME,"Clifford",9);
    cps_api_object_attr_add(_tempate.get(),0xc11ff,"Clifford",9);
    cps_api_key_set_group(_tempate.get(),"A");
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, _tempate.get(), 0, 200)==cps_api_ret_code_OK);
}

TEST(cps_api_db,db_node_set) {
    cps_api_object_guard og(cps_api_object_create());

    cps_api_object_clone(og.get(),_tempate.get());
    std::string val = "Clifford Wichmann";
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,val.c_str(),val.size());

    cps_api_object_attr_add(og.get(),3,val.c_str(),val.size());

    cps_api_object_attr_delete(_tempate.get(),BASE_IP_IPV6_NAME);
    cps_api_object_attr_add(_tempate.get(),BASE_IP_IPV6_NAME,val.c_str(),val.size());
    cps_api_object_attr_add(_tempate.get(),3,val.c_str(),val.size());

    cps_api_key_set_group(og.get(),"A");
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_SET, og.get(), 0, 200)==cps_api_ret_code_OK);
}
TEST(cps_api_db,db_node_get) {
    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET));
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,1);
    cps_api_key_set_group(og.get(),"A");

    ASSERT_TRUE(cps_api_get_objs(og.get(),lg.get(), 0, 200)==cps_api_ret_code_OK);
    {
        size_t ix = 0;
        size_t mx = cps_api_object_list_size(lg.get());
        for ( ; ix < mx ; ++ix ) {
            cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
            cps_api_object_attr_delete(o,CPS_OBJECT_GROUP_NODE);

            if (!cps_api_obj_tool_matches_filter(_tempate.get(),o,true)) {
                char buff[1024];
                printf("Object A: %s\n",cps_api_object_to_string(o,buff,sizeof(buff)));
                printf("Object B: %s\n",cps_api_object_to_string(og.get(),buff,sizeof(buff)));
                ASSERT_TRUE(false);
            }
        }
    }
}

TEST(cps_api_db,db_nodes) {
    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    const char *_name = "----";
    //create another element
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,3);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_name,strlen(_name)+1);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, og.get(), 0, 200)==cps_api_ret_code_OK);


    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);
    _name = "Shankara/Jana/Joe/Joe";

    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_name,strlen(_name)+1);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, og.get(), 0, 200)==cps_api_ret_code_OK);

    lg.set(cps_api_object_list_create());
    og.set(cps_api_object_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,0);    //filter all objects to get the VRF 0 only
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);    //filter all objects to get the IFINDEX 1 only
    cps_api_key_set_group(og.get(),"A");    //get from this node only

    ASSERT_TRUE(cps_api_get_objs(og.get(), lg.get(),0,100)==cps_api_ret_code_OK);

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,"Everyone",strlen("Everyone")+1);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_SET, og.get(), 0, 200)==cps_api_ret_code_OK);


    og.free();
    og.set(cps_api_object_create());
    lg.set(cps_api_object_list_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_key_set_group(og.get(),"A");
    ASSERT_TRUE(cps_api_get_objs(og.get(), lg.get(),0,100)==cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_object_list_size(lg.get()),2);

    lg.set(cps_api_object_list_create());
    og.free();
    og.set(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,0);    //filter all objects to get the VRF 0 only
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,1);    //filter all objects to get the IFINDEX 1 only
    cps_api_key_set_group(og.get(),"A");    //get from this node only

    ASSERT_TRUE(cps_api_get_objs(og.get(), lg.get(),0,100)==cps_api_ret_code_OK);

    ASSERT_TRUE(cps_api_object_list_size(lg.get())==2);

    og.set(cps_api_object_create());
    ASSERT_TRUE(og.get()!=nullptr);
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_DELETE, og.get(), 0, 200)==cps_api_ret_code_OK);

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
