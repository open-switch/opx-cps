/*
 * cps_api_db_connection_tools.h
 *
 *  Created on: Jul 9, 2017
 *      Author: cwichmann
 */

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_TOOLS_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_TOOLS_H_

#include "cps_api_db_connection.h"

#include <algorithm>
#include <tuple>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

#define __MAX_RETRIES_FOR_LOCAL_CONNECTION__ (15*2)	//15 seconds (with 500ms between)
#define __MAX_TIME_BETWEEN_LOCAL_CONNECTION_RETRIES__ (500) // wait 500ms between retries
#define __MAX_CONNECTION_TIMEOUT__ (500)

namespace cps_db {

/**
 * Check to see if the connection is a local node (local ip and local db port for redis server)
 * NOTE: need to update this if we ever use non-standard ports (eg.. change the define)
 * @param addr the [ip address:port] string
 * @return true if is a local DB connection otherwise false
 */
bool cps_api_db_is_local_node(const char *addr);

/**
 * Validate the connection being passed in.  If it is a local DB.. then this will be a alive test only otherwise
 * it will be a deeper DB underlying ping request
 * @param conn the connection to test
 * @return true if validate otherwise false
 */
bool cps_api_db_validate_connection(cps_db::connection *conn);

/**
 * Allocate a connection and ensure it is connected properly.  This will also doa cps_api_db_validate_connection
 * on the newly created connection to validate it
 * @param node the IP:port combination
 * @return a unique_ptr containing the valid connection or a nullptr
 */
std::unique_ptr<connection> cps_api_db_create_validated_connection(const char *node);

class connection_cache {
    enum { CONN_TIMEOUT_CHECK=MILLI_TO_MICRO(30*1000) };	//default timeout is 30 seconds
    std::mutex _mutex;
    using _conn_map = std::unordered_map<std::string,std::vector<std::unique_ptr<connection>>>;
    _conn_map _pool;
public:
    connection * get(const std::string &name, bool check_alive=true);

    void put(const std::string &name, connection* conn);
    void remove(const std::string &name);

    void flush_pending();
};

class connection_cache_no_validate : public connection_cache {
public:
    connection * get(const std::string &name) {
        return connection_cache::get(name,false);
    }
};

cps_db::connection_cache & ProcessDBCache();
cps_db::connection_cache_no_validate & ProcessDBEvents();

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


#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_CONNECTION_TOOLS_H_ */
