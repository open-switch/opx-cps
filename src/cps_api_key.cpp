
/* OPENSOURCELICENSE */
/*
 * cps_api_key.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "cps_api_key.h"
#include "cps_api_object_key.h"
#include "std_utils.h"

#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>

#define CPS_API_ATTR_KEY_ID CPS_API_ATTR_RESERVE_RANGE_END

extern "C" int cps_api_key_matches( cps_api_key_t *  key, cps_api_key_t * comparison, bool exact) {
    int src_len = cps_api_key_get_len_in_bytes(key);
    int comp_len = cps_api_key_get_len_in_bytes(comparison);
    int match_len = (src_len > comp_len) ? comp_len : src_len;
    int res = memcmp(cps_api_key_elem_start((cps_api_key_t *)key),
            cps_api_key_elem_start((cps_api_key_t *)comparison),match_len);

    if (exact && src_len!=comp_len) {
        return res!=0 ? res : src_len - comp_len;
    }

    if (comp_len > src_len) {
        return res!=0 ? res : 1;
    }

    return res;
}

#define REQUIRED_ADDITIONAL_SPACE (12) //10 chars + ws + 1 extra :)

extern "C" char * cps_api_key_print(cps_api_key_t *key, char *buff, size_t len) {
    STD_ASSERT(buff!=NULL);

    size_t klen = cps_api_key_get_len((cps_api_key_t *)key);
    if (klen > CPS_OBJ_MAX_KEY_LEN) {
        buff[0] = '\0';
        return buff;
    }

    size_t offset = 0;
    size_t ix = 0;
    buff[0] = '\0';
    const uint32_t *head = cps_api_key_elem_start(key);
    for ( ; (ix < klen) ; ++ix ) {
        int amt = snprintf(buff+offset, len - offset, "%d.",head[ix]);
        if (amt < 0) break;
        offset+=amt;
        if ((offset + REQUIRED_ADDITIONAL_SPACE) > len) {
            snprintf(buff+offset, len - offset, "TR");
            break;
        }
    }

    return buff;
}

extern "C" bool cps_api_key_from_string(cps_api_key_t *key,const char *buff) {
    std_parsed_string_t handle;
    if (!std_parse_string(&handle,buff,".")) return false;
    memset(key,0,sizeof(*key));
    size_t ix = 0;
    size_t mx = std_parse_string_num_tokens(handle);
    bool rc = true;
    for ( ; ix < mx ; ++ix ) {
        unsigned long int ul =strtoul(std_parse_string_at(handle,ix),NULL,0);
        if (ul==ULONG_MAX && errno==ERANGE) {
            rc = false;
            break;
        }
        cps_api_key_set(key,ix,ul);
    }
    cps_api_key_set_len(key,ix);
    std_parse_string_free(handle);
    return rc;

}

extern "C" cps_api_object_attr_t cps_api_get_key_data(cps_api_object_t obj,cps_api_attr_id_t id) {
    cps_api_attr_id_t ids[] = {CPS_API_ATTR_KEY_ID,id};
    cps_api_object_attr_t p = cps_api_object_e_get(obj,ids,sizeof(ids)/sizeof(*ids));
    if (p==NULL) p = cps_api_object_e_get(obj,&id,1);
    return p;
}


extern "C" bool cps_api_set_key_data(cps_api_object_t obj,cps_api_attr_id_t id,
        cps_api_object_ATTR_TYPE_t type, const void *data, size_t len) {
    cps_api_attr_id_t ids[] = {CPS_API_ATTR_KEY_ID,id};
    return cps_api_object_e_add(obj,ids,sizeof(ids)/sizeof(*ids),type,data,len);
}

extern "C" size_t cps_api_key_hash(cps_api_key_t *key) {
    size_t ix = 0;
    size_t mx = (CPS_OBJ_KEY_HEADER_SIZE + cps_api_key_get_len_in_bytes(key)) / CPS_OBJ_KEY_ELEM_SIZE;
    uint32_t *ptr = (uint32_t *)(*key);
    size_t hash = 0;
    std::hash<uint32_t> h;

    for ( ; ix < mx ; ++ix ) {
        hash <<=1;
        hash^=h(*(ptr++));
    }
    return hash;
}