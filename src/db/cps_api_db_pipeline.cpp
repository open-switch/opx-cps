
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


#include "cps_api_db.h"

#include "cps_api_db_response.h"
#include "cps_api_vector_utils.h"

#include "event_log.h"

constexpr size_t IN_THE_PIPE() { return 500; }


bool cps_db::store_objects(cps_db::connection &conn, cps_api_object_list_t objs) {

    cps_api_object_guard og(cps_api_object_create());

    return cps_utils::cps_api_range_split(cps_api_object_list_size(objs),IN_THE_PIPE(),
            [&](size_t start, size_t mx) -> bool {
            for ( size_t ix = start; ix < mx ; ++ix ) {
                cps_db::connection::db_operation_atom_t e[2];
                e[0].from_string("HSET");
                e[1].from_object(cps_api_object_list_get(objs,ix),true,true);
                if (!conn.operation(e,sizeof(e)/sizeof(*e))) return false;
            }
            for ( size_t ix = start; ix < mx ; ++ix ) {
                cps_db::response_set resp;
                if (conn.response(resp,false)) {
                    response r = resp.get_response(0);
                    if (!r.valid()) {
                        EV_LOGGING(DSAPI,ERR,"CPS-DB-PIPE","Failed to HSET");
                    }
                    continue;
                }
                return false;
            }
            return true;
        });
}

namespace {

static auto bulk_send_req = [] (cps_db::connection &conn, cps_api_object_list_t objs,
        size_t ix, size_t mx, std::vector<char> &key ) -> bool {

    cps_api_object_guard og(cps_api_object_create());

    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t obj = cps_api_object_list_get(objs,ix);
        key.clear();
        if (!cps_db::dbkey_from_instance_key(key,obj,false)) {
            EV_LOGGING(DSAPI,ERR,"DB-MERGE","Can't generate instance key");
            return false;
        }

        if (!cps_db::get_object_request(conn,&key[0],key.size())) {
            return false;
        }
    }

    return true;
};


static auto bulk_get_req_from_array = [] (cps_db::connection &conn,std::vector<std::vector<char>> &keys,
        size_t ix, size_t mx ) -> bool {

    cps_api_object_guard og(cps_api_object_create());

    for ( ; ix < mx ; ++ix ) {
        if (!cps_db::get_object_request(conn,&(keys[ix][0]),keys[ix].size())) {
            return false;
        }
    }

    return true;
};


}

bool cps_db::merge_objects(cps_db::connection &conn, cps_api_object_list_t objs) {

    cps_api_object_guard og(cps_api_object_create());

    return cps_utils::cps_api_range_split(cps_api_object_list_size(objs),IN_THE_PIPE(),
            [&](size_t start, size_t mx) -> bool {
            std::vector<char> key_buff;
            cps_api_object_guard og(cps_api_object_create());

            if (!bulk_send_req(conn,objs,start,mx,key_buff)) return false;
            for ( size_t ix = start ; ix < mx ; ++ix ) {
                cps_api_object_t _orig = cps_api_object_list_get(objs,ix);
                if (get_object_response(conn,og.get())) {
                    if (cps_api_object_attr_merge(og.get(),_orig,true)) {
                        if (cps_api_object_clone(_orig,og.get())) continue;
                    }
                }
                return false;
            }
            return true;
        });
}

bool cps_db::get_objects_bulk(cps_db::connection &conn, std::vector<std::vector<char>> &keys, cps_api_object_list_t objs) {
    cps_api_object_guard og(cps_api_object_create());

    return cps_utils::cps_api_range_split(keys.size(),IN_THE_PIPE(),
            [&](size_t start, size_t mx) -> bool {

            if (!bulk_get_req_from_array(conn,keys,start,mx)) return false;

            for ( size_t ix = start; ix < mx ; ++ix ) {
                cps_api_object_guard og(cps_api_object_create());
                cps_db::response_set resp;
                if (get_object_response(conn,og.get())) {
                    if (cps_api_object_list_append(objs,og.get())) {
                        og.release();
                    }
                }
            }
            return true;
        });
}

bool cps_db::get_object_list(cps_db::connection &conn,cps_api_object_list_t objs) {
    cps_api_object_list_guard lg(cps_api_object_list_create());

    cps_api_object_guard og(cps_api_object_create());

    bool rc = cps_utils::cps_api_range_split(cps_api_object_list_size(objs),IN_THE_PIPE(),
            [&](size_t start, size_t mx) -> bool {
            std::vector<char> key_buff;

            if (!bulk_send_req(conn,objs,start,mx,key_buff)) return false;

            cps_api_object_guard og(cps_api_object_create());

            for ( size_t ix = start ; ix < mx ; ++ix ) {
                if (!get_object_response(conn,og.get())) {
                    continue;
                }
                if (cps_api_object_list_append(lg.get(),og.get())) {
                    og.release();
                    og.set(cps_api_object_create());
                    if (og.get()==nullptr) return false;
                }
            }
            return true;
        });
    if (!rc) return false;
    cps_api_object_list_clear(objs,true);
    return (cps_api_object_list_merge(objs,lg.get()));
}

bool cps_db::delete_object_list(cps_db::connection &conn,cps_api_object_list_t objs) {
    return cps_utils::cps_api_range_split(cps_api_object_list_size(objs),IN_THE_PIPE(),
            [&](size_t start, size_t mx) -> bool {
            for ( size_t ix = start; ix < mx ; ++ix ) {
                cps_db::connection::db_operation_atom_t e[2];
                e[0].from_string("DEL");
                e[1].from_object(cps_api_object_list_get(objs,ix),true,false);
                if (!conn.operation(e,sizeof(e)/sizeof(*e))) return false;
            }
            for ( size_t ix = start; ix < mx ; ++ix ) {
                cps_db::response_set resp;
                if (conn.response(resp,false)) {
                    continue; ///TODO ignore errors and therefore this is essentially useless - but would like to leave
                }
            }
            return true;
        });
}

bool cps_db::get_objects(cps_db::connection &conn,std::vector<char> &key,cps_api_object_list_t objs) {

    cps_db::connection_request _query(ProcessDBCache(),conn.addr());
    cps_db::connection* query = &conn;
    if (_query.valid()) {
        query = &_query.get();
    }

    std::vector<std::vector<char>> all_keys;

    size_t _count = 0;

    bool rc = true;

    cps_api_object_guard og(cps_api_object_create());

    auto _drain_queue = [&](size_t &mx) {
        for ( size_t ix = 0; ix < mx ; ++ix ) {
            if (cps_db::get_object_response(*query,og.get())) {
                if (cps_api_object_list_append(objs,og.get())) {
                    og.release();
                    og.set(cps_api_object_create());
                    if (og.get()==nullptr) break;
                    continue;
                }
            }
        }
        mx = 0;
    };

    bool walked = walk_keys(conn, &key[0],key.size(),[&](const void *key, size_t len ){
        if (!rc) return ;

        if (!cps_db::get_object_request(*query,(const char *)key,len)) {
            rc = false;
        } else ++_count;

        if (_count < IN_THE_PIPE()) return;
        _drain_queue(_count);
    });
    if (walked==false || !rc) return false;

    _drain_queue(_count);
    return true;
}
