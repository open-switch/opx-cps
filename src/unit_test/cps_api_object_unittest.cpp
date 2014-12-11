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


TEST(cps_api_object,create) {
    cps_api_object_list_t list = cps_api_object_list_create();

    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));
    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));
    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));
    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));
    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));
    ASSERT_TRUE(cps_api_object_list_append(list, create_list()));

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(list);
    for (; ix < mx; ++ix) {
        cps_api_object_t o = (cps_api_object_list_get(list, ix));
        ASSERT_TRUE(o!=NULL);
        print_obj(o);

        size_t al = cps_api_object_to_array_len(o);
        void * p = malloc(al);
        memcpy(p, cps_api_object_array(o),al);

        ASSERT_TRUE((memcmp(p, cps_api_object_array(o), al)== 0));

        cps_api_object_t obj = CPS_API_OBJ_ALLOC;
        if (cps_api_array_to_object(p, al, obj)) {
            ASSERT_TRUE((memcmp(cps_api_object_array(obj), cps_api_object_array(o), al)==0));
            print_obj(obj);
            cps_api_object_delete(obj);
        }
        free(p);
    }

    cps_api_object_list_destroy(list, true);
}

TEST(cps_api_object,ram_based) {
    char buff[1024];
    cps_api_object_t obj = cps_api_object_init(buff,sizeof(buff));
    ASSERT_TRUE(cps_api_object_attr_add(obj,0,"Cliff",6));
    ASSERT_TRUE(cps_api_object_attr_add_u16(obj,1,16));
    ASSERT_TRUE(cps_api_object_attr_add_u32(obj,2,32));
    ASSERT_TRUE(cps_api_object_attr_add_u64(obj,3,64));

    cps_api_object_attr_t it = cps_api_object_attr_start(obj);

    for ( ; it != CPS_API_ATTR_NULL ; ) {
        int id = (int) cps_api_object_attr_id(it);
        switch (id) {
        case 0 : print_attr(it); break;
        case 1 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it),
                (int)cps_api_object_attr_data_u16(it));
            ASSERT_TRUE(cps_api_object_attr_data_u16(it)==16);
            break;
        case 2 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it),
                (int)cps_api_object_attr_data_u32(it));
            ASSERT_TRUE(cps_api_object_attr_data_u32(it)==32);
            break;
        case 3 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it),
                (int)cps_api_object_attr_data_u64(it));
            ASSERT_TRUE(cps_api_object_attr_data_u64(it)==64);
            break;
        }
        cps_api_object_attr_delete(obj,cps_api_object_attr_id(it));

        it = cps_api_object_attr_start(obj);
    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
