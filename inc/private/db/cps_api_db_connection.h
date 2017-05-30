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


#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_

#include "cps_api_object.h"
#include "cps_api_errors.h"
#include "std_time_tools.h"

#include <stddef.h>

#include <inttypes.h>

#include <vector>
#include <string>
#include <mutex>
#include <memory>
#include <list>

#include <unordered_map>

#define DEFAULT_REDIS_PORT 6379
#define DEFAULT_REDIS_ADDR "127.0.0.1:6379"

namespace cps_db {

class response_set;

//Note.. this is not a multthread safe class - not expected to at this point.
class connection {
public:
    struct db_operation_atom_t {
        const char *_string=nullptr;
        size_t _len=0;
        cps_api_object_t _object=nullptr;

        enum class obj_fields_t: int {     obj_field_STRING,    /// String or binary data
                                        obj_field_OBJ_CLASS,/// a class key
                                        obj_field_OBJ_INSTANCE,
                                        obj_field_OBJ_KEY_AND_DATA,
                                        obj_field_OBJ_KEY_AND_ALL_FIELDS,
                                        obj_field_OBJ_DATA,
                                        obj_field_CLASS,
                                        obj_field_OBJ_EVENT_DATA,
        };                                             ///types to enable class, instance or object
        obj_fields_t _atom_type=obj_fields_t::obj_field_STRING;    ///currently defined type

        void from_string(const char *str, size_t len);
        void from_string(const char *str);
        void from_object(cps_api_object_t obj, bool instance, bool data);
        void for_event(cps_api_object_t obj);
    };

    void disconnect();
    bool reconnect();
    bool connect(const std::string &em, const std::string &db_instance="", bool async=false);
    bool clone(connection &conn);

    int get_fd();

    ~connection() { disconnect() ; }

    std::string addr() { return _addr; } //make a copy.. since reconnects could change it in the future


    bool command(db_operation_atom_t * lst,size_t len,response_set &set);

    bool response(response_set &data, bool expect_events = false);

    bool operation(db_operation_atom_t * lst,size_t len, bool force_flush=false);
    bool flush();

    bool get_event(response_set &data,bool &err);
    bool has_event(bool &err);

    void update_used();
    bool timedout(uint64_t relative);

private:
    std::string _addr ;
    bool _async=false;
    void * _ctx=nullptr;

    std::list<void*> _pending_events;
    uint64_t _last_used=0;

};

class connection_cache {
    enum { CONN_TIMEOUT_CHECK=MILLI_TO_MICRO(10*60*1000) };
    std::mutex _mutex;
    using _conn_key = std::tuple<std::string, std::string>;
    struct _conn_key_hash : public std::unary_function<_conn_key,std::size_t>  {
        std::size_t operator()(const _conn_key &key) const {
            return std::hash<std::string>()(std::get<0>(key)) ^ std::hash<std::string>()(std::get<1>(key));
        }
    };
    using _conn_map = std::unordered_map<std::string,std::vector<std::unique_ptr<connection>>>;

    _conn_map _pool;
public:
    connection * get(const std::string &name, bool check_alive=true);

    void put(const std::string &name, connection* conn);
    void remove(const std::string &name);

    void flush_pending();
};

class connection_cache_events : public connection_cache {
public:
    connection * get(const std::string &name) {
        return connection_cache::get(name,false);
    }
};

cps_db::connection_cache & ProcessDBCache();
cps_db::connection_cache_events & ProcessDBEvents();

class connection_request {
    std::string _name;
    cps_db::connection *_conn;
    cps_db::connection_cache &_cache;
public:
    bool valid() const { return _conn!=nullptr; }
    cps_db::connection &get() { return *_conn; }

    connection_request(cps_db::connection_cache & cache, const char *addr);
    connection_request(cps_db::connection_cache & cache, const std::string &addr) :
        connection_request(cache,addr.c_str()) {}

    ~connection_request() ;
};

}

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_ */
