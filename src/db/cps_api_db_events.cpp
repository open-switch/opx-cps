/*
 * cps_api_db_events.cpp
 *
 *  Created on: Oct 15, 2016
 */


#include "cps_api_db.h"

#include "cps_api_db_connection.h"
#include "cps_api_db_response.h"
#include "cps_api_vector_utils.h"

#include "cps_string_utils.h"

#include "event_log.h"



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

bool cps_db::subscribe(cps_db::connection &conn, cps_api_object_t obj) {
    std::vector<char> key;
    if (!cps_db::dbkey_from_instance_key(key,obj,true)) return false;
    return subscribe(conn,key);
}

bool cps_db::publish(cps_db::connection &conn, cps_api_object_t obj) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PUBLISH");
    e[1].for_event(obj);

    response_set resp;
    if (!conn.command(e,sizeof(e)/sizeof(*e),resp)) {
        return false;
    }

    cps_db::response r = resp.get_response(0);
    bool rc = r.is_int();

    return rc;
}


