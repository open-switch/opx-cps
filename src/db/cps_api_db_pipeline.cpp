/*
 * cps_api_db_pipeline.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: cwichmann
 */


#include "cps_api_db.h"


namespace {

using _pipeline_function = std::function<bool(cps_db::connection &conn,
		size_t ix, size_t mx, cps_api_object_list_t objs)>;

bool pipeline_loop(cps_db::connection &conn,cps_api_object_list_t objs,size_t range,
		const _pipeline_function &request,  const _pipeline_function &response) {
    size_t list_start = 0;
    size_t list_end = cps_api_object_list_size(objs);

    size_t _fill=0;
    size_t _click = range;
    for ( ; list_start < list_end ; ++list_start ) {
    	//_fill+=_click;

    	ssize_t next_step[] = { list_end - list_start , list_start + _click };

    	//is full logic - and to max without if
    	ssize_t val = ((_fill+_click) - list_end);

    	ssize_t val_sign = val/val;	//(+1 or -1) or 0
    	ssize_t _positive_val_or_zero = val_sign*val_sign;	//now 1 or 0

    	size_t _ix = list_start;
    	size_t _mx = next_step[_positive_val_or_zero];
    	if (request(conn,_ix,_mx,objs)) {
    		if (response(conn,_ix,_mx,objs)) {
    	    	list_start = _ix;
    			continue;
    		}
    	}
    	return false;
    }
    return true;
}

bool pipeline_ops(cps_db::connection &conn,cps_api_object_list_t objs) {


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



bool cps_db::delete_objects(cps_db::connection &conn,cps_api_object_list_t objs) {
	return pipeline_loop(conn,objs,1000,
			[&](cps_db::connection &conn,size_t ix, size_t mx, cps_api_object_list_t objs) -> bool {
			for ( ; ix < mx ; ++ix ) {
				cps_db::connection::db_operation_atom_t e[2];
				e[0].from_string("DEL");
				e[1].from_object(cps_api_object_list_get(objs,ix),true,false);
				if (!conn.operation(e,2,false)) return false;
			}
			return true;
		},[&](cps_db::connection &conn,	size_t ix, size_t mx, cps_api_object_list_t objs) -> bool {
		    for ( ; ix < mx ; ++ix ) {
		    	cps_db::response_set resp;
		    	if (conn.response(resp,false)) {
		    		continue;
		    	}
		    	return false;
		    }
			return true;
		});
}
