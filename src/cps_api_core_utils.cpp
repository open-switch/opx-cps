
#include "cps_api_core_utils.h"

#include "cps_api_db_connection.h"
#include "cps_api_db.h"

bool cps_api_core_publish(cps_api_object_t obj) {
    cps_db::connection_request r(cps_db::ProcessDBCache(),DEFAULT_REDIS_ADDR);
    return cps_db::publish(r.get(),obj);
}

