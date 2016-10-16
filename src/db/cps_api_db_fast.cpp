/*
 * cps_api_db_fast.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: cwichmann
 */

#include "cps_api_db.h"

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


bool cps_db::get_objects(cps_db::connection &conn, cps_api_object_list_t filters,cps_api_object_list_t objects) {

}

bool cps_db::merge_objects(cps_db::connection &conn, cps_api_object_list_t obj_list) {



    std::vector<char> k;
    if (!cps_db::dbkey_from_instance_key(k,obj)) return false;
    return get_objects(conn,k,obj_list);
}

