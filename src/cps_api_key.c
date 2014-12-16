
/* OPENSOURCELICENSE */
/*
 * cps_api_key.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "cps_api_key.h"

#include <stdio.h>

int cps_api_key_matches(const cps_api_key_t *  key, const cps_api_key_t * comparison, bool exact) {
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

char * cps_api_key_print(cps_api_key_t *key, char *buff, size_t len) {
    STD_ASSERT(buff!=NULL);

    size_t klen = cps_api_key_get_len(key);
    if (klen > CPS_OBJ_MAX_KEY_LEN) {
        buff[0] = '\0';
        return buff;
    }

    size_t offset = 0;
    size_t ix = 0;
    buff[0] = '\0';
    uint32_t *head = cps_api_key_elem_start(key);
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
