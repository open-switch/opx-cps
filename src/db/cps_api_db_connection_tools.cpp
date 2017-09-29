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

#include "cps_api_db_connection_tools.h"

#include "std_assert.h"
#include "event_log.h"

#include "cps_api_db.h"

#include <pthread.h>

static pthread_once_t  onceControl = PTHREAD_ONCE_INIT;
static cps_db::connection_cache * _cache;
static cps_db::connection_cache_no_validate * _event_cache;

void __init(void) {
    _cache = new cps_db::connection_cache;
    _event_cache = new cps_db::connection_cache_no_validate;
    STD_ASSERT(_cache!=nullptr && _event_cache!=nullptr);
}

cps_db::connection_cache & cps_db::ProcessDBCache() {
    pthread_once(&onceControl,__init);
    return *_cache;
}

cps_db::connection_cache_no_validate & cps_db::ProcessDBEvents() {
    pthread_once(&onceControl,__init);
    return *_event_cache;
}

bool cps_db::cps_api_db_is_local_node(const char *node) {
	bool _rc = strcmp(DEFAULT_REDIS_ADDR,node)==0;
	return _rc;	//allow for debuggers to change the value before returning
}

std::unique_ptr<cps_db::connection> cps_db::cps_api_db_create_validated_connection(const char *node) {
	size_t _countdown = 1;
	bool _is_local = cps_db::cps_api_db_is_local_node(node);

	if (_is_local) {
		_countdown = __MAX_RETRIES_FOR_LOCAL_CONNECTION__;
	}
	for ( ; _countdown > 0 ; --_countdown) {
		std::unique_ptr<cps_db::connection> _c(new cps_db::connection);
		if (_c->connect(node)) {
			if (cps_api_db_validate_connection(_c.get())) {
				return std::move(_c);
			}
		}
		if (_countdown > 1) std_usleep(MILLI_TO_MICRO(__MAX_TIME_BETWEEN_LOCAL_CONNECTION_RETRIES__));
	}
	return nullptr;
}

bool cps_db::cps_api_db_validate_connection(cps_db::connection *conn) {
	if (cps_db::cps_api_db_is_local_node(conn->addr().c_str())) {
		return conn->connection_valid();	//if the connection is wriable assume that it is all good
	}
	return cps_db::ping(*conn,__MAX_CONNECTION_TIMEOUT__);
}


cps_db::connection * cps_db::connection_cache::get(const std::string &name, bool check_alive) {
    std::lock_guard<std::mutex> l(_mutex);

    while (true) {
        auto it = _pool.find(name);
        if (it==_pool.end() || it->second.size()==0) break;

        std::unique_ptr<cps_db::connection> _conn(it->second.back().release());
        it->second.pop_back();

        if (check_alive && _conn->used_within(CONN_TIMEOUT_CHECK)) {
        	if (!cps_api_db_validate_connection(_conn.get())) {
        		EV_LOGGING(CPS,WARNING,"CPS-CONN-CACHE","Cache entry for DB connection stale for %s - getting second",it->first.c_str());
        		continue;
        	}
        }
        return _conn.release();
    }

    auto _ref = cps_api_db_create_validated_connection(name.c_str());
    return _ref.release();
}

void cps_db::connection_cache::put(const std::string &name, connection* conn) {
    std::lock_guard<std::mutex> l(_mutex);
    const static size_t MAX_OPEN_CONN=3;
    auto _ptr = std::unique_ptr<cps_db::connection>(conn);

    if (_pool[name].size()<MAX_OPEN_CONN) {
        _pool[name].push_back(std::move(_ptr));
    }
}

void cps_db::connection_cache::remove(const std::string &name) {
    std::lock_guard<std::mutex> l(_mutex);
    _pool.erase(name);
}

cps_db::connection_request::connection_request(cps_db::connection_cache & cache,const char *addr) : _cache(cache) {
    _name = addr;
    _conn = _cache.get(addr);
}

cps_db::connection_request::~connection_request() {
    if(_conn!=nullptr) {
        _cache.put(_name,_conn);
        _conn = nullptr;
    }
}
