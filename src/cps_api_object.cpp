/*
 * filename: cps_api_common_list.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * cps_api_common_list.cpp
 */
#include "std_mutex_lock.h"

#include "std_tlv.h"
#include "cps_api_object.h"

#include <endian.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <map>
#include <stdio.h> //for snprintf
#include <string>

#define DEF_OBJECT_SIZE (512)
#define DEF_OBJECT_REALLOC_STEP_SIZE (128)


static inline size_t obj_used_len(cps_api_object_internal_t *cur) {
    return cur->len - cur->remain;
}

static cps_api_object_internal_t * obj_realloc(cps_api_object_internal_t *cur, size_t  len) {
    if (cur->allocated==false) return NULL;

    cps_api_object_data_t *p = (cps_api_object_data_t*)realloc(cur->data, len + sizeof(cps_api_object_data_t));
    if (p == NULL) {
        return NULL;
    }

    cur->data = p;
    cur->remain += (len - cur->len);
    cur->len = len;
    return cur;
}

static cps_api_object_internal_t * obj_alloc(size_t  len) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)calloc(1,sizeof(cps_api_object_internal_t));
    if (p == NULL) return NULL;
    p->data = (cps_api_object_data_t*)calloc(1, sizeof(cps_api_object_data_t) + len);
    if (p->data == NULL) {
        free(p);
        return NULL;
    }
    p->allocated = true;
    p->len = len;
    p->remain = len;
    return p;
}

static void obj_delloc(cps_api_object_internal_t *p) {
    if (!p->allocated) return;
    free(p->data);
    free(p);
}

static void * obj_data(cps_api_object_internal_t *p) {
    return ((uint8_t*)p->data) + sizeof(*p->data);
}

static size_t obj_data_offset(cps_api_object_internal_t *p, void * tlv) {
    uint8_t* loc = (uint8_t*)tlv;
    uint8_t* data = (uint8_t*)obj_data(p);

    return loc - data;
}

static void * add_get_tlv_pos_with_enough_space(cps_api_object_internal_t * p, uint64_t attr, uint64_t len) {
    if (p->remain < (len + STD_TLV_HDR_LEN)) {
        p = obj_realloc(p, (size_t)(len + obj_used_len(p) + STD_TLV_HDR_LEN) + DEF_OBJECT_REALLOC_STEP_SIZE);
        if (p == NULL) return NULL;
    }
    return std_tlv_offset(obj_data(p), obj_used_len(p));
}


static bool add_attribute(cps_api_object_internal_t * p, uint64_t attr, uint64_t len, const void *data) {
    void * ptr = add_get_tlv_pos_with_enough_space(p,attr,len);
    if (ptr==NULL) return false;
    ptr = std_tlv_add(ptr, &p->remain, attr, len, data);
    return (ptr != NULL) ;
}

extern "C" {

struct tracker_detail {
    const char *desc;
    unsigned int ln;
};

typedef std::map<cps_api_object_t,tracker_detail> tTrackerList;

static std_mutex_lock_create_static_init_rec(db_tracker_lock);
static tTrackerList trackers;

cps_api_object_t cps_api_object_init(void *data, size_t bufflen) {
    if (bufflen < CPS_API_MIN_OBJ_LEN) return false;
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)data;
    bufflen -= sizeof(cps_api_object_internal_t);
    p->allocated = false;
    p->data = (cps_api_object_data_t*)(p+1);
    p->remain = bufflen - sizeof(cps_api_object_data_t);
    p->len = p->remain;

    return (cps_api_object_t)(data);
}

void db_list_tracker_add(cps_api_object_t obj, const char * label, unsigned int line) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    if (obj==NULL) return ;
    try {
        tracker_detail d={label,line};
        trackers[obj] = d;
    } catch (...) {}
}

void db_list_tracker_rm(cps_api_object_t obj) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    tTrackerList::iterator it = trackers.find(obj);
    if (it!=trackers.end()) trackers.erase(it);
    //todo log if not found
}

void cps_api_list_debug() {
    //TODO implement
    std_mutex_simple_lock_guard g(&db_tracker_lock);
}

cps_api_object_t cps_api_object_create_int(const char *desc, unsigned int line) {
    cps_api_object_t obj = obj_alloc(DEF_OBJECT_SIZE);
    db_list_tracker_add(obj, desc, line);
    return obj;

}

bool cps_api_object_clone(cps_api_object_t d, cps_api_object_t s) {
    cps_api_object_internal_t *dest = (cps_api_object_internal_t*)d;
    cps_api_object_internal_t *src = (cps_api_object_internal_t*)s;
    size_t amt = obj_used_len(src);

    if (dest->len < amt) {
        if (obj_realloc(dest,src->len)==NULL) return false;
    }
    memcpy(dest->data,src->data,amt);
    return true;
}

bool cps_api_object_reserve(cps_api_object_t obj, size_t amount_of_space_to_reserve) {
    return obj_realloc((cps_api_object_internal_t *)obj,amount_of_space_to_reserve)!=NULL;
}
size_t cps_api_object_get_reserve_len(cps_api_object_t obj) {
    return ((cps_api_object_internal_t *)obj)->len;
}

bool cps_api_object_received(cps_api_object_t obj, size_t size_of_object_received) {
    cps_api_object_internal_t *o = (cps_api_object_internal_t*)obj;
    if (o->len < size_of_object_received) return false;
    if (size_of_object_received < sizeof(*o->data)) return false;

    //TODO could walk through the list of newly created TLVs and validate each one
    //but that is probably not necessary at this time.
    return true;
}



void cps_api_object_delete(cps_api_object_t o) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)o;
    if(p->allocated==false) return;
    db_list_tracker_rm(o);
    obj_delloc((cps_api_object_internal_t*)o);
}

void cps_api_object_set_key(cps_api_object_t obj, cps_api_key_t *key) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)obj;
    cps_api_key_copy(&p->data->key, key);
}

cps_api_object_attr_t cps_api_object_attr_get(cps_api_object_t obj, uint64_t attr) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)obj;
    size_t len = obj_used_len(p);
    void *tlv = obj_data(p);
    while (len > STD_TLV_HDR_LEN) {
        uint64_t tag = std_tlv_tag(tlv);
        if (attr == tag) {
            return (cps_api_object_attr_t)tlv;
        }
        tlv = std_tlv_next(tlv, &len);
    }
    return NULL;
}

void cps_api_object_attr_delete(cps_api_object_t obj, cps_api_attr_id_t attr_id) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)obj;

    cps_api_object_attr_t  attr = cps_api_object_attr_get(obj, attr_id);
    if (attr != CPS_API_ATTR_NULL) {
        void * tlv_end = (((uint8_t*)obj_data(p)) + obj_used_len(p));
        size_t rm_len = std_tlv_total_len(attr);
        void * tlv_rm_end = ((uint8_t*)attr) + rm_len;
        size_t copy_len = (uint8_t*)tlv_end - (uint8_t*)tlv_rm_end;

        memmove(attr, tlv_rm_end, copy_len);
        p->remain += rm_len;
    }
}

bool cps_api_object_attr_add_u16(cps_api_object_t obj, cps_api_attr_id_t id,uint16_t data) {
    cps_api_object_internal_t* p = (cps_api_object_internal_t*)obj;

    void * ptr = add_get_tlv_pos_with_enough_space(p,id,sizeof(data));
    if (ptr==NULL) return false;

    ptr = std_tlv_add_u16(ptr, &p->remain, id, data);
    return (ptr != NULL);
}

bool cps_api_object_attr_add_u32(cps_api_object_t obj, cps_api_attr_id_t id,uint32_t data) {
    cps_api_object_internal_t* p = (cps_api_object_internal_t*)obj;

    void * ptr = add_get_tlv_pos_with_enough_space(p,id,sizeof(data));
    if (ptr==NULL) return false;

    ptr = std_tlv_add_u32(ptr, &p->remain, id, data);
    return (ptr != NULL);
}

bool cps_api_object_attr_add_u64(cps_api_object_t obj, cps_api_attr_id_t id,uint64_t data) {
    cps_api_object_internal_t* p = (cps_api_object_internal_t*)obj;

    void * ptr = add_get_tlv_pos_with_enough_space(p,id,sizeof(data));
    if (ptr==NULL) return false;

    ptr = std_tlv_add_u64(ptr, &p->remain, id, data);
    return (ptr != NULL);
}

bool cps_api_object_attr_add(cps_api_object_t o, cps_api_attr_id_t id,const void *data, size_t len) {
    return add_attribute((cps_api_object_internal_t*)o, id, len, data);
}

cps_api_object_attr_t cps_api_object_attr_first(cps_api_object_t obj) {
    void * ptr = (cps_api_object_attr_t)obj_data((cps_api_object_internal_t*)obj);
    return std_tlv_valid(ptr,obj_used_len((cps_api_object_internal_t*)obj)) ?
            ptr : NULL;
}

cps_api_object_attr_t cps_api_object_attr_next(cps_api_object_t obj,cps_api_object_attr_t attr) {
    if (attr == NULL) return NULL;
    cps_api_object_internal_t* obj_ptr = (cps_api_object_internal_t*)obj;
    size_t offset = obj_data_offset(obj_ptr, attr);
    size_t left = obj_used_len(obj_ptr) - offset;

     void * p = std_tlv_next(attr, &left);
     if (p != NULL) {
         if (!std_tlv_valid(p, left)) return NULL;
     }
     return p;
}

const char * cps_api_object_attr_to_string(cps_api_object_attr_t attr, char *buff, size_t len) {
    snprintf(buff,len,"Attr %X, Len %d",(int)cps_api_object_attr_id(attr),
            (int)cps_api_object_attr_len(attr));
    return buff;
}

cps_api_attr_id_t cps_api_object_attr_id(cps_api_object_attr_t attr) {
    return std_tlv_tag(attr);
}

size_t cps_api_object_attr_len(cps_api_object_attr_t attr) {
    return (size_t)std_tlv_len(attr);
}

uint16_t cps_api_object_attr_data_u16(cps_api_object_attr_t attr) {
    return std_tlv_data_u16(attr);
}

uint32_t cps_api_object_attr_data_u32(cps_api_object_attr_t attr) {
    return std_tlv_data_u32(attr);
}

uint64_t cps_api_object_attr_data_u64(cps_api_object_attr_t attr) {
    return std_tlv_data_u64(attr);
}

void *cps_api_object_attr_data_bin(cps_api_object_attr_t attr) {
    return std_tlv_data(attr);
}

size_t cps_api_object_to_array_len(cps_api_object_t obj) {
    cps_api_object_internal_t* p = (cps_api_object_internal_t*)obj;
    return obj_used_len(p) + sizeof(*p->data);
}

void * cps_api_object_array(cps_api_object_t obj) {
    cps_api_object_internal_t* p = (cps_api_object_internal_t*)obj;
    return p->data;
}

cps_api_key_t * cps_api_object_key(cps_api_object_t obj) {
    return &(((cps_api_object_internal_t*)obj)->data->key);
}

bool cps_api_array_to_object(void * data, size_t len, cps_api_object_t obj) {
    if (obj == NULL) return false;

    cps_api_object_internal_t * p = (cps_api_object_internal_t *)obj;
    p->remain = p->len;
    void *new_obj = obj_realloc((cps_api_object_internal_t*)obj, len);
    if (new_obj == NULL) return false;

    memcpy(p->data,data, len);
    p->remain = (p->len - len) + sizeof(*p->data);
    return true;
}

typedef std::vector<cps_api_object_t> tObjList;

cps_api_object_list_t cps_api_object_list_create(void) {
    tObjList * p = new tObjList;
    return (cps_api_object_list_t)p;
}

void cps_api_object_list_destroy(cps_api_object_list_t list, bool delete_all_objects) {
    STD_ASSERT(list!=NULL);
    tObjList * p = (tObjList*)list;
    if (delete_all_objects) {
        size_t ix = 0;
        size_t mx = p->size();
        for ( ; ix < mx ; ++ix ) {
            cps_api_object_delete((*p)[ix]);
        }
    }
    delete p;
}

bool cps_api_object_list_append(cps_api_object_list_t list, cps_api_object_t obj) {
    STD_ASSERT(list!=NULL);
    tObjList * p = (tObjList*)list;
    try {
        p->push_back(obj);
    } catch (...) {
        return false;
    }
    return true;
}

void cps_api_object_list_remove(cps_api_object_list_t list, size_t ix) {
    STD_ASSERT(list!=NULL);
    tObjList * p = (tObjList*)list;
    if (p->size()<=ix) return;

    tObjList::iterator it = p->begin()+ix;
    if (it==p->end()) return;

    p->erase(it);
}

cps_api_object_t cps_api_object_list_get(cps_api_object_list_t list,size_t ix) {
    STD_ASSERT(list!=NULL);
    tObjList * p = (tObjList*)list;
    if (p->size()<=ix) return NULL;
    return (*p)[ix];
}

size_t cps_api_object_list_size(cps_api_object_list_t list) {
    STD_ASSERT(list!=NULL);
    tObjList * p = (tObjList*)list;
    return p->size();
}

const char * cps_api_object_to_string(cps_api_object_t obj, char *buff, size_t len) {
    char *k_str = cps_api_key_print(cps_api_object_key(obj),buff,len);

    std::string str;
    str+= k_str;
    str+= " ";
    cps_api_object_attr_t it = cps_api_object_attr_first(obj);
    for ( ; it != NULL ; it = cps_api_object_attr_next(obj,it)) {
        str+= cps_api_object_attr_to_string(it,buff,len);
        str+=" - ";
    }
    buff[len-1] = '\0';
    strncpy(buff,str.c_str(),len-1);
    return buff;
}

}
