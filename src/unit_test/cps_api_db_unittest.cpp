
#include "cps_api_events.h"


#include <stdio.h>
#include <stdlib.h>


#include "gtest/gtest.h"

#include "cps_api_node.h"
#include "cps_api_node_private.h"

#include "cps_api_db.h"
#include "cps_string_utils.h"

#include "cps_class_map.h"
#include "cps_api_events.h"
#include "cps_api_event_init.h"
#include "std_event_service.h"

#include "cps_api_service.h"

#include <pthread.h>
#include <sys/select.h>

#include <vector>
#include "cps_class_ut_data.h"

TEST(cps_api_db,init) {
    std::vector<cps_api_attr_id_t> ids ;

    size_t ix = 0;
    for ( ; ix < lst_len ; ++ix ) {
        cps_class_map_init(lst[ix].id,&(lst[ix]._ids[0]),lst[ix]._ids.size(),&lst[ix].details);
    }
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
	size_t mx = 100;
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

	ASSERT_TRUE(cps_api_object_list_size(newl)==cps_api_object_list_size(list));
}

TEST(cps_api_db,db_node_list) {

	cps_api_object_t obj = cps_api_object_create();
	cps_api_object_guard og(obj);

	cps_api_object_list_t list =cps_api_object_list_create();
	cps_api_object_list_guard lg(list);

	cps_api_node_group_t _g;
	const char *names[] = { "127.0.0.1:6379" , "172.17.0.6:6379" };

	_g.id = "A";
	_g.addresses = names;
	_g.addr_len = 2;
	_g.data_type = cps_api_node_data_NODAL;

	cps_api_create_node_group(&_g);

	cps_api_nodes nodes;
	ASSERT_TRUE(nodes.load());

	ASSERT_TRUE(nodes.part_of("A","127.0.0.1:6379"));
	ASSERT_TRUE(nodes.part_of("A","172.17.0.6:6379"));
	ASSERT_TRUE(!nodes.part_of("A","372.17.0.6:6379"));
}

TEST(cps_api_db,db_node_alias) {
	cps_db::connection_request b(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);

	const char *lst[]={"cliff","this","local" };
	ASSERT_TRUE(cps_api_set_identity(DEFAULT_REDIS_ADDR,lst,sizeof(lst)/sizeof(*lst))==cps_api_ret_code_OK);

	cps_api_node_alias a;
	a.load();

	for (size_t ix = 0, mx = sizeof(lst)/sizeof(*lst) ; ix < mx ; ++ix ) {
		const char *addr = a.addr(lst[ix]);
		ASSERT_TRUE(strcmp(addr,DEFAULT_REDIS_ADDR)==0);
	}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
