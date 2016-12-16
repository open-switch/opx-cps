/*
 * cps_api_db_events.cpp
 *
 *  Created on: Oct 15, 2016
 */


#include "cps_api_db.h"

#include "cps_api_db_connection.h"
#include "cps_api_db_response.h"
#include "cps_api_vector_utils.h"
#include "cps_api_select_utils.h"


#include "cps_string_utils.h"

#include "event_log.h"

#include <std_condition_variable.h>
#include <mutex>
#include <pthread.h>

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

#include <condition_variable>
#include <chrono>
#include <thread>


#include <tuple>
#include <thread>

using _cps_event_queue_elem_t = std::tuple<std::string,cps_api_object_t>;
static std::vector<_cps_event_queue_elem_t> _events;

static pthread_once_t __one_time_only;
static std::mutex __mutex;
static std::condition_variable __event_wait;
static std::thread * __event_thread;

namespace {
bool _send_event(cps_db::connection *conn, cps_api_object_t obj){
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PUBLISH");
    e[1].for_event(obj);

    if (!conn->operation(e,sizeof(e)/sizeof(*e),false)) {
        return false;
    }
    return true;
}
bool _change_connection(std::string &current_address, const std::string &new_addr,
		cps_db::connection *& _conn ) {
	const std::string _db_key = current_address;
	current_address = "";
	if (_conn!=nullptr) {
		_conn->flush();	//clean up existing events
		cps_db::ProcessDBEvents().put(_db_key,_conn);	//store the handles
	}

	_conn = cps_db::ProcessDBEvents().get(new_addr);

	if (_conn==nullptr) ///TODO log error
		return false;

	current_address=new_addr;
	return true;
}

bool _drain_connection(cps_db::connection &conn) {
    cps_api_select_guard sg(cps_api_select_alloc_read());
    if (!sg.valid()) {
        return false;
    }
    size_t _fd = conn.get_fd();
    sg.add_fd(_fd);

    while (sg.get_event(0)) {
        cps_db::response_set resp;
        if (!conn.response(resp,false)) {
            conn.reconnect();
            return false;
        }
    }
    return true;
}

void __thread_main_loop() {
	std::unique_lock<std::mutex> l(__mutex);
	//uint32_t _rapid_timeout=0;

	while (true) {

		if (__event_wait.wait_for(l,std::chrono::milliseconds(1),[]()->bool{return false;})) {
			continue; //as we get new events within 1ms - don't bother flushing
		} else {
			std::string _current_addr = "";
			cps_db::connection *_conn =nullptr;

			decltype(_events) _current ;

			std::swap(_events,_current);
			l.unlock();

			bool _halted = false;
			for ( auto &it : _current ) {
				const std::string &_addr = std::get<0>(it);
				cps_api_object_t _obj = std::get<1>(it);

				if (_current_addr!=_addr) {
					if (!_change_connection(_current_addr,_addr,_conn)) {
						_halted = true;
						break;
					}
				}
				if (_conn==nullptr) {
					//STD_ASSERT(_current_addr=="");
					_halted = true;
					break;
				}

				if (!_send_event(_conn,_obj)) {
					_halted = true;
					break;
				}
				cps_api_object_delete(_obj);
			}
			if (_halted) {
				l.lock();
				_events.insert(_events.begin(),_current.begin(),_current.end());
				l.unlock();
				continue;
			}

			if (_conn!=nullptr) {
				_change_connection(_current_addr,"",_conn);
			}

			l.lock();
		}
	}

}

}

static void __cps_api_event_thread_push_init() {
	__event_thread = new std::thread([](){
		__thread_main_loop();
	});
}

bool cps_db::publish(cps_db::connection &conn, cps_api_object_t obj) {
	pthread_once(&__one_time_only,__cps_api_event_thread_push_init);
	{
		cps_api_object_guard og(cps_api_object_create_clone(obj));	///TODO better to do read-only copy on write ref
		if (og.get()==nullptr) return false;
		std::lock_guard<std::mutex> lg(__mutex);
		try {
			_events.emplace_back(conn.addr(),cps_api_object_reference(og.get(),false));
		} catch (...) {
			return false;
		}
		//_rc = (cps_api_object_list_append_copy(__pending,obj,true));
	}
	__event_wait.notify_one();

	return true;

#if 0
    cps_api_select_guard sg(cps_api_select_alloc_read());
    if (!sg.valid()) {
        return false;
    }
    size_t _fd = conn.get_fd();
    sg.add_fd(_fd);

    while (sg.get_event((size_t)~0)) {
        cps_db::response_set resp;
        if (!conn.response(resp,false)) {
            conn.reconnect();
        }
    }

    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PUBLISH");
    e[1].for_event(obj);

    if (!conn.operation(e,sizeof(e)/sizeof(*e),true)) {
        return false;
    }
#endif
    return true;
}


