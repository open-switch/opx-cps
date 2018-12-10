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


#include "cps_api_core_utils.h"
#include "cps_class_map.h"
#include "cps_api_db.h"
#include "cps_string_utils.h"

#include "cps_api_db_interface.h"
#include "cps_api_object_tools.h"

#include "cps_api_db_connection_tools.h"

#include "cps_api_operation_tools.h"
#include "cps_dictionary.h"

#include "cps_api_node.h"
#include "cps_class_ut_data.h"
#include "dell-cps.h"

#include "gtest/gtest.h"

#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>


/**
 * Test DB and DB Direct calls
 */
TEST(cps_api_db,init) {
    std::vector<cps_api_attr_id_t> ids ;

    __init_class_map();
    cps_api_object_guard og(cps_api_object_create());
    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6_ADDRESS,cps_api_qualifier_TARGET);
    cps_api_obj_set_ownership_type(cps_api_object_key(og.get()),CPS_API_OBJECT_DB);
}

/************************************************************************************
 * Internal DB Infra tests...
 *
 */
TEST(cps_api_db,internal_db_validate) {
    cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    ASSERT_TRUE(b.valid());
    ASSERT_TRUE(cps_db::ping(b.get()));
}

TEST(cps_api_db,internal_db_inc_cntr) {

    cps_db::connection con;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    std::vector<char> _buff;
    cps_api_key_from_attr_with_qual(cps_api_object_key(obj),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(obj,BASE_IP_IPV6_IFINDEX,1);

    cps_db::dbkey_from_instance_key(_buff,obj,false);
    std::cout << "DB instance data " << cps_string::tostring(&_buff[0],_buff.size());

    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));

    ssize_t cntr=0;

    cps_db::delete_object(con,_buff);

    for ( size_t ix = 0; ix < 10 ; ++ix ) {
        cps_db::get_sequence(con,_buff,cntr);
        ASSERT_TRUE(cntr==((ssize_t)ix+1));
    }
    cps_db::delete_object(con,_buff);
}

TEST(cps_api_db,internal_db_set_obj) {
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

    cps_api_object_attr_add_u32(clone,111,0);

    ASSERT_TRUE(con.connect(DEFAULT_REDIS_ADDR,"0"));

    printf("CPS Objects... \n");
    cps_api_object_print(obj);
    cps_api_object_print(clone);

    cps_db::delete_object(con,obj);

    ASSERT_TRUE(cps_db::store_object(con,obj));

    ASSERT_TRUE(cps_db::get_object(con,clone));

    //clone and obj must be the same now... so can't be original one
    ASSERT_TRUE(memcmp(cps_api_object_array(obj),cps_api_object_array(clone),cps_api_object_to_array_len(clone))==0);

    cps_db::delete_object(con,obj);
    ASSERT_FALSE(cps_db::get_object(con,clone));
}

TEST(cps_api_db,internal_db_set_list_obj) {
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


    ASSERT_TRUE(cps_db::delete_object_list(b.get(),list));

    ASSERT_TRUE(cps_db::store_objects(b.get(),list));

    cps_api_object_list_t newl=cps_api_object_list_create();
    cps_api_object_list_guard lg2(newl);

    ASSERT_TRUE(cps_db::get_objects(b.get(),obj,newl));

    ASSERT_EQ(cps_api_object_list_size(newl),cps_api_object_list_size(list));
}

/************************************************************************************
 * CPS DB API validation...
 *
 */
TEST(cps_api_db,cps_db_api) {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6_ADDRESS,true));
    ASSERT_TRUE(og.get()!=nullptr);


    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,1);
    ASSERT_TRUE(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),nullptr,true)==cps_api_ret_code_OK);

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);
    ASSERT_TRUE(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),nullptr,true)==cps_api_ret_code_OK);

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),cps_api_oper_SET);
    ASSERT_TRUE(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),nullptr,true)==cps_api_ret_code_OK);

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),cps_api_oper_DELETE);
    ASSERT_TRUE(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),nullptr,true)==cps_api_ret_code_OK);

}

TEST(cps_api_db,cps_db_standard_api_test) {
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6_ADDRESS,true));
    ASSERT_TRUE(og.get()!=nullptr);


    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,0);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,1);

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,100)==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_DELETE,og.get(),1,100)==cps_api_ret_code_OK);

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,100)==cps_api_ret_code_OK);

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),cps_api_oper_SET);

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_SET,og.get(),1,100)==cps_api_ret_code_OK);

    ASSERT_TRUE(cps_api_object_exact_match(og.get(),true));
    cps_api_object_list_guard lg(cps_api_object_list_create());

    ASSERT_EQ(cps_api_get_objs(og.get(),lg.get(),2,100),cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_object_list_size(lg.get()),1);

    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_DELETE,og.get(),1,100)==cps_api_ret_code_OK);
}


size_t get_object_count(cps_api_object_t obj) {
    cps_api_object_guard _og(cps_api_object_create());
    cps_api_object_set_key(_og.get(),cps_api_object_key(obj));

    cps_api_object_list_guard lst(cps_api_object_list_create());
    if (cps_api_db_get(_og.get(),lst.get())==cps_api_ret_code_OK) {
        return cps_api_object_list_size(lst.get());
    }
    return 0;
}

size_t get_instance_count(cps_api_object_t obj) {
    cps_api_object_list_guard lst(cps_api_object_list_create());
    if (cps_api_db_get(obj,lst.get())==cps_api_ret_code_OK) {
        return cps_api_object_list_size(lst.get());
    }
    return 0;
}

bool delete_object_instance(cps_api_object_t obj) {
    printf("Cleaning... %d %s objects... \n",(int)get_instance_count(obj),cps_api_object_to_c_string(obj).c_str());
    cps_api_return_code_t _rc = cps_api_db_commit_one(cps_api_oper_DELETE,obj,nullptr,false);
    if (_rc!=cps_api_ret_code_OK) {
        printf("Return code from delete is %d\n",_rc);
        return false;
    }

    return (get_object_count(obj)==0);
}

bool test_reset() {
    cps_api_object_list_guard lg(cps_api_object_list_create());

    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,true));
    if (!delete_object_instance(og.get())) return false;

    og.set(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV4,true));
    return delete_object_instance(og.get());
}

TEST(cps_api_db,objects_with_wildcard_chars_in_key) {
    test_reset();

    cps_api_object_guard         og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));
    cps_api_object_list_guard     lg(cps_api_object_list_create());

    delete_object_instance(og.get());
    delete_object_instance(og.get());

    //add 2 key attributes with elements that have wildcard chars
    std::string s;
    size_t c = 0;
    const static size_t _max = UINT8_MAX;
    for ( ; c < _max ; ++c ) {
        s+=c;
        s+=".";
    }
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_IFINDEX,s.c_str(),s.size()+1);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_VRF_ID,s.c_str(),s.size()+1);

    cps_api_object_guard clone(cps_api_object_create());
    cps_api_object_clone(clone.get(),og.get());

    cps_api_core_publish(og.get());

    cps_api_object_guard _prev(cps_api_object_create());
    ASSERT_TRUE(_prev.get()!=nullptr);

    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),_prev.get(),true),cps_api_ret_code_OK);

    ASSERT_EQ(get_object_count(og.get()),1);

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    ASSERT_EQ(get_object_count(og.get()),1);

    for ( size_t ix = 0; ix < 1000 ; ++ix ) {
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,ix);
        ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),_prev.get(),false),cps_api_ret_code_OK);
    }

    lg.set(cps_api_object_list_create());

    ASSERT_EQ(cps_api_db_get(og.get(),lg.get()),cps_api_ret_code_OK);

    delete_object_instance(og.get());
    ASSERT_TRUE(get_instance_count(og.get())==0);

    std::vector<char> _lst;
    cps_db::db_key_copy_with_escape(_lst,s.c_str(),s.size());

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_filter_wildcard_attrs(og.get(),true);

    std::string _data;

    //reduce the list to just the first 100
    if (_lst.size()>100) {
    	_lst.resize(100);
    }
    while (_lst.size()> 1) {
        (void)_lst.pop_back(); //zap last
        if (_lst[_lst.size()-1]=='\\') {
        	continue;
        }
        _lst.push_back('*');
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
        cps_api_object_attr_add(og.get(),BASE_IP_IPV6_VRF_ID,&_lst[0],_lst.size());
        if (_lst.size()<100) {	//
            size_t _cnt = get_instance_count(og.get());
            if (_cnt>0) {
                printf("Found %d matching objects\n",(int)_cnt);
            }
        }
        (void)_lst.pop_back();	//remove the *
    }

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
    delete_object_instance(og.get());
}


TEST(cps_api_db,direct_delete) {
    cps_api_object_list_guard lg(cps_api_object_list_clone(Get1000(),true));
    ASSERT_TRUE(get_object_count(cps_api_object_list_get(lg.get(),0))==0);

    cps_api_db_commit_bulk_t bulk;
    bulk.objects = lg.get();
    bulk.op =cps_api_oper_CREATE;
    bulk.node_group = nullptr;
    bulk.publish = false;

    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);

    size_t _len = get_object_count(cps_api_object_list_get(Get1000(),0));

    ASSERT_EQ(_len,cps_api_object_list_size(Get1000()));

    bulk.op =cps_api_oper_DELETE;
    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);

    ASSERT_EQ(get_object_count(cps_api_object_list_get(lg.get(),0)),0);
}

TEST(cps_api_db,stanard_error_codes_set) {
    //cps_api_db_get cps_api_db_commit_one cps_api_db_commit_bulk

    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    cps_api_object_t obj = cps_api_object_list_get(Get100(),0);

    //set up a few objects (current and make a prev)
    cps_api_object_clone(og.get(),obj);
    cps_api_object_guard prev(cps_api_object_create());


    //with the operation type being create, do a commit
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_DELETE,og.get(),prev.get(),true),cps_api_ret_code_OK);

    //with the operation type being create, do a commit
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),prev.get(),true),cps_api_ret_code_OK);
    //prove that we can set one when there is no data

    ASSERT_EQ(cps_api_db_get(og.get(),lg.get()),cps_api_ret_code_OK);
    ASSERT_EQ(get_instance_count(og.get()),1);

    cps_api_object_print(cps_api_object_list_get(lg.get(),0));

    //with the operation type being create, do a commit
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_DELETE,og.get(),prev.get(),true),cps_api_ret_code_OK);

    //with the operation type being create, do a commit
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,og.get(),prev.get(),true),cps_api_ret_code_OK);
    //prove that we can set one when there is no data

    ASSERT_EQ(cps_api_db_get(og.get(),lg.get()),cps_api_ret_code_OK);
    ASSERT_EQ(get_instance_count(og.get()),1);

    cps_api_object_print(cps_api_object_list_get(lg.get(),0));


    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_DELETE,og.get(),prev.get(),true),cps_api_ret_code_OK);

    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS,10);
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,og.get(),prev.get(),true),cps_api_ret_code_OK);

    ASSERT_TRUE(cps_api_object_get_data(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);
    ASSERT_TRUE(cps_api_object_get_data(prev.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)==nullptr);

    cps_api_object_list_guard _found_list(cps_api_object_list_create());
    ASSERT_EQ(cps_api_db_get(og.get(),_found_list.get()),cps_api_ret_code_OK);

    ASSERT_EQ(cps_api_object_list_size(_found_list.get()),1);
    cps_api_object_t _updated = cps_api_object_list_get(_found_list.get(),0);
    printf("Updated object in the db\n");
    cps_api_object_print(_updated);

    ASSERT_TRUE(cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);
    ASSERT_EQ(*(uint32_t*)cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS),10);


    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS);
    cps_api_object_print(og.get());

    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS,nullptr,0);
    printf("Current after add\n");
    cps_api_object_print(og.get());

    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,og.get(),prev.get(),true),cps_api_ret_code_OK);

    cps_api_object_list_clear(_found_list.get(),true);

    ASSERT_EQ(cps_api_db_get(og.get(),_found_list.get()),cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_object_list_size(_found_list.get()),1);

    _updated = cps_api_object_list_get(_found_list.get(),0);
    ASSERT_TRUE(cps_api_object_get_data(prev.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);

    ASSERT_TRUE(cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)==nullptr);
    printf("Updated value\n");
    cps_api_object_print(_updated);


    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,prev.get(),nullptr,true),cps_api_ret_code_OK);

    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_DELETE,prev.get(),nullptr,true),cps_api_ret_code_OK);

}


TEST(cps_api_db,simple) {
    //cps_api_db_get cps_api_db_commit_one cps_api_db_commit_bulk

    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    cps_api_object_t obj = cps_api_object_list_get(Get100(),0);

    //set up a few objects (current and make a prev)
    cps_api_object_clone(og.get(),obj);
    cps_api_object_guard prev(cps_api_object_create());

    //with the operation type being create, do a commit
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_CREATE,og.get(),prev.get(),true),cps_api_ret_code_OK);

    ASSERT_EQ(get_instance_count(og.get()),1);

    auto _disp = [&]() {
        printf("Current\n");
        cps_api_object_print(og.get());
        printf("Prev\n");
        cps_api_object_print(prev.get());
    };

    _disp();

    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS,10);
    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,og.get(),prev.get(),true),cps_api_ret_code_OK);

    ASSERT_TRUE(cps_api_object_get_data(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);
    ASSERT_TRUE(cps_api_object_get_data(prev.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)==nullptr);

    cps_api_object_list_guard _found_list(cps_api_object_list_create());
    ASSERT_EQ(cps_api_db_get(og.get(),_found_list.get()),cps_api_ret_code_OK);

    ASSERT_EQ(cps_api_object_list_size(_found_list.get()),1);
    cps_api_object_t _updated = cps_api_object_list_get(_found_list.get(),0);
    printf("Updated object in the db\n");
    cps_api_object_print(_updated);

    ASSERT_TRUE(cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);
    ASSERT_EQ(*(uint32_t*)cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS),10);

    _disp();
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS);
    printf("Current after delete\n");
    cps_api_object_print(og.get());

    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS,nullptr,0);
    printf("Current after add\n");
    cps_api_object_print(og.get());

    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,og.get(),prev.get(),true),cps_api_ret_code_OK);

    cps_api_object_list_clear(_found_list.get(),true);

    ASSERT_EQ(cps_api_db_get(og.get(),_found_list.get()),cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_object_list_size(_found_list.get()),1);

    _updated = cps_api_object_list_get(_found_list.get(),0);
    ASSERT_TRUE(cps_api_object_get_data(prev.get(),BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)!=nullptr);

    ASSERT_TRUE(cps_api_object_get_data(_updated,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS)==nullptr);
    printf("Updated value\n");
    cps_api_object_print(_updated);


    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_SET,prev.get(),nullptr,true),cps_api_ret_code_OK);

    ASSERT_EQ(cps_api_db_commit_one(cps_api_oper_DELETE,prev.get(),nullptr,true),cps_api_ret_code_OK);

}

TEST(cps_api_db,cleanup_1) {
    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));
    ASSERT_EQ(cps_api_db_get(og.get(),lg.get()),cps_api_ret_code_OK);

    cps_api_db_commit_bulk_t bulk;
    bulk.objects = lg.get();
    bulk.op =cps_api_oper_DELETE;
    bulk.node_group = nullptr;
    bulk.publish = false;

    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);
}

TEST(cps_api_db,test_bulk_10000_create_set_delete) {

    cps_api_object_list_guard lg(cps_api_object_list_clone(Get10000(),true));

    cps_api_db_commit_bulk_t bulk;
    bulk.objects = lg.get();
    bulk.node_group = nullptr;
    bulk.publish = false;
    bulk.op = cps_api_oper_CREATE;
    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);

    for (size_t ix = 0, mx = cps_api_object_list_size(lg.get()); ix  < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
        cps_api_object_attr_add_u32(o,BASE_IP_IPV6_DUP_ADDR_DETECT_TRANSMITS,ix);
    }

    bulk.op = cps_api_oper_SET;
    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);

    bulk.op = cps_api_oper_DELETE;
    ASSERT_EQ(cps_api_db_commit_bulk(&bulk),cps_api_ret_code_OK);
}

/************************************************************************************
 * CPS API with DB backend validation...
 *
 */

TEST(cps_api_db,db_general_test) {
    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    std::string _name = "----";

    ASSERT_TRUE(cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET));
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,1);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_name.c_str(),_name.size()+1);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, og.get(), 0, 200)==cps_api_ret_code_OK);

    //replace attributes
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_NAME);

    _name = "Shankara/Jana/Joe/Joe";
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,3);
    cps_api_object_attr_add(og.get(),BASE_IP_IPV6_NAME,_name.c_str(),_name.size()+1);
    ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, og.get(), 0, 200)==cps_api_ret_code_OK);

    lg.set(cps_api_object_list_create());
    og.set(cps_api_object_create());

    cps_api_key_from_attr_with_qual(cps_api_object_key(og.get()),BASE_IP_IPV6,cps_api_qualifier_TARGET);
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,1);    //filter all objects to get the VRF 0 only
    cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,2);    //filter all objects to get the IFINDEX 1 only

    ASSERT_TRUE(cps_api_get_objs(og.get(), lg.get(),0,100)==cps_api_ret_code_OK);
    ASSERT_TRUE(cps_api_object_list_size(lg.get())>0);

    for ( size_t ix = 0, mx = cps_api_object_list_size(lg.get()) ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_get(lg.get(),ix);
        uint32_t *_if = static_cast<uint32_t*>(cps_api_object_get_data(o,BASE_IP_IPV6_IFINDEX));
        ASSERT_EQ(*_if,2);

        uint32_t *_vrf = static_cast<uint32_t*>(cps_api_object_get_data(o,BASE_IP_IPV6_VRF_ID));
        ASSERT_EQ(*_vrf,1);
    }
}

size_t cps_general_count(cps_api_object_t obj) {
    cps_api_object_guard _og(cps_api_object_create());
    cps_api_object_set_key(_og.get(),cps_api_object_key(obj));

    cps_api_object_list_guard lst(cps_api_object_list_create());
    if (cps_api_get_objs(_og.get(),lst.get(),0,100)==cps_api_ret_code_OK) {
        return cps_api_object_list_size(lst.get());
    }
    return 0;
}

size_t get_general_instance_count(cps_api_object_t obj) {
    cps_api_object_list_guard lst(cps_api_object_list_create());
    if (cps_api_get_objs(obj,lst.get(),0,100)==cps_api_ret_code_OK) {
        return cps_api_object_list_size(lst.get());
    }
    return 0;
}

TEST(cps_api_db,cps_general_db_backend) {
    test_reset();

    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_TARGET,BASE_IP_IPV6,false));

    std::string _name = "----";

    size_t ix = 0;
    size_t mx = 1000;

    for ( ; ix < mx ; ++ix ) {
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
        cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);

        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_VRF_ID,1);
        cps_api_object_attr_add_u32(og.get(),BASE_IP_IPV6_IFINDEX,ix);

        ASSERT_TRUE(cps_api_commit_one(cps_api_oper_CREATE, og.get(), 1, 0)==cps_api_ret_code_OK);
    }
    ASSERT_EQ(cps_general_count(og.get()),ix);

    ix = 0;
    for ( ; ix < mx ; ++ix ){
        if (ix!=0) cps_api_object_attr_delete(og.get(),ix-1);
        cps_api_object_attr_add_u32(og.get(),ix,ix);
        ASSERT_TRUE(cps_api_commit_one(cps_api_oper_SET, og.get(), 0, 200)==cps_api_ret_code_OK);
    }

    cps_api_object_list_clear(lg.get(),true);

    ASSERT_EQ(cps_api_get_objs(og.get(),lg.get(),1,0),cps_api_ret_code_OK);
    ASSERT_EQ(cps_api_object_list_size(lg.get()),1);
    cps_api_object_t obj = cps_api_object_list_get(lg.get(),0);
    ix = 0;
    for ( ; ix < mx ; ++ix ) {
        ASSERT_TRUE(cps_api_object_get_data(obj,ix)!=nullptr);
        ASSERT_EQ(*(uint32_t*)cps_api_object_get_data(obj,ix),ix);
    }

    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_VRF_ID);
    cps_api_object_attr_delete(og.get(),BASE_IP_IPV6_IFINDEX);

    ASSERT_EQ(cps_api_commit_one(cps_api_oper_DELETE, og.get(), 1, 0),cps_api_ret_code_OK);
    ASSERT_EQ(cps_general_count(og.get()),0);
}

TEST(cps_api_db,ignore_comm_errors) {

    cps_api_object_list_guard lg(cps_api_object_list_create());
    cps_api_object_guard og(cps_api_object_create());

    cps_api_object_t obj = cps_api_object_list_get(Get100(),0);

    cps_api_object_clone(og.get(),obj);
    cps_api_object_guard prev(cps_api_object_create());

    cps_api_key_set_group(og.get(),"127.0.0.1:6377");

    auto _start = std_get_uptime(nullptr);
    cps_api_get_objs(og.get(),lg.get(),1,0);
    auto _end = std_get_uptime(nullptr);
    ASSERT_TRUE((_end-_start)< 100*1000*3);

    _start = std_get_uptime(nullptr);
    cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,0);
    _end = std_get_uptime(nullptr);
    ASSERT_TRUE((_end-_start)< 100*1000*3);

    //"cps.db.ignore-comm-failure"
    //"cps.db.comm-failure-retry-delay"
    //"cps.db.comm-failure-retry-delay-retry"

    cps_api_set_library_flags("cps.db.ignore-comm-failure","1");
    cps_api_set_library_flags("cps.db.comm-failure-retry-delay","500");
    cps_api_set_library_flags("cps.db.comm-failure-retry","2");

    _start = std_get_uptime(nullptr);
    cps_api_get_objs(og.get(),lg.get(),1,0);
    _end = std_get_uptime(nullptr);
    ASSERT_TRUE((_end-_start)> 1000*1000*1);

    _start = std_get_uptime(nullptr);
    cps_api_commit_one(cps_api_oper_CREATE,og.get(),1,0);
    _end = std_get_uptime(nullptr);
    ASSERT_TRUE((_end-_start)> 1000*1000*1);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
