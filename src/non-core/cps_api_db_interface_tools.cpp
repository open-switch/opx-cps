

#include "cps_api_db_interface.h"

cps_api_return_code_t cps_api_config_db_write(cps_api_object_t obj) {
    cps_api_return_code_t rc = cps_api_ret_code_OK;
    cps_api_key_t *key = cps_api_object_key(obj);

    cps_api_qualifier_t _qual = cps_api_key_get_qual(key);

    CPS_CONFIG_TYPE_t type = cps_api_object_get_config_type(obj);
    cps_api_operation_types_t op = cps_api_object_type_operation(key);

    if ((type == CPS_CONFIG_TYPE_RUNNING_CONFIG) ||
            (type == CPS_CONFIG_TYPE_STARTUP_AND_RUNNING) ||
            (type == CPS_CONFIG_TYPE_RUNNING_CONFIG_ONLY)) {
        cps_api_key_set_qualifier(key, cps_api_qualifier_RUNNING_CONFIG);
        rc = cps_api_db_commit_one(op, obj, NULL, true);
        if (rc != cps_api_ret_code_OK) return rc;
    }
    if ((type == CPS_CONFIG_TYPE_STARTUP_CONFIG) ||
            (type == CPS_CONFIG_TYPE_STARTUP_AND_RUNNING)) {
        cps_api_key_set_qualifier(key, cps_api_qualifier_STARTUP_CONFIG);
        rc = cps_api_db_commit_one(op, obj, NULL, true);
        if (rc != cps_api_ret_code_OK) return rc;
    }
    cps_api_key_set_qualifier(key,_qual);
    return rc;
}

