/*
 * cps_api_db.h
 *
 *  Created on: May 24, 2016
 *      Author: cwichmann
 */

#ifndef CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_
#define CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_


#include "cps_api_db_connection.h"

#include "cps_api_operation.h"


#include <algorithm>
#include <functional>
#include <stddef.h>


namespace cps_db {

    bool dbkey_from_class_key(std::vector<char> &lst, const cps_api_key_t *key);
    bool dbkey_from_instance_key(std::vector<char> &lst, cps_api_object_t obj);

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

}


namespace cps_db {
    cps_api_return_code_t cps_api_db_init();
}

#endif /* CPS_API_INC_PRIVATE_DB_CPS_API_DB_H_ */
