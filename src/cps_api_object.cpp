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
 * filename: cps_api_common_list.cpp
 */

/*
 * cps_api_common_list.cpp
 */
#include "std_mutex_lock.h"

#include "cps_string_utils.h"

#include "std_tlv.h"
#include "cps_api_object.h"
#include "private/cps_api_object_internal.h"
#include "event_log.h"

#include <endian.h>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <unordered_map>
#include <stdio.h>
#include <string>

#define DEF_OBJECT_SIZE (512)
#define DEF_OBJECT_REALLOC_STEP_SIZE (128)


//Basic Object Data Functions


//Return the amount of space used by attributes within this object
static inline size_t obj_used_len(cps_api_object_internal_t *cur) {
    return cur->len - cur->remain;
}

//return the location of the first TLV
static inline void * obj_data(cps_api_object_internal_t *p) {
    return ((uint8_t*)p->data) + sizeof(*p->data);
}

static inline void * obj_data_end(cps_api_object_internal_t *p) {
    return std_tlv_offset(obj_data(p), obj_used_len(p));
}

//given a pointer to data within the object, return the offset within the object
static inline size_t obj_data_offset(cps_api_object_internal_t *p, void * tlv) {
    uint8_t* loc = (uint8_t*)tlv;
    uint8_t* data = (uint8_t*)obj_data(p);

    return loc - data;
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

static bool reserve_spacespace(cps_api_object_internal_t * p, uint64_t len) {
    if (p->remain < len ) {
        p = obj_realloc(p, (size_t)(len + obj_used_len(p) + DEF_OBJECT_REALLOC_STEP_SIZE));
        if (p == NULL) return false;
    }
    return true;
}

//Reserve space and return a pointer to the end of the current set of attributes
static void * add_get_tlv_pos_with_enough_space(cps_api_object_internal_t * p, uint64_t attr, uint64_t len) {
    if (!reserve_spacespace(p,len+STD_TLV_HDR_LEN)) return NULL;
    return obj_data_end(p);
}

struct tracker_detail {
    const char *desc;
    unsigned int ln;
    const char *file;
};

typedef std::unordered_map<cps_api_object_t,tracker_detail> tTrackerList;

static std_mutex_lock_create_static_init_rec(db_tracker_lock);
static tTrackerList trackers;

extern "C" {

cps_api_object_ATTR_TYPE_t cps_api_object_int_type_for_len(size_t len) {
    static const cps_api_object_ATTR_TYPE_t types[] = {
            cps_api_object_ATTR_T_BIN,    //0
            cps_api_object_ATTR_T_BIN,    //1
            cps_api_object_ATTR_T_U16,    //2
            cps_api_object_ATTR_T_BIN,    //3
            cps_api_object_ATTR_T_U32,    //4
            cps_api_object_ATTR_T_BIN,    //5
            cps_api_object_ATTR_T_BIN,    //6
            cps_api_object_ATTR_T_BIN,    //7
            cps_api_object_ATTR_T_U64,    //8
    };
    if (len >= (sizeof(types)/sizeof(*types))) {
        return cps_api_object_ATTR_T_BIN;
    }
    return types[len];
}

cps_api_object_t cps_api_object_init(void *data, size_t bufflen) {
    if (bufflen < CPS_API_MIN_OBJ_LEN) return NULL;
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)data;
    bufflen -= sizeof(cps_api_object_internal_t);
    p->allocated = false;
    p->data = (cps_api_object_data_t*)(p+1);
    p->remain = bufflen - sizeof(cps_api_object_data_t);
    p->len = p->remain;

    cps_api_key_set_len(cps_api_object_key((cps_api_object_t)data),0);
    return (cps_api_object_t)(data);
}

void db_list_tracker_add(cps_api_object_t obj, const char * label, unsigned int line,const char *file) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    if (obj==NULL) return ;
    if (file==nullptr) file = "";
    trackers[obj] = {label,line,file};
}

void db_list_tracker_rm(cps_api_object_t obj) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    size_t before = trackers.size();
    trackers.erase(obj);
    size_t after = trackers.size();
    if (before <= after ) {
        EV_LOG(ERR,DSAPI,0,"SWERR","Invalid object delete found.");
    }
}

bool cps_api_list_debug() {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    tTrackerList::iterator it = trackers.begin();
    tTrackerList::iterator end = trackers.end();
    if (it==end) return true;
    for ( ; it != end ; ++it ) {
        printf("Objects still found from %s:%s:%d\n",it->second.file,it->second.desc,it->second.ln);
    }
    return false;
}

cps_api_object_t cps_api_object_create_int(const char *desc, unsigned int line, const char *file) {
    cps_api_object_t obj = obj_alloc(DEF_OBJECT_SIZE);
    if (obj==nullptr) {
        EV_LOG(ERR,DSAPI,0,"CPS-OBJ","Failed to allocate memory for object.. expect sytem issues.");
    }
    db_list_tracker_add(obj, desc, line,file);
    return obj;
}

bool cps_api_object_clone(cps_api_object_t d, cps_api_object_t s) {
    cps_api_object_internal_t *dest = (cps_api_object_internal_t*)d;
    cps_api_object_internal_t *src = (cps_api_object_internal_t*)s;
    size_t amt = obj_used_len(src);

    if (dest->len < amt) {
        if (obj_realloc(dest,src->len)==NULL) return false;
    }
    memcpy(dest->data,src->data,amt+sizeof(cps_api_object_data_t));
    dest->remain = dest->len - amt;
    return true;
}

void __delete_repeated_attributes(cps_api_object_t d, cps_api_object_t s) {
    cps_api_object_it_t it;
    cps_api_object_it_begin(s,&it);

    while (cps_api_object_it_valid(&it)) {
        cps_api_attr_id_t id = cps_api_object_attr_id(it.attr);
        cps_api_object_attr_delete(d,id);
        cps_api_object_it_next(&it);
    }
}

bool cps_api_object_attr_merge(cps_api_object_t d, cps_api_object_t s, bool remove_dup) {
    cps_api_object_internal_t *dest = (cps_api_object_internal_t*)d;
    cps_api_object_internal_t *src = (cps_api_object_internal_t*)s;

    size_t _cur = obj_used_len(dest);
    size_t _amt = obj_used_len(src);

    if (_amt==0) return true;

    if (remove_dup && (_cur>0)) {
        //@TODO optimize in the case of zero attributes
        __delete_repeated_attributes(d,s);
        _cur = obj_used_len(dest);
    }

    size_t _tot = _amt + _cur;

    if (dest->len < (_tot)) {
        if (obj_realloc(dest,_tot)==NULL) return false;
    }

    memcpy(obj_data_end(dest),obj_data(src),_amt);
    dest->remain -= _amt;
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
    o->remain = o->len - (size_of_object_received - sizeof(*o->data));

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
    while (len >= STD_TLV_HDR_LEN) {
        uint64_t tag = std_tlv_tag(tlv);
        if (attr == tag) {
            return (cps_api_object_attr_t)tlv;
        }
        tlv = std_tlv_next(tlv, &len);
    }
    return NULL;
}

bool cps_api_object_attr_get_list(cps_api_object_t obj,
        cps_api_attr_id_t *attr, cps_api_object_attr_t *pointers, size_t count) {
    size_t ix = 0;
    for ( ; ix < count ; ++ix ) {
        pointers[ix] = cps_api_object_attr_get(obj,attr[ix]);
        if (pointers[ix]==NULL) return false;
    }
    return true;
}

void cps_api_object_attr_delete(cps_api_object_t obj, cps_api_attr_id_t attr_id) {
    cps_api_object_internal_t *p = (cps_api_object_internal_t*)obj;

    cps_api_object_attr_t  attr = cps_api_object_attr_get(obj, attr_id);
    if (attr != nullptr) {
        void * tlv_end = (((uint8_t*)obj_data(p)) + obj_used_len(p));
        size_t rm_len = std_tlv_total_len(attr);
        void * tlv_rm_end = ((uint8_t*)attr) + rm_len;
        size_t copy_len = (uint8_t*)tlv_end - (uint8_t*)tlv_rm_end;

        memmove(attr, tlv_rm_end, copy_len);
        p->remain += rm_len;
    }
}

cps_api_object_attr_t cps_api_object_e_get(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size) {
    size_t len = obj_used_len((cps_api_object_internal_t*)obj);
    void *tlv = obj_data((cps_api_object_internal_t*)obj);
    return std_tlv_efind(tlv,&len,id,id_size);
}

void* cps_api_object_e_get_data(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size) {
    cps_api_object_attr_t attr = cps_api_object_e_get(obj,id,id_size);
    if (attr!=nullptr) {
        return std_tlv_data(attr);
    }
    return nullptr;
}

size_t cps_api_object_e_get_list_data(cps_api_object_t obj, cps_api_attr_id_t *ids, size_t len,
        void ** attr_data, size_t attr_max) {

    cps_api_object_it_t it;
    if (!cps_api_object_it(obj,ids,len,&it)) {
        return 0;
    }

    size_t ix = 0;

    do {
        if (ix >= attr_max) break;
        attr_data[ix++] = cps_api_object_attr_data_bin(it.attr);
        cps_api_object_it_next(&it);
        it.attr = std_tlv_find_next(it.attr,&it.len,ids[len-1]);
    } while (cps_api_object_it_valid(&it));
    return ix;
}

bool cps_api_object_it(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size, cps_api_object_it_t *it) {
    it->attr =  obj_data((cps_api_object_internal_t*)obj);
    it->len = obj_used_len((cps_api_object_internal_t*)obj);

    size_t ix = 0;
    size_t mx = id_size;
    for ( ; ix < mx ; ++ix ) {
        it->attr = std_tlv_find_next(it->attr,&it->len,id[ix]);
        if (!cps_api_object_it_valid(it)) break;
        if ((ix +1) == mx ) {
            return true;
        }
        cps_api_object_it_inside(it);
    }
    return false;
}

static bool add_attribute(cps_api_object_internal_t * p, uint64_t attr, uint64_t len, const void *data) {
    void * ptr = add_get_tlv_pos_with_enough_space(p,attr,len);
    if (ptr==NULL) return false;
    ptr = std_tlv_add(ptr, &p->remain, attr, len, data);
    return (ptr != NULL) ;
}

static bool add_embedded(cps_api_object_internal_t * obj, cps_api_attr_id_t *id,
        size_t id_size, const void *data, size_t dlen) {
    size_t current_obj_len = obj_used_len(obj);

    size_t len = current_obj_len;
    void *tlv = obj_data(obj);
    size_t ix = 0;

    for ( ; ix < (id_size-1); ++ix) {
        tlv = std_tlv_find_next(tlv,&len,id[ix]);
        if (tlv==NULL) break;
        tlv = std_tlv_data(tlv);
        len -= STD_TLV_HDR_LEN;
    }

    size_t remaining_needed = dlen + (STD_TLV_HDR_LEN * (id_size - ix));

    if (!reserve_spacespace(obj,remaining_needed))
       return false;

    (obj)->remain -= remaining_needed;

    tlv = obj_data(obj);
    size_t cur_tlv_len = current_obj_len;

    for ( ix = 0 ; ix < (id_size-1) ; ++ix) {
        len = cur_tlv_len;
        void *target_tlv = std_tlv_find_next(tlv,&len,id[ix]);
        if (target_tlv==NULL) break;

        cur_tlv_len = std_tlv_len(target_tlv);
        std_tlv_set_len(target_tlv,cur_tlv_len+remaining_needed);

        tlv = std_tlv_data(target_tlv);
    }
    tlv = ((uint8_t*)tlv) + cur_tlv_len;    //shift to end of current tlv
    size_t offset = obj_data_offset((cps_api_object_internal_t*)obj,tlv);
    size_t left = current_obj_len - offset;

    memmove(((uint8_t*)tlv)+remaining_needed,tlv,left);

    for ( ; ix < (id_size-1) ; ++ix ) {
        remaining_needed -= STD_TLV_HDR_LEN;
        std_tlv_set_len(tlv,remaining_needed);
        std_tlv_set_tag(tlv,id[ix]);
        tlv = ((uint8_t*)tlv)+ STD_TLV_HDR_LEN;
    }
    std_tlv_set_tag(tlv, id[ix]);
    std_tlv_set_len(tlv, dlen);
    memcpy(std_tlv_data(tlv), data, (size_t)dlen);

    return true;

}

bool cps_api_object_e_add_object(cps_api_object_t obj,cps_api_attr_id_t *id,
        size_t id_size,cps_api_object_t emb_object) {

    cps_api_object_it_t it;
    cps_api_object_it_begin(emb_object,&it);

    return cps_api_object_e_add(obj,id,id_size,cps_api_object_ATTR_T_BIN,it.attr,it.len);
}

bool cps_api_object_e_add(cps_api_object_t obj, cps_api_attr_id_t *id,
        size_t id_size, cps_api_object_ATTR_TYPE_t type, const void *data, size_t dlen) {

    union {
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
    } un;

    switch(type) {
    case cps_api_object_ATTR_T_U16:
        un.u16 = htole16(*(uint16_t*)data); data = &un.u16; break;
    case cps_api_object_ATTR_T_U32:
        un.u32 = htole32(*(uint32_t*)data); data = &un.u32; break;
    case cps_api_object_ATTR_T_U64:
        un.u64 = htole64(*(uint64_t*)data); data = &un.u64; break;
    default:
        //bin;
        break;
    }
    if (id_size==1)
        return add_attribute((cps_api_object_internal_t*)obj,id[0],dlen,data);

    return add_embedded((cps_api_object_internal_t*)obj,id,id_size,data,dlen);
}

void cps_api_object_attr_fill_list(cps_api_object_t obj, size_t base_attr_id, cps_api_object_attr_t *attr, size_t len) {
    void * ptr = (cps_api_object_attr_t)obj_data((cps_api_object_internal_t*)obj);
    size_t used_len = obj_used_len((cps_api_object_internal_t*)obj);
    memset(attr,0,sizeof(*attr)*len);

    if (!std_tlv_valid(ptr,used_len)) return ;

    while (ptr!=NULL && used_len > 0) {
        std_tlv_tag_t tag = std_tlv_tag(ptr);
        size_t offset = tag - base_attr_id;
        if ((tag >= base_attr_id) && (offset < len)) {
            attr[offset] = ptr;
        }
        ptr = std_tlv_next(ptr,&used_len);
    }
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

bool cps_api_array_to_object(const void * data, size_t len, cps_api_object_t obj) {
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

void cps_api_object_it_begin(cps_api_object_t obj, cps_api_object_it_t *it) {
    it->len = obj_used_len( (cps_api_object_internal_t*)obj );
    it->attr =obj_data( (cps_api_object_internal_t*)obj );
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

cps_api_object_t cps_api_object_list_create_obj_and_append(cps_api_object_list_t list) {
    cps_api_object_t obj = cps_api_object_create();
    if (obj==NULL) return NULL;
    if (cps_api_object_list_append(list,obj)) return obj;
    cps_api_object_delete(obj);
    return NULL;
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

    std::string str = "Key (";
    str += cps_api_key_print(cps_api_object_key(obj),buff,len);
    str += ")\n";
    str += cps_string::tostring(cps_api_object_array(obj), cps_api_object_to_array_len(obj));

    buff[len-1] = '\0';
    strncpy(buff,str.c_str(),len-1);
    return buff;
}

}
