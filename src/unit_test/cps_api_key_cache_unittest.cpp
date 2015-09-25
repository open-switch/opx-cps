/** OPENSOURCELICENSE */
/*
 * cps_api_key_cache_unittest.cpp
 *
 *  Created on: Sep 24, 2015
 */


/** OPENSOURCELICENSE */

/* OPENSOURCELICENSE */
/*
 * cps_api_operation_unittest.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */



#include "gtest/gtest.h"

#include "private/cps_api_key_cache.h"
#include "cps_api_operation.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <cstdio>
#include <memory>

std::vector<cps_api_key_t*> lst;
std::vector<cps_api_key_t*> missing;

struct entry {
    cps_api_key_t key;
    uint64_t ix;
};

cps_api_key_cache<entry> cache;
cps_api_key_cache<entry> full_cache;

cps_api_key_cache<entry> _missing_cache;


void key_add(size_t *_k_data, size_t pos, size_t mx) {
    if (pos >= 5) return;
    for (int a = 0, _a = 10; a < _a ; ++a ) {
        _k_data[pos] = a;
        key_add(_k_data,pos+1,mx);
        if ((pos+1) == 4) {
            cps_api_key_t *key = (cps_api_key_t*)calloc(1,sizeof(cps_api_key_t));
            cps_api_key_init(key,(cps_api_qualifier_t)(_k_data[0]+1),_k_data[1]+1,_k_data[2]+1,2,_k_data[3],_k_data[4]);

            char b1[100];
            printf("Adding for test.. %s\n",cps_api_key_print(key,b1,sizeof(b1)));
            lst.push_back(key);
        }
    }
}

TEST(cps_api_key_cache,test_setup) {
    srand(time(NULL));
    size_t _k_data[5];
    memset(_k_data,0,sizeof(_k_data));

    key_add(_k_data,0,10);
}

TEST(cps_api_key_cache,test_key_inserts) {
    size_t ix = 0;
    size_t mx = lst.size();
    for ( ; ix < mx ; ++ix ) {
        if ((rand()%20)==1) {
            char b1[100];
            missing.push_back(lst[ix]);
            entry e;
            _missing_cache.insert(lst[ix],e);
            printf("Skipping.. %s\n",cps_api_key_print(lst[ix],b1,sizeof(b1)));
            ASSERT_TRUE(_missing_cache.find(lst[ix],e,true));
            continue;
        }
        entry e;
        cps_api_key_copy(&e.key,lst[ix]);
        e.ix = ix;

        ASSERT_FALSE(cache.find(&e.key,e,true));

        cache.insert(&e.key,e);

        size_t klen = (rand() % 4) + 1;
        cps_api_key_set_len(&e.key,klen);
        if (cache.find(&e.key,e,true)) continue;
        cache.insert(&e.key,e);

    }
}

TEST(cps_api_key_cache,test_key_inserts_full) {
    size_t ix = 0;
    size_t mx = lst.size();
    for ( ; ix < mx ; ++ix ) {
        entry e;

        cps_api_key_copy(&e.key,lst[ix]);
        e.ix = ix;

        ASSERT_FALSE(full_cache.find(&e.key,e,true));
        full_cache.insert(&e.key,e);
        ASSERT_TRUE(full_cache.find(&e.key,e,true));
    }
}

TEST(cps_api_key_cache,test_missing_basic) {
    size_t mx = 100;
    while (mx-- > 1) {
        size_t ix = rand() % missing.size();
        entry val;
        if (!cache.find(missing[ix],val,false)) {
            char b1[100];
            printf("Can't find %s\n",cps_api_key_print(missing[ix],b1,sizeof(b1)));
            continue;
        }
        char b1[100],b2[100];
        printf("Found key %s looking for key %s\n",cps_api_key_print(&val.key,b2,sizeof(b2)),
                cps_api_key_print(missing[ix],b1,sizeof(b1)));
    }
}

TEST(cps_api_key_cache,test_key_basic) {
    size_t mx = 100;
    while (mx-- > 1) {
        size_t ix = rand() % lst.size();
        entry val;
        if (!cache.find(lst[ix],val,true)) {
            if (_missing_cache.find(lst[ix],val,true)) {
                continue;
            }
            char b1[100];
            printf("Can't find %s\n",cps_api_key_print(lst[ix],b1,sizeof(b1)));
            continue;
        }
        ASSERT_TRUE(ix == val.ix);
    }

    mx = 100;
    while (mx-- > 1) {
        size_t ix = rand() % lst.size();
        entry val;
        if (!cache.find(lst[ix],val,false)) {
            if (_missing_cache.find(lst[ix],val,true)) {
                continue;
            }
            char b1[100];
            printf("Can't find %s\n",cps_api_key_print(lst[ix],b1,sizeof(b1)));
            continue;
        }
        char b1[100],b2[100];
        printf("Found key %s looking for key %s\n",cps_api_key_print(&val.key,b2,sizeof(b2)),
                cps_api_key_print(lst[ix],b1,sizeof(b1)));
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
