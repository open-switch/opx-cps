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


#include "cps_api_object.h"
#include "cps_api_key.h"
#include "cps_api_object_key.h"

#include "cps_api_db.h"
#include "cps_api_db_response.h"
#include "cps_string_utils.h"

#include "cps_api_vector_utils.h"
#include "event_log.h"

#include <vector>
#include <functional>
#include <mutex>

#include <hiredis/hiredis.h>


static std::mutex _mutex;

bool cps_db::get_sequence(cps_db::connection &conn, std::vector<char> &key, ssize_t &cntr) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("INCR");
    e[1].from_string(&key[0],key.size());

    response_set resp;

    if (!conn.command(e,sizeof(e)/sizeof(*e),resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    if (r.is_int()) {
        cntr = (r.get_int());
        return true;
    }
    return false;
}

bool cps_db::fetch_all_keys(cps_db::connection &conn, const void *filt, size_t flen,
        const std::function<void(const void *key, size_t klen)> &fun) {

    std::string start = "0";

    do {
        cps_db::connection::db_operation_atom_t e[6];
        e[0].from_string("SCAN");
        e[1].from_string(start.c_str());
        e[2].from_string("MATCH");
        e[3].from_string((const char *)filt,flen);
        e[4].from_string("COUNT");
        e[5].from_string(CPS_DB_MAX_ITEMS_PER_SCAN);

        response_set resp;
        if (!conn.command(e,sizeof(e)/sizeof(*e),resp)) {
            return false;
        }

        cps_db::response it(resp.get()[0]);
        if (it.elements()!=2) {
            return false;
        }
        cps_db::response cursor(it.element_at(0));
        cps_db::response values(it.element_at(1));
        start = cursor.get_str();

        for (size_t ix =0,mx=values.elements(); ix < mx ; ++ix ) {
            redisReply * data = (redisReply *)values.element_at(ix);
            fun(data->str,data->len);
        }

        if (start=="0") break;
    } while(true);
    return true;
}

bool cps_db::ping(cps_db::connection &conn) {
    cps_db::connection::db_operation_atom_t e;
    e.from_string("PING");
    response_set resp;

    if (!conn.command(&e,1,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    if (r.has_elements()) {
        r = cps_db::response(r.element_at(0));
        return strcasecmp((const char *)r.get_str(),"PONG")==0 ;
    }
    return r.is_ok() ?
            strcasecmp((const char *)r.get_str(),"PONG")==0 :
            false;
}

bool cps_db::select_db(cps_db::connection &conn,const std::string &db_id) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("SELECT");
    e[1].from_string(db_id.c_str());

    response_set resp;
    if (!conn.command(e,2,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    return  (r.is_status() && strcmp("OK",r.get_str())==0);
}

bool cps_db::multi_start(cps_db::connection &conn) {
    cps_db::connection::db_operation_atom_t e;
    e.from_string("MULTI");

    response_set resp;
    if (!conn.command(&e,1,resp)) {
        return false;
    }
    cps_db::response r = resp.get_response(0);
    return  (r.is_status() && strcmp("OK",r.get_str())==0);
}

bool cps_db::multi_end(cps_db::connection &conn, bool commit) {
    cps_db::connection::db_operation_atom_t e;
    e.from_string(commit? "EXEC" : "DISCARD");

    response_set resp;
    if (!conn.command(&e,1,resp)) {
        return false;
    }
    cps_db::response r = resp.get_response(0);
    return  (r.is_status() && strcmp("OK",r.get_str())==0);
}

bool cps_db::delete_object(cps_db::connection &conn,std::vector<char> &key) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("DEL");
    e[1].from_string(&key[0],key.size());
    response_set resp;
    return conn.command(e,2,resp);
}

bool cps_db::delete_object(cps_db::connection &conn,cps_api_object_t obj) {
    std::vector<char> key;
    if (cps_db::dbkey_from_instance_key(key,obj)) {
        return delete_object(conn,key);
    }
    return false;
}

namespace {
bool op_on_objects(const char *op, cps_db::connection &conn,cps_api_object_list_t objs) {

    cps_db::connection::db_operation_atom_t multi;
    multi.from_string("MULTI");
    if (!conn.operation(&multi,1,false)) return false;

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(objs);
    for ( ; ix < mx ; ++ix ) {
        cps_db::connection::db_operation_atom_t e[2];
        e[0].from_string(op);
        if (strcmp(op,"DEL")==0) {
            e[1]._atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_INSTANCE;
        } else {
            e[1]._atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_KEY_AND_DATA;
        }
        e[1]._object = cps_api_object_list_get(objs,ix);;
        if (!conn.operation(e,2,false)) return false;
    }
    multi.from_string("EXEC");
    if (!conn.operation(&multi,1,false)) return false;

    {
        cps_db::response_set multi;
        if (!conn.response(multi,1)) return false;
        cps_db::response _multi(multi.get()[0]);
        if (!_multi.is_str() || strcmp(_multi.get_str(),"OK")!=0) return false;
    }
    for (ix = 0; ix < mx ; ++ix ) {
        cps_db::response_set queue;
        if (!conn.response(queue,1)) return false;
        cps_db::response _queue(queue.get()[0]);
        if (!_queue.is_str() || strcmp(_queue.get_str(),"QUEUED")!=0) return false;
    }

    cps_db::response_set resp;
    if (!conn.response(resp,1)) return false;

    if (resp.get()[0]==nullptr) return false;

    cps_db::response _exec(resp.get()[0]);

    for ( ix = 0, mx = _exec.elements(); ix < mx ; ++ix ) {
        cps_db::response _resp(_exec.element_at(ix));
        if (_resp.is_ok()) continue;
        return false;
    }
    return true;
}
}

bool cps_db::delete_objects(cps_db::connection &conn,cps_api_object_list_t objs) {
    return op_on_objects("DEL",conn,objs);
}

bool cps_db::store_objects(cps_db::connection &conn,cps_api_object_list_t objs) {
    return op_on_objects("HSET",conn,objs);
}

bool cps_db::subscribe(cps_db::connection &conn, cps_api_object_t obj) {
    std::vector<char> key;
    if (!cps_db::dbkey_from_instance_key(key,obj)) return false;
    return subscribe(conn,key);
}

bool cps_db::publish(cps_db::connection &conn, cps_api_object_t obj) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("PUBLISH");
    e[1]._atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_INSTANCE;
    e[1]._object = obj;
    e[2]._atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_DATA;
    e[2]._object = obj;

    response_set resp;
    if (!conn.command(e,3,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    bool rc = r.is_int();

    return rc;
}

bool cps_db::subscribe(cps_db::connection &conn, std::vector<char> &key) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PSUBSCRIBE");
    if (key[key.size()-1]!='*') cps_utils::cps_api_vector_util_append(key,"*",1);
    e[1].from_string(&key[0],key.size());

    response_set resp;
    if (!conn.command(e,2,resp)) {
        EV_LOG(ERR,DSAPI,0,"CPS-DB-SUB","Subscribe failed to return response");
        return false;
    }

    cps_db::response r = resp.get_response(0);
    if (r.elements()==3) {
        cps_db::response msg (r.element_at(0));
        cps_db::response status (r.element_at(2));
        if (msg.is_str() && strcasecmp(msg.get_str(),"psubscribe")==0) {

            bool rc = (status.is_int() && status.get_int()>0);
            if (!rc) EV_LOG(ERR,DSAPI,0,"CPS-DB-SUB","Subscribe failed rc %d",status.get_int());
            return rc;
        }
    }
    return false;
}

bool cps_db::store_object(cps_db::connection &conn,cps_api_object_t obj) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("HSET");
    e[1]._atom_type = cps_db::connection::db_operation_atom_t::obj_fields_t::obj_field_OBJ_KEY_AND_DATA;
    e[1]._object = obj;

    response_set resp;
    if (!conn.command(e,2,resp)) {
        return false;
    }
    cps_db::response r = resp.get_response(0);
    return r.is_int() && (r.get_int()==0 || r.get_int()==1);
}

bool cps_db::get_object(cps_db::connection &conn, const std::vector<char> &key, cps_api_object_t obj) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("HGET");
    e[1].from_string(&key[0],key.size());
    e[2].from_string("object");
    response_set resp;
    if (!conn.command(e,3,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);

    bool rc = false;

    if (r.is_str() && cps_api_array_to_object(r.get_str(),r.get_str_len(),obj)) {
        rc = true;
    }

    return rc;
}

bool cps_db::get_object(cps_db::connection &conn, cps_api_object_t obj) {
    std::vector<char> key;
    if (!cps_db::dbkey_from_instance_key(key,obj)) return false;
    return get_object(conn,key,obj);
}

#include <iostream>
#include "cps_string_utils.h"

namespace {
bool __get_objs(cps_db::connection &conn, std::vector<std::vector<char>> &all_keys, size_t start, size_t mx , cps_api_object_list_t obj_list) {
    cps_db::connection::db_operation_atom_t multi;
    multi.from_string("MULTI");
    if (!conn.operation(&multi,1,false)) return false;

    for ( size_t ix = start; ix < mx ; ++ix  ) {
        cps_db::connection::db_operation_atom_t e[3];
        e[0].from_string("HGET");
        e[1].from_string(&all_keys[ix][0],all_keys[ix].size());
        e[2].from_string("object");
        if (!conn.operation(e,3,false)) return false;
    }

    multi.from_string("EXEC");
    if (!conn.operation(&multi,1,false)) return false;

    {    //start of multi
        cps_db::response_set multi;
        if (!conn.response(multi,1)) return false;
        cps_db::response _multi(multi.get()[0]);
        if (!_multi.is_str() || strcmp(_multi.get_str(),"OK")!=0) return false;
    }

    for ( size_t ix = start; ix < mx ; ++ix  ) {
        cps_db::response_set queue;
        if (!conn.response(queue,1)) return false;
        cps_db::response _queue(queue.get()[0]);
        if (!_queue.is_str() || strcmp(_queue.get_str(),"QUEUED")!=0) return false;
    }

    cps_db::response_set resp;
    if (!conn.response(resp,1)) return false;
    if (resp.get()[0]==nullptr) return false;

    //load the respose for the exec - all responses in array
    cps_db::response _exec(resp.get()[0]);
    for ( size_t ix = 0, _end_elem = _exec.elements(); ix < _end_elem; ++ix  ) {
        cps_db::response _resp(_exec.element_at(ix));
        cps_api_object_guard og(cps_api_object_create());
        if (_resp.is_str() && _resp.get_str()!=nullptr &&
                cps_api_array_to_object(_resp.get_str(),_resp.get_str_len(),og.get())) {
            if (!cps_api_object_list_append(obj_list,og.get())) {
                return false;
            }
            og.release();
        }
    }
    return true;
}
}

bool cps_db::get_objects(cps_db::connection &conn,std::vector<char> &key,cps_api_object_list_t obj_list) {
    cps_utils::cps_api_vector_util_append(key,"*",1);

    std::vector<std::vector<char>> all_keys;
    bool rc = fetch_all_keys(conn, &key[0],key.size(),[&conn,&all_keys](const void *key, size_t len){
        std::vector<char> c;
        cps_utils::cps_api_vector_util_append(c,key,len);
        all_keys.push_back(std::move(c));
    });
    (void)rc;  //if (!rc) return false;

    size_t _processed_len = 0;
    const static int CHUNK_SIZE = CPS_DB_MAX_ITEMS_PER_PIPELINE;

    do {
        size_t _start = _processed_len;
        _processed_len = _start + CHUNK_SIZE;
        if (_processed_len > all_keys.size()) _processed_len = all_keys.size();
        if (!__get_objs(conn,all_keys,_start,_processed_len,obj_list)) {
            return false;
        }
    } while (_processed_len < all_keys.size());

    return true;
}

bool cps_db::get_objects(cps_db::connection &conn, cps_api_object_t obj,cps_api_object_list_t obj_list) {
    std::vector<char> k;
    if (!cps_db::dbkey_from_class_key(k,cps_api_object_key(obj))) return false;
    return get_objects(conn,k,obj_list);
}

cps_db::response_set::~response_set() {
    for ( size_t ix = 0; ix < _used ; ++ix ) {
        freeReplyObject((redisReply*)_data[ix]);
    }
    _used = 0;
}

