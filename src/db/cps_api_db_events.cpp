/*
 * cps_api_db_events.cpp
 *
 *  Created on: Oct 15, 2016
 */


#include "cps_api_db.h"

#include "cps_api_db_connection.h"
#include "cps_api_db_connection_tools.h"

#include "cps_api_db_response.h"
#include "cps_api_vector_utils.h"
#include "cps_api_select_utils.h"
#include "cps_class_map_query.h"

#include "cps_string_utils.h"

#include "std_condition_variable.h"
#include "std_time_tools.h"

#include "event_log.h"

#include <tuple>
#include <type_traits>

#include <pthread.h>

class _cps_event_flush {
public:
    _cps_event_flush(void) {}
    ~_cps_event_flush(void);
};

using _cps_event_queue_elem_t = std::tuple<std::string,cps_api_object_t>;
using _cps_event_queue_list_t = std::vector<_cps_event_queue_elem_t>;

#include "std_thread_tools.h"

namespace {    //as part of internal coding policies - need to use static

//Mutex handing tweaks
static size_t _MAX_ERROR_RETRY = 1000;
static size_t _MUTEX_ERROR_RETRY_DELAY= 5;
static size_t _DB_PUSH_MAX_SOCK_FAIL= 5;

//Events and event logs
static size_t _LOG_INTERVAL = 1000;
static size_t _TOTAL_MAX_INFLIGHT_EVENTS = 100000;
static size_t _TOTAL_MAX_INFLIGHT_EVENTS_REDLINE = _TOTAL_MAX_INFLIGHT_EVENTS*2;
static size_t _event_flush_timeout=3000;

static _cps_event_flush _flush_on_exit;

//List of current events
static auto _events = new _cps_event_queue_list_t;

static pthread_once_t __one_time_only;

//Mutex and condition for event list lock/signaling
static std_mutex_lock_create_static_init_rec(__mutex);
static std_condition_var_t __events;

//Statistics on pushed events
static size_t _published = 0;
static size_t _pushed = 0;

static std_thread_create_param_t _event_pushing_thread;

static void *__thread_main_loop(void *param) ;

static void __cps_api_event_thread_push_init() {
    std_condition_var_timed_init(&__events);

    std_thread_init_struct(&_event_pushing_thread) ;

    _event_pushing_thread.name = "CPS-Event-Sync";
    _event_pushing_thread.thread_function = __thread_main_loop;

    t_std_error rc = std_thread_create(&_event_pushing_thread);
    if (rc!=STD_ERR_OK) {
        EV_LOGGING(CPS,ERR,"CPS-EVENTS","Failed to create the event thread.  Resources?");
    }

    cps_api_update_ssize_on_param_change("cps.events.max-queued",
            (ssize_t*)&_TOTAL_MAX_INFLIGHT_EVENTS);
    cps_api_update_ssize_on_param_change("cps.events.queue-redline",
            (ssize_t*)&_TOTAL_MAX_INFLIGHT_EVENTS_REDLINE);
    cps_api_update_ssize_on_param_change("cps.events.flush-timeout",
            (ssize_t*)&_event_flush_timeout);
    cps_api_update_ssize_on_param_change("cps.events.log-every-x",
            (ssize_t*)&_LOG_INTERVAL);
}

//All locks must be taken
void _must_lock_mutex(std_mutex_type_t *lock) {
    size_t _errors = 0;
    int _rc = 0;
    while (true) {
        _rc = std_mutex_lock(lock);
        if (_rc==0) break;
        ++_errors;
        if (_errors>_MAX_ERROR_RETRY) {
            abort();
        }
        std_usleep(MILLI_TO_MICRO(_MUTEX_ERROR_RETRY_DELAY));
    }
}

}


bool cps_db::subscribe(cps_db::connection &conn, std::vector<char> &key) {
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PSUBSCRIBE");
    if (key[key.size()-1]!='*') cps_utils::cps_api_vector_util_append(key,"*",1);
    e[1].from_string(&key[0],key.size());

    //set the flag on the connection indicating that it will be used to handle events
    conn.used_for_events();

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

static bool _send_event(cps_db::connection *conn, cps_api_object_t obj){
    cps_db::connection::db_operation_atom_t e[2];
    e[0].from_string("PUBLISH");
    e[1].for_event(obj);

    if (!conn->operation(e,sizeof(e)/sizeof(*e),false)) {
        return false;
    }
    return true;
}

static bool _drain_connection(cps_db::connection &conn, size_t amount) {
    EV_LOGGING(CPS-DB-EV-CONN,INFO,"DRAIN-EV","Draining %d events",amount);
    size_t _err_count=0;
    for ( ; amount > 0 ; --amount ) {

        cps_api_return_code_t _rc = cps_api_ret_code_OK;
        if (!conn.readable(_event_flush_timeout,&_rc)) {
            ++_err_count;
            if (_err_count>_DB_PUSH_MAX_SOCK_FAIL && _rc==cps_api_ret_code_TIMEOUT) {
                EV_LOGGING(CPS-DB-EV-CONN,INFO,"DRAIN-EV","Draining failed - remaining %d",amount);
                return false;
            }
        }
        _err_count = 0;

        cps_db::response_set resp;
        if (!conn.response(resp,_event_flush_timeout)) {
            conn.reconnect();
            EV_LOGGING(CPS-DB-EV-CONN,INFO,"DRAIN-EV","Draining failed - remaining %d",amount);
            return false;
        }
    }
    EV_LOGGING(CPS-DB-EV-CONN,INFO,"DRAIN-EV","All events drained");
    return true;
}

static bool _change_connection(std::string &current_address, const std::string &new_addr,
        cps_db::connection *& _conn, size_t wait_for) {

    const std::string _db_key = current_address;
    current_address = "";

    if (_conn!=nullptr) {
        if (_conn->flush(_event_flush_timeout)) {
            if (!_drain_connection(*_conn, wait_for)) {
                EV_LOGGING(CPS-DB-EV-CONN,ERR,"EV-DRAIN","Failed to drain responses from server");
            }
        } else {
            EV_LOGGING(CPS-DB-EV-CONN,ERR,"EV-FLUSH","Failed to flush events to server");
        }
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

static bool _process_list(_cps_event_queue_list_t &current) {
    bool _halted = false;
    size_t _sent = 0;
    size_t _pid = getpid();
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
            if (!_change_connection(current_addr,_addr,conn,_sent)) {
                _halted = true;
                break;
            }
            _sent = 0;
        }
        if (conn==nullptr) {
            _halted = true;
            break;
        }

        cps_api_object_attr_add_u64(_obj,CPS_OBJECT_GROUP_THREAD_ID,_pid);
        cps_api_object_attr_add_u64(_obj,CPS_OBJECT_GROUP_SEQUENCE,_pushed);

        if (!_send_event(conn,_obj)) {
            _halted = true;
            break;
        }
        ++_sent;
        ++_pushed;
        cps_api_object_delete(_obj);
        std::get<1>(it) = nullptr;
    }

    if (conn!=nullptr) {
        _change_connection(current_addr,"",conn,_sent);
    }
    return !_halted;
}

static void _remove_x_events(size_t ix, size_t mx, _cps_event_queue_list_t &events) {
    const auto _events_len = events.size();
    if (mx > _events_len) {
        mx = _events_len;
    }
    for (size_t _ix=ix ; _ix < mx ; ++_ix ) {
        cps_api_object_t &_ptr = std::get<1>((events)[_ix]);
        cps_api_object_t _obj = _ptr;
        if (_obj==nullptr) continue;
        cps_api_object_delete(_obj);
        _ptr=nullptr;    //help debug issues if there is a crash due to some events not being sent/erased twice
    }
    events.erase(events.begin()+ix,events.begin()+mx);
}

//need to modify the common utilities to give simular functionality
static t_std_error _condition_var_timed_wait (std_condition_var_t* cond,
        std_mutex_type_t* mutex,
        size_t time_in_millisec,
        bool &timedout)
{
    size_t time_sec = (time_in_millisec / 1000);
    size_t time_nanosec = ((time_in_millisec % 1000) * 1000000);

    struct timespec timeout;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    timeout.tv_sec = now.tv_sec + time_sec;
    timeout.tv_nsec = now.tv_nsec + time_nanosec;
    timedout = false;

    int rc = pthread_cond_timedwait(cond, mutex, &timeout);
    if (rc == ETIMEDOUT) {
        timedout = true;
        return STD_ERR_OK;
    }
    if (rc!=0) return STD_ERR(NULL,FAIL,0);
    return STD_ERR_OK;
}

static void *__thread_main_loop(void *param) {

    std_mutex_simple_lock_guard lg(&__mutex);

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
    size_t _failed_counter = 0;

    while (true) {
        bool _timedout=false;
        int _rc = _condition_var_timed_wait(&__events,&__mutex,_current_timeout,
                _timedout);
        if (_rc!=STD_ERR_OK) {
            //note.. may happen on shutdown - needs to address in future
            ++_failed_counter;
            if (_failed_counter>_MAX_ERROR_RETRY)abort();
            std_usleep(MILLI_TO_MICRO(_MUTEX_ERROR_RETRY_DELAY));
            continue;
        }
        _failed_counter=0;
        if (!_timedout) {    //false means not timedout
            bool _handle_it = _should_process();
            if (!_handle_it) continue;
        }

        if (_events->size()==0) {
            _increase_wait_time();
            continue;
        }

        _cps_event_queue_list_t _current ;
        std::swap(*_events,_current);

        std_mutex_unlock(&__mutex);

        if (!_process_list(_current)) {    //need to handle the case of the system never working.. need to throw away events at some point
            _must_lock_mutex(&__mutex);
            if (_events->size() > (_TOTAL_MAX_INFLIGHT_EVENTS_REDLINE)) {
                //shrink it down to size of event list and the size of the new list

                auto _limit = [](_cps_event_queue_list_t &lst, size_t limit) -> void {
                    if (lst.size() > limit) {
                        size_t _len = lst.size();
                        size_t _issue = _len - limit;
                        _remove_x_events(_issue,_len,lst);
                    }
                };

                auto _val = _current.size()+_events->size()- _TOTAL_MAX_INFLIGHT_EVENTS;

                EV_LOGGING(DSAPI,ERR,"CPS-EVT","Not possible to forward events - emptying...%d events",
                        _val);
                _limit(_current,_TOTAL_MAX_INFLIGHT_EVENTS);
                _limit(*_events,(size_t)( _TOTAL_MAX_INFLIGHT_EVENTS -_current.size()));

            }
            _events->insert(_events->begin(),_current.begin(),_current.end());
            continue;    //incase we add additional logic below during health update
                        //skip back to the top
        }

        _must_lock_mutex(&__mutex);
    }
    return nullptr;
}

}

bool cps_db::publish(cps_db::connection &conn, cps_api_object_t obj) {
    pthread_once(&__one_time_only,__cps_api_event_thread_push_init);

    cps_api_object_guard og(cps_api_object_create());
    if (og.get()==nullptr) return false;
    cps_api_object_clone(og.get(),obj);

    bool _success = false;

    _must_lock_mutex(&__mutex);

    do {
        if (_events->size()>_TOTAL_MAX_INFLIGHT_EVENTS) {
            std_mutex_unlock(&__mutex);
            std_usleep(MILLI_TO_MICRO(1));
            _must_lock_mutex(&__mutex);
            continue;
        }
        try {
            _cps_event_queue_elem_t _e(conn.addr(),cps_api_object_reference(og.get(),false));
            _events->push_back(std::move(_e));
        } catch (...) {
            break;
        }
        _success=true;
    } while (0);

    if (_success) {
        ++_published;

        if ((_published%_LOG_INTERVAL)==0) {
            EV_LOGGING(CPS-DB-EV-CONN,INFO,"EVENT-PUSH","Pushed total is %d events",(int)_published);
        }
    }

    std_mutex_unlock(&__mutex);

    if (_success) (void)std_condition_var_signal(&__events);

    return _success;
}

_cps_event_flush::~_cps_event_flush() {
    _must_lock_mutex(&__mutex);
    _process_list(*_events);
}

void cps_api_event_stats() {
    printf("Sent %d events\n",(int)_published);
    printf("Flushed to DB %d events\n",(int)_pushed);
    printf("Remaining to be pushed = %d\n",(int)(_published-_pushed));
}

