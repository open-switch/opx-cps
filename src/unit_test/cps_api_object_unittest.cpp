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
    char buff[1024];
    printf("%s\n",cps_api_object_attr_to_string(it,buff,sizeof(buff)-1));

}


void walk_object(cps_api_object_it_t *it, int level, std::string path) {
    while (cps_api_object_it_valid(it)) {
        char buff[1024];
        printf("%d %s - %s\n",level,path.c_str(),cps_api_object_attr_to_string(it->attr,buff,sizeof(buff)-1));
        if (level<5) {
            std::string new_path = path;
            cps_api_object_it_t internal = *it;
            cps_api_object_it_inside(&internal);
            char b[100];
            snprintf(b,sizeof(b)-1,"%d",(int)cps_api_object_attr_id(it->attr));
            new_path+=".";
            new_path+=b;
            walk_object(&internal,level+1, new_path);
        }
        cps_api_object_it_next(it);
    }
}

cps_api_object_t create_list() {
    cps_api_object_t obj = cps_api_object_create();

    size_t ix = 0;
    size_t mx = 10;

    for (; ix < mx; ++ix) {
        time_t tm = time(NULL);
        cps_api_object_attr_add(obj, ix + (tm & 0xf) , (void*)"cliff", 6);
    }

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    while ( cps_api_object_it_valid(&it) ) {
        if (std_tlv_tag(it.attr) & 1) {
            cps_api_object_attr_delete(obj, std_tlv_tag(it.attr));
            cps_api_object_it_begin(obj,&it);
        }
        cps_api_object_it_next(&it);
    }
    cps_api_object_delete(obj);
    obj = cps_api_object_create();



    cps_api_attr_id_t lst[][6] = {
                                { 1,2,3,4,5,6 },
                                { 2,2,3,4,5,6 },
                                { 1,3,3,4,5,6 },
                                { 1,2,3,4,5,7 },
                                { 1,2,3,4,4,6 },
                                       { 3,2,3,4,5,6 }
    };
    const char * c = "clifford was here";

    ix = 0;
    mx = 6;
    for ( ; ix < mx ; ++ix ) {
        if (!cps_api_object_e_add(obj,lst[ix],6,cps_api_object_ATTR_T_BIN,
                c,strlen(c))) {
            return NULL;
        }
        uint16_t val=1;
        if (!cps_api_object_e_add(obj,lst[ix],6,cps_api_object_ATTR_T_U16,
                &val,sizeof(val))) {
            return NULL;
        }
        size_t i = 0;
        for ( ; i < ix ; ++i ) {
            cps_api_object_attr_t it = cps_api_object_e_get(obj,lst[i],6);

            if (it==NULL) return NULL;
        }
    }

    printf("Object walk\n");
    cps_api_object_it_t oit;
    cps_api_object_it_begin(obj,&oit);
    std::string s;

    walk_object(&oit,0,s);

    return obj;
}

void print_obj(cps_api_object_t obj) {
    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    while ( cps_api_object_it_valid(&it) ) {
        print_attr(it.attr);
        cps_api_object_it_next(&it);
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

        cps_api_object_t obj = cps_api_object_create();
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

    cps_api_object_attr_t list[4];
    size_t llen = sizeof(list)/sizeof(*list);
    cps_api_object_attr_fill_list(obj,0,list,llen);
    size_t ix = 0;
    size_t mx =llen ;
    size_t count = 0;
    for ( ; ix < mx ; ++ix ) {
        if (list[ix]!=NULL) ++count;
    }
    ASSERT_TRUE(count==4);

    cps_api_object_attr_fill_list(obj,2,list,llen); //show 2 and 3
    ix = 0;
    count = 0;
    for ( ; ix < mx ; ++ix ) {
        if (list[ix]!=NULL) ++count;
    }
    ASSERT_TRUE(count==2);


    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    while ( cps_api_object_it_valid(&it) ) {
        int id = (int) cps_api_object_attr_id(it.attr);
        switch (id) {
        case 0 : print_attr(it.attr); break;
        case 1 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it.attr),
                (int)cps_api_object_attr_data_u16(it.attr));
            ASSERT_TRUE(cps_api_object_attr_data_u16(it.attr)==16);
            break;
        case 2 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it.attr),
                (int)cps_api_object_attr_data_u32(it.attr));
            ASSERT_TRUE(cps_api_object_attr_data_u32(it.attr)==32);
            break;
        case 3 : printf("Attr %d, val %d\n",(int)cps_api_object_attr_id(it.attr),
                (int)cps_api_object_attr_data_u64(it.attr));
            ASSERT_TRUE(cps_api_object_attr_data_u64(it.attr)==64);
            break;
        }
        cps_api_object_attr_delete(obj,cps_api_object_attr_id(it.attr));

        cps_api_object_it_begin(obj,&it);

    }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
