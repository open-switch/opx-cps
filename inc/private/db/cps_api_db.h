/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_


#include "cps_api_db_connection.h"

#include "cps_api_operation.h"


#include <algorithm>
#include <functional>
#include <stddef.h>

#define CPS_DB_MAX_ITEMS_PER_SCAN "1000"
#define CPS_DB_MAX_ITEMS_PER_PIPELINE 200

namespace cps_db {



    bool dbkey_from_class_key(std::vector<char> &lst, const cps_api_key_t *key);
    bool dbkey_from_instance_key(std::vector<char> &lst,cps_api_object_t obj, bool escape);
    bool dbkey_instance_or_wildcard(std::vector<char> &lst, cps_api_object_t obj, bool &is_wildcard);

    static inline cps_api_qualifier_t QUAL(cps_api_key_t *key) { return cps_api_key_get_qual(key); }

    static inline cps_api_qualifier_t UPDATE_QUAL(cps_api_key_t *key, cps_api_qualifier_t qual) {
        cps_api_qualifier_t _cur = cps_api_key_get_qual(key);
        cps_api_key_set(key,0,qual);
        return _cur;
    }
}

/**
 * The following functions are performed on an existing connection
 */
namespace cps_db {

    bool get_sequence(cps_db::connection &conn, std::vector<char> &key, ssize_t &cntr);

    bool fetch_all_keys(cps_db::connection &conn, const void *filt, size_t flen,
            const std::function<void(const void *key, size_t klen)> &fun);

    bool select_db(cps_db::connection &conn,const std::string &id);

    bool multi_start(cps_db::connection &conn);
    bool multi_end(cps_db::connection &conn, bool commit=true);

    bool delete_object(cps_db::connection &conn,std::vector<char> &key);
    bool delete_object(cps_db::connection &conn,cps_api_object_t obj);
    bool delete_objects(cps_db::connection &conn,cps_api_object_list_t objs);

    bool store_object(cps_db::connection &conn,cps_api_object_t obj);
    bool store_objects(cps_db::connection &conn,cps_api_object_list_t objs);

    bool get_object(cps_db::connection &conn, const std::vector<char> &key, cps_api_object_t obj);
    bool get_object(cps_db::connection &conn, cps_api_object_t obj);

    bool get_objects(cps_db::connection &conn,std::vector<char> &key,cps_api_object_list_t obj_list) ;
    bool get_objects(cps_db::connection &conn, cps_api_object_t obj,cps_api_object_list_t obj_list);

    bool subscribe(cps_db::connection &conn, cps_api_object_t obj);
    bool subscribe(cps_db::connection &conn, std::vector<char> &key);

    bool publish(cps_db::connection &conn, cps_api_object_t obj);

    bool ping(cps_db::connection &conn);

    bool make_slave(cps_db::connection &conn, std::string slave_ip);
    bool remove_slave(cps_db::connection &conn);

    cps_api_return_code_t cps_api_db_init();
}

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_ */
