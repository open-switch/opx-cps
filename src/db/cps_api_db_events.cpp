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

/**
 * @file cps_api_db_events.cpp
 */


#include "cps_api_db.h"

#include "cps_api_db_connection.h"
#include "cps_api_db_response.h"
#include "cps_api_vector_utils.h"
#include "cps_api_select_utils.h"

#include "cps_string_utils.h"

#include "event_log.h"
#include "std_time_tools.h"

#include <type_traits>
#include <std_condition_variable.h>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <tuple>
#include <thread>
#include <chrono>


class _cps_event_flush {
public:
	_cps_event_flush() {}
	~_cps_event_flush();
};

using _cps_event_queue_elem_t = std::tuple<std::string,cps_api_object_t>;
using _cps_event_queue_list_t = std::vector<_cps_event_queue_elem_t>;

namespace {
const size_t _TOTAL_MAX_INFLIGHT_EVENTS = 100000;

_cps_event_flush _flush_on_exit;

auto _events = new _cps_event_queue_list_t;

pthread_once_t __one_time_only;
std::mutex __mutex;
std::condition_variable __event_wait;
std::thread * __event_thread;
size_t _published = 0;
size_t _pushed = 0;

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

bool cps_db::subscribe(cps_db::connection &conn, cps_api_object_t obj) {
    std::vector<char> key;
    if (!cps_db::dbkey_from_instance_key(key,obj,true)) return false;
    return subscribe(conn,key);
}


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

bool _change_connection(std::string &current_address, const std::string &new_addr,
        cps_db::connection *& _conn ) {

    const std::string _db_key = current_address;
    current_address = "";

    if (_conn!=nullptr) {
        _conn->flush();    //clean up existing events
        _drain_connection(*_conn);

        cps_db::ProcessDBEvents().put(_db_key,_conn);    //store the handles
    }

    if (new_addr.size()==0) {
        return true;
    }

    _conn = cps_db::ProcessDBEvents().get(new_addr);

    if (_conn==nullptr) {
        return false;
    }

    current_address=new_addr;
    return true;
}

bool _process_list(_cps_event_queue_list_t &current) {
    bool _halted = false;
    std::string current_addr="";
    cps_db::connection *conn=nullptr;

    for ( auto &it : current ) {
        std::string _addr = std::get<0>(it);
        cps_api_object_t _obj = std::get<1>(it);
        if (_obj==nullptr) continue;

        if (_addr.size()==0 ) {
            _addr = DEFAULT_REDIS_ADDR;
            EV_LOGGING(DSAPI,DEBUG,0,"No address provided - using default localhost");
        }

        if (current_addr!=_addr) {
            if (!_change_connection(current_addr,_addr,conn)) {
                _halted = true;
                break;
            }
        }
        if (conn==nullptr) {
            _halted = true;
            break;
        }

        if (!_send_event(conn,_obj)) {
            _halted = true;
            break;
        }
        ++_pushed;
        cps_api_object_delete(_obj);
        std::get<1>(it) = nullptr;
    }

    if (conn!=nullptr) {
        _change_connection(current_addr,"",conn);
    }
    return !_halted;
}

void _remove_x_events(size_t ix, size_t mx, _cps_event_queue_list_t &events) {
	for (size_t _ix=ix ; _ix < mx ; ++_ix ) {
		cps_api_object_t &_ptr = std::get<1>((events)[_ix]);
		cps_api_object_t _obj = _ptr;
		if (_obj==nullptr) continue;
		cps_api_object_delete(_obj);
		_ptr=nullptr;	//help debug issues if there is a crash due to some events not being sent/erased twice
	}
	events.erase(events.begin()+ix,events.begin()+mx);
}

void __thread_main_loop() {
    std::unique_lock<std::mutex> l(__mutex);
    //uint32_t _rapid_timeout=0;

    size_t _MAX_TIMEOUT=1000;
    size_t _current_timeout=_MAX_TIMEOUT;

    static const size_t _MAX_PENDING=50;

    auto _should_process = [&] () -> bool {
        _current_timeout = 1;
        return _events->size() > _MAX_PENDING;
    };
    auto _increase_wait_time = [&]() {
        _current_timeout <<=1;
        if (_current_timeout > _MAX_TIMEOUT) _current_timeout = _MAX_TIMEOUT;
    };

    while (true) {///TODO need to use a incremental profile for chrono because this one can be effected by TOD issues... yuck
        std::cv_status _rc =__event_wait.wait_for(l,std::chrono::milliseconds(_current_timeout));
        if (_rc==std::cv_status::no_timeout) {
            bool _handle_it = _should_process();
            if (!_handle_it) continue;
        }
        if (_events->size()==0) {
            _increase_wait_time();
            continue;
        }

        _cps_event_queue_list_t _current ;
        std::swap(*_events,_current);

        l.unlock();

        if (!_process_list(_current)) {    //need to handle the case of the system never working.. need to throw away events at some point
            l.lock();
        	if (_events->size()>(_TOTAL_MAX_INFLIGHT_EVENTS*2)) {
        		size_t mx = _events->size()-_TOTAL_MAX_INFLIGHT_EVENTS;
        		EV_LOGGING(DSAPI,ERR,0,"CPS-EVT","Not possible to forward events - emptying...%d events",mx);
        		_remove_x_events(0,mx,*_events);
        	}
            _events->insert(_events->begin(),_current.begin(),_current.end());
            l.unlock();
            continue;
        }

        l.lock();
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
	cps_api_object_guard og(cps_api_object_create());
	if (og.get()==nullptr) return false;
	cps_api_object_clone(og.get(),obj);

	std::lock_guard<std::mutex> lg(__mutex);
	while (_events->size()>_TOTAL_MAX_INFLIGHT_EVENTS) {
		__mutex.unlock();
		std_usleep(MILLI_TO_MICRO(1));
		__mutex.lock();
	}
	try {
		_events->emplace_back(conn.addr(),cps_api_object_reference(og.get(),false));
	} catch (...) {
		return false;
	}
    }
    __event_wait.notify_one();
    ++_published;
    return true;
}

_cps_event_flush::~_cps_event_flush() {
	std::unique_lock<std::mutex> l(__mutex);
	_process_list(*_events);
}

void cps_api_event_stats() {
    printf("Sent %d events\n",(int)_published);
    printf("Flushed to DB %d events\n",(int)_pushed);
    printf("Remaining to be pushed = %d\n",(int)(_published-_pushed));
}

