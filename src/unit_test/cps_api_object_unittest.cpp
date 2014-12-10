/* OPENSOURCELICENSE */
/*
 * cps_api_object_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */



#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "gtest/gtest.h"
#include "cps_api_object.h"


#include "std_tlv.h"
#include "cps_api_object.h"
#include <time.h>

void print_attr(cps_api_object_attr_t it) {
	if (it == NULL) return;
	uint64_t attr = cps_api_object_attr_id(it);
	uint64_t len = cps_api_object_attr_len(it);
	void * data = cps_api_object_attr_data_bin(it);

	printf("attr: %d, len %d, val %s\n", (int)attr, (int)len, (char*)data);
}


cps_api_object_t create_list() {
	cps_api_object_t obj = CPS_API_OBJ_ALLOC;

	size_t ix = 0;
	size_t mx = 10;

	for (; ix < mx; ++ix) {
		time_t tm = time(NULL);
		cps_api_object_attr_add(obj, ix + (tm & 0xf) , (void*)"cliff", 6);
	}

	cps_api_object_attr_t it = cps_api_object_attr_start(obj);

	while (it != CPS_API_ATTR_NULL) {
		if (std_tlv_tag(it) & 1) {
			cps_api_object_attr_delete(obj, std_tlv_tag(it));
			it = cps_api_object_attr_start(obj);
		}
		it = cps_api_object_attr_next(obj, it);
	}

	return obj;
}

void print_obj(cps_api_object_t obj) {
	cps_api_object_attr_t it = cps_api_object_attr_start(obj);

	while (it != CPS_API_ATTR_NULL) {
		print_attr(it);
		it = cps_api_object_attr_next(obj, it);
		if (it == NULL) {
			printf("Null\n");
		}
	}
}
#include <stdlib.h>


void test_object() {
	cps_api_object_list_t list = cps_api_object_list_create();

	cps_api_object_list_append(list, create_list());
	cps_api_object_list_append(list, create_list());
	cps_api_object_list_append(list, create_list());
	cps_api_object_list_append(list, create_list());
	cps_api_object_list_append(list, create_list());
	cps_api_object_list_append(list, create_list());

	size_t ix = 0;
	size_t mx = cps_api_object_list_size(list);
	for (; ix < mx; ++ix) {
		cps_api_object_t o = (cps_api_object_list_get(list, ix));

		print_obj(o);

		size_t al = cps_api_object_to_array_len(o);
		void * p = malloc(al);
		memcpy(p, cps_api_object_array(o),al);
		if (memcmp(p, cps_api_object_array(o), al) != 0) {
			exit(1);
		}
		cps_api_object_t obj = CPS_API_OBJ_ALLOC;
		if (cps_api_array_to_object(p, al, obj)) {
			if (memcmp(cps_api_object_array(obj), cps_api_object_array(o), al) != 0) {
				exit(1);
			}

			print_obj(obj);
			cps_api_object_delete(obj);
		}
		free(p);

	}

	cps_api_object_list_destroy(list, true);
}

TEST(cps_api_key,key_create) {
	test_object();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
