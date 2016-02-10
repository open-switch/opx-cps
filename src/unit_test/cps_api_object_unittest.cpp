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

/*
 * cps_api_object_unittest.cpp
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
#include <stdlib.h>

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

TEST(cps_api_object,add_delete_test) {
    cps_api_object_t obj = cps_api_object_create();

    size_t ix = 0;
    size_t mx = 10;

    for (; ix < mx; ++ix) {
        time_t tm = time(NULL);
        size_t id = ix + (tm & 0xf);
        ASSERT_TRUE(cps_api_object_attr_add(obj, id , (void*)"cliff", 6));
        ASSERT_TRUE(cps_api_object_attr_get(obj,id)!=NULL);
    }

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    while ( cps_api_object_it_valid(&it) ) {
        cps_api_object_attr_delete(obj, std_tlv_tag(it.attr));
        cps_api_object_it_begin(obj,&it);
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
        ASSERT_TRUE(cps_api_object_e_add(obj,lst[ix],6,cps_api_object_ATTR_T_BIN,
                c,strlen(c)));
        uint16_t val=1;
        ASSERT_TRUE(cps_api_object_e_add(obj,lst[ix],6,cps_api_object_ATTR_T_U16,
                &val,sizeof(val)));
        size_t i = 0;
        for ( ; i < ix ; ++i ) {
            cps_api_object_attr_t it = cps_api_object_e_get(obj,lst[i],6);
            ASSERT_TRUE(it!=NULL);
        }
    }
}

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

TEST(cps_api_object,cps_obj_attr_null) {
    cps_api_object_guard og(cps_api_object_create());
    ASSERT_TRUE(og.valid());
    cps_api_object_attr_add(og.get(),0,NULL,0);
    ASSERT_TRUE(cps_api_object_attr_get(og.get(),0)!=NULL);
    cps_api_attr_id_t ids[4];    //don't really care about the values
    ids[0] = 0;
    ASSERT_TRUE(cps_api_object_e_get(og.get(),ids,1)!=NULL);

    ASSERT_TRUE(cps_api_object_e_add(og.get(),ids,4,cps_api_object_ATTR_T_BIN,NULL,0));
    ASSERT_TRUE(cps_api_object_e_get(og.get(),ids,4)!=NULL);
}

TEST(cps_api_object,cps_obj_attr) {
    cps_api_object_t obj = cps_api_object_create();
    ASSERT_TRUE(obj!=nullptr);
    uint8_t u8=1;
    uint16_t u16=2;
    uint32_t u32=3;
    uint64_t u64=4;

    ASSERT_TRUE(cps_api_object_int_type_for_len(sizeof(u64))==cps_api_object_ATTR_T_U64);
    ASSERT_TRUE(cps_api_object_int_type_for_len(sizeof(u16))==cps_api_object_ATTR_T_U16);
    ASSERT_TRUE(cps_api_object_int_type_for_len(sizeof(u32))==cps_api_object_ATTR_T_U32);
    ASSERT_TRUE(cps_api_object_int_type_for_len(sizeof(u8))==cps_api_object_ATTR_T_BIN);

    cps_api_attr_id_t id = u8;
    ASSERT_TRUE(cps_api_object_e_add_int(obj,&id,1,&u8,sizeof(u8)));
    id = u16;
    ASSERT_TRUE(cps_api_object_e_add_int(obj,&id,1,&u16,sizeof(u16)));
    id = u32;
    ASSERT_TRUE(cps_api_object_e_add_int(obj,&id,1,&u32,sizeof(u32)));
    id = u64;
    ASSERT_TRUE(cps_api_object_e_add_int(obj,&id,1,&u64,sizeof(u64)));

    cps_api_object_attr_t a = cps_api_object_attr_get(obj,u8);
    ASSERT_TRUE(a!=nullptr);
    ASSERT_TRUE(cps_api_object_attr_data_uint(a)==u8);

    a = cps_api_object_attr_get(obj,u16);
    ASSERT_TRUE(a!=nullptr);
    ASSERT_TRUE(cps_api_object_attr_data_uint(a)==u16);

    a = cps_api_object_attr_get(obj,u32);
    ASSERT_TRUE(a!=nullptr);
    ASSERT_TRUE(cps_api_object_attr_data_uint(a)==u32);

    a = cps_api_object_attr_get(obj,u64);
    ASSERT_TRUE(a!=nullptr);
    ASSERT_TRUE(cps_api_object_attr_data_uint(a)==0);

    cps_api_object_delete(obj);

    obj = cps_api_object_create();

    cps_api_object_attr_add_u32(obj,0,1);
    cps_api_object_attr_add_u32(obj,0,1);
    cps_api_object_attr_add_u32(obj,1,1);
    cps_api_object_attr_add_u32(obj,0,1);
    cps_api_object_attr_add_u32(obj,1,1);
    cps_api_object_attr_add_u32(obj,0,1);

    {
        cps_api_object_it_t it;
        size_t ix  = 0;
        for ( cps_api_object_it_begin(obj,&it) ; cps_api_object_it_attr_walk(&it,0) ; cps_api_object_it_next(&it) ) {
            ++ix;
        }
        ASSERT_TRUE(ix==4);
    }
    cps_api_object_delete(obj);

}

TEST(cps_api_object,cps_object_attr_merge) {
    //test out object merges

    cps_api_object_guard o1(cps_api_object_create());
    cps_api_object_guard o2(cps_api_object_create());
    ASSERT_TRUE(o1.get()!=nullptr && o2.get()!=nullptr);

    cps_api_object_attr_add_u32(o1.get(),0,1);
    cps_api_object_attr_add_u32(o1.get(),1,1);

    cps_api_object_attr_add_u32(o2.get(),2,1);
    cps_api_object_attr_add_u32(o2.get(),3,1);
    cps_api_object_attr_add_u32(o2.get(),4,1);

    ASSERT_TRUE(cps_api_object_attr_merge(o1.get(),o2.get(),false));

    {
        cps_api_object_it_t it;
        size_t ix  = 0;
        for ( cps_api_object_it_begin(o1.get(),&it) ;
                cps_api_object_it_valid(&it) ;
                cps_api_object_it_next(&it) ) {
            printf("Expected %d found %d\n",(int)ix,(int)cps_api_object_attr_id(it.attr));
            ASSERT_TRUE(cps_api_object_attr_id(it.attr)==ix);
            ++ix;
        }
        ASSERT_TRUE(ix==5); //0123 and 4 since the increment at end
    }

    ASSERT_TRUE(cps_api_object_attr_merge(o2.get(),o1.get(),true));
    {
        cps_api_object_it_t it;
        size_t ix  = 0;
        for ( cps_api_object_it_begin(o1.get(),&it) ;
                cps_api_object_it_valid(&it) ;
                cps_api_object_it_next(&it) ) {
            printf("Expected %d found %d\n",(int)ix,(int)cps_api_object_attr_id(it.attr));
            ASSERT_TRUE(cps_api_object_attr_id(it.attr)==ix);
            ++ix;
        }
        ASSERT_TRUE(ix==5); //0123 and 4 since the increment at end
    }

    ASSERT_TRUE(cps_api_object_attr_merge(o1.get(),o2.get(),false));
    {
        cps_api_object_it_t it;
        size_t ix  = 0;
        for ( cps_api_object_it_begin(o1.get(),&it) ;
                cps_api_object_it_valid(&it) ;
                cps_api_object_it_next(&it) ) {
            printf("Expected %d found %d\n",(int)ix,(int)cps_api_object_attr_id(it.attr));
            ++ix;
        }
        ASSERT_TRUE(ix==10); //0123 and 4 since the increment at end
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
