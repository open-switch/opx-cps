/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
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
#include "cps_api_operation.h"

#include "cps_api_vector_utils.h"
#include "event_log.h"

#include <vector>
#include <functional>
#include <mutex>

#include <iostream>

#include <hiredis/hiredis.h>


static std::mutex _mutex;

bool cps_db::ping(cps_db::connection &conn) {
    cps_db::connection::db_operation_atom_t e;
    e.from_string("PING");
    response_set resp;

    if (!conn.operation(&e,1)) return false;

    if (!conn.response(resp,true)) {
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


bool cps_db::make_slave(cps_db::connection &conn,std::string ip) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("SLAVEOF");
    auto lst = cps_string::split(ip,":");
    if (lst.size()!=2) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB","Failed to connect to server... bad address (%s)",ip.c_str());
        return false;
    }

    e[1].from_string(lst[0].c_str());
    e[2].from_string(lst[1].c_str());

    response_set resp;

    if (!conn.command(e,3,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    return  (r.is_status() && strcmp("OK",r.get_str())==0);
}

bool cps_db::remove_slave(cps_db::connection &conn) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("SLAVEOF");
    e[1].from_string("NO");
    e[2].from_string("ONE");

    response_set resp;

    if (!conn.command(e,3,resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    return  (r.is_status() && strcmp("OK",r.get_str())==0);

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

bool cps_db::delete_object(cps_db::connection &conn,cps_api_object_t obj) {
    std::vector<char> key;
    bool _is_wildcard =false;
    if (!cps_db::dbkey_instance_or_wildcard(key,obj,_is_wildcard)) {
        return false;
    }
    if (!_is_wildcard) {
        cps_db::delete_object(conn,&key[0],key.size());
    } else {
        cps_db::connection_request r(cps_db::ProcessDBCache(),conn.addr());
        if (!r.valid()) return false;
        cps_db::walk_keys(r.get(),&key[0],key.size(),[&](const void *data, size_t len) {
            cps_db::delete_object(conn,(const char *)data,len);
        });
    }
    return true;
}

bool cps_db::get_objects(cps_db::connection &conn, cps_api_object_t obj,cps_api_object_list_t obj_list) {
    std::vector<char> k;
    bool wildcard = false;

    if (cps_api_filter_has_wildcard_attrs(obj)) {    //if prefer to search entries
        wildcard=true;
        if (!cps_db::dbkey_from_instance_key(k,obj,true)) return false;
    } else {    //otherwise guess from attributes provided
        if (!cps_db::dbkey_instance_or_wildcard(k,obj,wildcard)) return false;
    }

    if (wildcard) return get_objects(conn,k,obj_list);

    cps_api_object_guard og(cps_api_object_create());
    if (cps_db::get_object(conn,k,og.get())) {
        if (cps_api_object_list_append(obj_list,og.get())) {
            og.release();
            return true;
        }
    }
    return false;
}


cps_db::response_set::~response_set() {
    for ( size_t ix = 0; ix < _used ; ++ix ) {
        freeReplyObject((redisReply*)_data[ix]);
    }
    _used = 0;
}


bool cps_db::delete_object(cps_db::connection &conn,const char *key, size_t key_len) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("DEL");
    e[1].from_string(key,key_len);
    response_set resp;
    return conn.command(e,2,resp);
}

bool cps_db::atomic_count_set(cps_db::connection &conn,const char *key, size_t key_len, int64_t data) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("SET");
    e[1].from_string(key,key_len);
    e[2].from_string((const char *)&data,sizeof(data));
    response_set resp;
    return conn.command(e,sizeof(e)/sizeof(*e),resp);
}

bool cps_db::atomic_count_change(cps_db::connection &conn,bool inc, const char *key, size_t key_len,
        int64_t &data) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string(inc==true? "INCR" : "DECR");
    e[1].from_string(key,key_len);

    response_set resp;

    if (!conn.command(e,sizeof(e)/sizeof(*e),resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    if (r.is_int()) {
        data = (r.get_int());
        return true;
    }
    return false;
}

bool cps_db::dbkey_field_set_request(cps_db::connection &conn, const char *key, size_t key_len, const char *field, size_t field_len, const char *data, size_t data_len) {
    cps_db::connection::db_operation_atom_t e[4];
    e[0].from_string("HSET");
    e[1].from_string(key, key_len);
    e[2].from_string(field, field_len);
    e[3].from_string(data, data_len);
    return conn.operation(e,sizeof(e)/sizeof(*e));
}

bool cps_db::dbkey_field_set_response(cps_db::connection &conn) {
    return cps_db::set_object_response(conn);
}

bool cps_db::dbkey_field_get_request(cps_db::connection &conn, const char *key, size_t key_len, const char *field, size_t field_len) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("HGET");
    e[1].from_string(key, key_len);
    e[2].from_string(field, field_len);
    return conn.operation(e,sizeof(e)/sizeof(*e));
}

std::string cps_db::dbkey_field_get_response_string(cps_db::connection &conn) {
    response_set resp;
    if (conn.response(resp,false)) {
        cps_db::response r = resp.get_response(0);
        if(r.is_str()) {
            return r.get_str();
        }
    }
    return "";
}

bool cps_db::dbkey_field_delete_request(cps_db::connection &conn, const char *key, size_t key_len, const char * field, size_t field_len) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("HDEL");
    e[1].from_string(key, key_len);
    e[2].from_string(field, field_len);
    return conn.operation(e,sizeof(e)/sizeof(*e));
}

bool cps_db::dbkey_field_delete_response(cps_db::connection &conn) {
    return cps_db::set_object_response(conn);
}

bool cps_db::walk_keys(cps_db::connection &conn, const void *filt, size_t flen,
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

bool cps_db::set_object_request(cps_db::connection &conn, cps_api_object_t obj, bool *check_exists, size_t *lifetime) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("HSET");
    e[1].from_object(obj,true,true);
    return conn.operation(e,sizeof(e)/sizeof(*e));
}

bool cps_db::set_object_response(cps_db::connection &conn) {
    cps_db::response_set rs;
    cps_db::response_set resp;

    if (conn.response(resp,false)) {
        //this level of validation is really not needed since all respnses are valid (no fail besides connection)
        cps_db::response r = resp.get_response(0);
        return r.is_int() && (r.get_int()==0 || r.get_int()==1);
    }
    return false;
}


bool cps_db::get_object_request(cps_db::connection &conn, const char*key, size_t len) {
    cps_db::connection::db_operation_atom_t e[3];
    e[0].from_string("HGET");
    e[1].from_string(key,len);
    e[2].from_string("object");
    return conn.operation(e,sizeof(e)/sizeof(*e));
}

bool cps_db::get_object_response(cps_db::connection &conn, cps_api_object_t obj) {
    cps_db::response_set resp;

    if (conn.response(resp,false)) {
        response r = resp.get_response(0);
        if (r.is_str() && cps_api_array_to_object(r.get_str(),r.get_str_len(),obj)) {
            return true;
        }
    }
    return false;
}

bool cps_db::get_object(cps_db::connection &conn, cps_api_object_t obj) {
    std::vector<char> key;
    if (!cps_db::dbkey_from_instance_key(key,obj,false)) return false;
    return get_object(conn,key,obj);
}
