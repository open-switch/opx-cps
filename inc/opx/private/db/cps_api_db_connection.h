/*
 * Copyright (c) 2019 Dell Inc.
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

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_

#include "cps_api_db_response.h"

#include "cps_api_object.h"
#include "cps_api_errors.h"
#include "std_time_tools.h"

#include <stddef.h>

#include <inttypes.h>

#include <sys/poll.h>
#include <string>
#include <list>

#include <unordered_map>

#define DEFAULT_REDIS_PORT 6379
#define DEFAULT_REDIS_ADDR "127.0.0.1:6379"

namespace cps_db {

class response_set;

//Note.. this is not a multthread safe class - not expected to at this point.
class connection {

public:
    enum { _SELECT_MS_WAIT = (2000) };
    enum error_rc_e {     error_rc_e_timeout=1,    //timeout during communication
                        error_rc_e_channel=2,     //the specific communication channel
                        error_rc_e_response=3,    //response
    };
    struct db_operation_atom_t {
        const char *_string=nullptr;
        size_t _len=0;
        cps_api_object_t _object=nullptr;

        enum class obj_fields_t: int { obj_field_STRING, /// String or binary data
                                        obj_field_OBJ_CLASS, /// a class key
                                        obj_field_OBJ_INSTANCE,
                                        obj_field_OBJ_KEY_AND_DATA,
                                        obj_field_OBJ_KEY_AND_ALL_FIELDS,
                                        obj_field_OBJ_DATA,
                                        obj_field_CLASS,
                                        obj_field_OBJ_EVENT_DATA,
        }; ///types to enable class, instance or object
        obj_fields_t _atom_type=obj_fields_t::obj_field_STRING; ///currently defined type

        void from_string(const char *str, size_t len);
        void from_string(const char *str);
        void from_object(cps_api_object_t obj, bool instance, bool data);
        void for_event(cps_api_object_t obj);
    };

    void disconnect();
    bool reconnect();
    bool connect(const std::string &em, const std::string &db_instance="");

    bool clone(connection &conn);

    int get_fd();

    ~connection() { disconnect() ; }

    std::string addr() { return _addr; } //make a copy.. since reconnects could change it in the future

    bool command(db_operation_atom_t * lst,size_t len,response_set &set,
            size_t timeoutms=_SELECT_MS_WAIT, cps_api_return_code_t *rc=nullptr);
    bool response(response_set &data, size_t timeoutms=_SELECT_MS_WAIT,
            cps_api_return_code_t *rc=nullptr, bool _force_timeout_check=false);

    bool operation(db_operation_atom_t * lst,size_t len, bool force_flush=false,
            size_t timeoutms=_SELECT_MS_WAIT, cps_api_return_code_t *rc=nullptr);
    bool flush(size_t timeoutms=_SELECT_MS_WAIT, cps_api_return_code_t *rc=nullptr);

    /**
     * Check for any pending events, check the redis context for data and then finally read from the socket
     * to see if any event has arrived
     * @param data the reponse set to fill in
     * @param err a flag that will be set if there is any errors
     * @return true if successful otherwise false
     */
    bool get_event(response_set &data,bool &err, cps_api_return_code_t *rc=nullptr);
    /**
     * Check the redis context for data but don't read from socket - fast non-blocking call
     * @param err a flag that indicates if there was an error with the context
     * @return true if there is an event waiting otherwise false
     */
    bool has_event(bool &err, cps_api_return_code_t *rc=nullptr);

    void update_used();
    void reset_last_used() { _last_used = 0; }
    bool used_within(uint64_t relative);

    void used_for_events() { _event_connection = true; }

    bool connection_valid() {
        bool _rc = _ctx!=nullptr && writable(0);
        if (_rc) update_used();    //if the connection was tested update the validated flag
        return _rc;
    }
    bool readable(size_t timeoutms, cps_api_return_code_t *rc=nullptr);
    bool writable(size_t timeoutms, cps_api_return_code_t *rc=nullptr);
private:

    //The IP addresss
    std::string _addr ;
    std::string _local;

    void * _ctx=nullptr;// the redis context
    struct pollfd _rd, _wr;    //cache for the poll data structures on this handle
    //link list of events that were read already
    std::list<void*> _pending_events;

    // The last time the connection was used to talk to the redis server
    uint64_t _last_used=0;

    //set to true if the connection is used for events only.  This is due to the redis nature of
    //having limited command set when providing events.. also expect multiplexed events on the handle
    //e.g. send a ping request and get a event while processing the response
    bool _event_connection = false;
};

}

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_ */
