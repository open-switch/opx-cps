/*
 * cps_api_db_connection.h
 *
 *  Created on: May 24, 2016
 *      Author: cwichmann
 */

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_

#include "cps_api_object.h"
#include "cps_api_errors.h"

#include <stddef.h>

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
                                        obj_field_CLASS
        };                                             ///types to enable class, instance or object
        obj_fields_t _atom_type=obj_fields_t::obj_field_STRING;    ///currently defined type

        void from_string(const char *str, size_t len);
        void from_string(const char *str);
        void from_object(cps_api_object_t obj, bool instance, bool data);
    };

    bool reconnect();
    bool connect(const std::string &em, const std::string &db_instance="", bool async=false);
    int get_fd();

    ~connection() { disconnect() ; }

    std::string addr() { return _addr; } //make a copy.. since reconnects could change it in the future

    void disconnect();

    bool response(std::vector<void*> &data, ssize_t at_least=0);
    bool sync_operation(db_operation_atom_t * lst,size_t len, std::vector<void*> &data);

    bool sync_operation(db_operation_atom_t * lst,size_t len, response_set &set) ;
    bool response(response_set &set, ssize_t at_least=0);

    bool operation(db_operation_atom_t * lst,size_t len, bool no_reponse=false);

    bool async_operation(db_operation_atom_t * lst,size_t len);

    bool get_event(std::vector<void*> &data);
private:
    std::string _addr ;
    bool _async=false;
    void * _ctx=nullptr;
    size_t _pending = 0;

    std::list<std::vector<void*>> _pending_events;
};

class connection_cache {
    std::mutex _mutex;
    using _conn_key = std::tuple<std::string, std::string>;
    struct _conn_key_hash : public std::unary_function<_conn_key,std::size_t>  {
        std::size_t operator()(const _conn_key &key) const {
            return std::hash<std::string>()(std::get<0>(key)) ^ std::hash<std::string>()(std::get<1>(key));
        }
    };
    using _conn_map = std::unordered_map<std::string,std::unique_ptr<connection>>;

    _conn_map _pool;
public:
    connection * get(const std::string &name);

    void put(const std::string &name, connection* conn);
    void remove(const std::string &name);
};

cps_db::connection_cache & ProcessDBCache();

class connection_request {
    std::string _name;
    cps_db::connection *_conn;
    cps_db::connection_cache &_cache;
public:
    bool valid() const { return _conn!=nullptr; }
    cps_db::connection &get() { return *_conn; }

    connection_request(cps_db::connection_cache & cache, const char *addr);
    ~connection_request() ;
};

}

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_H_ */
