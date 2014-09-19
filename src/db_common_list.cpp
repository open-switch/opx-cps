/*
 * filename: db_common_list.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */
/*
 * db_common_list.cpp
 */

#include "std_mutex_lock.h"
#include "ds_common_list.h"

#include <string.h>
#include <vector>
#include <stdlib.h>
#include <map>

extern "C" {

typedef std::vector<db_list_entry_t *> tList;

#define L(x) static_cast<tList *>(x)

static void free_node(db_list_entry_t *p) {
    if (p->allocated) free(p->data);
    free(p);
}
struct tracker_detail {
    const char *desc;
    unsigned int ln;
};
typedef std::map<db_common_list_t,tracker_detail> tTrackerList;

static std_mutex_lock_create_static_init_rec(db_tracker_lock);
static tTrackerList trackers;


void db_list_tracker_add(db_common_list_t list, const char * label, unsigned int line) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    if (list==NULL) return ;
    try {
        tracker_detail d={label,line};
        trackers[list] = d;
    } catch (...) {}
}

void db_list_tracker_rm(db_common_list_t list) {
    std_mutex_simple_lock_guard g(&db_tracker_lock);
    tTrackerList::iterator it = trackers.find(list);
    if (it!=trackers.end()) trackers.erase(it);
    //todo log if not found
}

void db_list_debug() {
    //TODO implement
    std_mutex_simple_lock_guard g(&db_tracker_lock);
}


db_common_list_t db_list_create(const char *desc, unsigned int line) {
    try {
        db_common_list_t p = new tList;
        db_list_tracker_add(p,desc,line);
        return p;
    } catch(...) {     }
    return NULL;
}

void db_list_destroy(db_common_list_t l) {
    tList *list = L(l);
    size_t ix = 0;
    size_t mx = list->size();
    for ( ; ix < mx ; ++ix) {
        db_list_entry_t *e = (*list)[ix];
        free_node(e);
    }
    db_list_tracker_rm(list);
    delete(list);
}

db_list_entry_t *db_list_elem_get(db_common_list_t l,size_t ix) {
    tList &list = *(L(l));
    if (list.size()>ix) return list[ix];
    return NULL;
}

void db_list_elem_del(db_common_list_t l,size_t ix) {
    tList &list = *(L(l));
    if (list.size()<=ix) return;

    tList::iterator it = list.begin()+ix;
    if (it==list.end()) return;

    free_node(*it);
    list.erase(it);
}


bool db_list_elem_add(db_common_list_t l, db_object_type_t type,void *data, size_t len, bool deep_copy) {
    tList &list = *(L(l));

    db_list_entry_t *p = (db_list_entry_t*)malloc(sizeof(db_list_entry_t));
    if (p==NULL) return false;
    if (deep_copy) {
        p->data = malloc(len);
        if (p->data==NULL) {
            free(p);
            return false;
        }
        p->allocated=true;
        memcpy(p->data,data,len);
    } else {
        p->data = data;
        p->allocated=false;
    }
    p->len = len;
    p->type = type;

    try {
        list.push_back(p);
    } catch (...) {
        free_node(p);
        return false;
    }

    return true;
}

size_t db_list_get_len(db_common_list_t l) {
    tList &list = *(L(l));
    return list.size();
}

db_list_entry_t *db_list_elem_next(db_common_list_t l,size_t *index) {
    tList &list = *(L(l));
    if ((*index)>=list.size()) return NULL;
    db_list_entry_t *p = db_list_elem_get(l,*index);
    ++(*index);
    return p;
}

size_t db_list_array_len(db_common_list_t l,db_list_elem_array_calc optional_calc_fun, void *context) {
    tList &list = *(L(l));
    size_t ix = 0;
    size_t mx = list.size();
    size_t amount = 0;
    for ( ; ix < mx ; ++ix ) {
        if (optional_calc_fun!=NULL) {
            amount += optional_calc_fun(context,list[ix]);
        } else amount+=list[ix]->len;
    }
    return amount;
}

bool db_list_mk_array(db_common_list_t list,void *data, size_t len, db_list_convert_functions_t *converter) {
    //TODO implement
    return true;
}

bool db_list_from_array(db_common_list_t list,void *data, size_t len, db_list_convert_function fun,void * convert_context, bool deep_copy) {
//TODO implement
    return true;
}


}
