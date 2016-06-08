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
 * cps_api_key.c
 */

#include "cps_class_map.h"
#include "cps_api_key.h"
#include "cps_api_object_key.h"
#include "std_utils.h"

#include <limits.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <string.h>


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

extern "C" char *cps_api_key_name_print(cps_api_key_t *key, char *buff, size_t len) {

    STD_ASSERT(buff!=NULL);
    
    const char *qual = nullptr;
    const char *path = nullptr;
    
    qual = cps_class_qual_from_key(key);
    path = cps_class_string_from_key(key, 1);
        
    if (qual != nullptr && path != nullptr) { strncpy(buff,qual,len-1); strncat(buff,"/",len-1);  }
    else *buff = '\0';
      
    if (path != nullptr) strncat(buff,path,len-1);
    else cps_api_key_print(key,buff,len);

    buff[len-1] = '\0';
        
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
    cps_api_attr_id_t ids[] = {CPS_API_OBJ_KEY_ATTRS,id};
    cps_api_object_attr_t p = cps_api_object_e_get(obj,ids,sizeof(ids)/sizeof(*ids));
    if (p==NULL) p = cps_api_object_e_get(obj,&id,1);
    return p;
}


extern "C" bool cps_api_set_key_data(cps_api_object_t obj,cps_api_attr_id_t id,
        cps_api_object_ATTR_TYPE_t type, const void *data, size_t len) {
    cps_api_attr_id_t ids[] = {CPS_API_OBJ_KEY_ATTRS,id};
    return cps_api_object_e_add(obj,ids,sizeof(ids)/sizeof(*ids),type,data,len);
}

extern "C" uint64_t cps_api_key_hash(cps_api_key_t *key) {
    cps_api_key_element_t *ptr = (cps_api_key_element_t *)cps_api_key_elem_start(key);
    cps_api_key_element_t *end = ptr + cps_api_key_get_len(key);

    uint64_t hash = 0;
    std::hash<uint64_t> h;
    while(ptr < end) {
        hash = (hash << 8) | hash>> ((sizeof(hash)*8)-8);
        hash ^= h(*(ptr++));
    }
    return hash;
}


bool cps_api_key_insert_element(cps_api_key_t *key, size_t ix, cps_api_key_element_t elem) {
    if ((cps_api_key_get_len(key)+1) >= CPS_OBJ_MAX_KEY_LEN) {
        return false;
    }
    size_t cur_len = cps_api_key_get_len(key);
    if (ix > cur_len) return false;

    cps_api_key_element_t * src = cps_api_key_elem_start(key) + ix;
    cps_api_key_element_t * dest = src+1;

    memmove(dest,src,(cur_len - ix)*CPS_OBJ_KEY_ELEM_SIZE);

    cps_api_key_set_len(key,cur_len+1);
    cps_api_key_set(key,ix,elem);

    return true;
}

void cps_api_key_remove_element(cps_api_key_t *key, size_t ix) {
    size_t cur_len = cps_api_key_get_len(key);
    if (ix >= cur_len) return ;

    cps_api_key_element_t * dest = cps_api_key_elem_start(key) + ix;
    cps_api_key_element_t * src = dest+1;
    --cur_len;

    memmove(dest,src,(cur_len - ix)*CPS_OBJ_KEY_ELEM_SIZE);

    cps_api_key_set_len(key,cur_len);
}

/*Sets the key (leaving offset# of spaces at the beginning). */
void cps_api_key_init_from_attr_array(cps_api_key_t *key, cps_api_attr_id_t *elems,size_t len, size_t offset)  {
    size_t ix = 0;

    /*The offset is used to leave space at the beginning of the key.  Used when adding a target qualifier for example*/
    cps_api_key_element_t * dest = cps_api_key_elem_start(key) + offset;
    for (; ix < len ; ++ix ) {
        *dest++ = cps_api_key_element_t(*elems++);
    }
    /*Sets the key len to the size requested and the offset */
    cps_api_key_set_len(key,len+offset);

}
