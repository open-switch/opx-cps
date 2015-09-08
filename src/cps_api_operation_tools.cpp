/** OPENSOURCELICENSE */
/*
 * cps_api_operation_tools.cpp
 *
 *  Created on: Sep 3, 2015
 */


#include "cps_api_object_tools.h"


extern "C" cps_api_return_code_t cps_api_commit_one(cps_api_operation_types_t type,
        cps_api_object_t obj, size_t retry_count) {

    cps_api_transaction_params_t tr;

    if (cps_api_transaction_init(&tr)!=cps_api_ret_code_OK) {
        return cps_api_ret_code_ERR;
    }
    cps_api_transaction_guard tr_g(&tr);

    cps_api_key_set_attr(cps_api_object_key(obj),type);

    if (!cps_api_object_list_append(tr.change_list,obj)) {
        return cps_api_ret_code_ERR;
    }
    bool inf = retry_count==0;

    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    while (inf || (retry_count >0) ) {
        rc = cps_api_commit(&tr);
        if (rc==cps_api_ret_code_OK) {
            //since this will be erased by the caller remove it from the transaction list
            break;
        }
        if (!inf) --retry_count;
    }
    cps_api_object_list_remove(tr.change_list,0);
    return rc;
}

extern "C" cps_api_return_code_t cps_api_get_objs(cps_api_object_t filt, cps_api_object_list_t obj_list,
        size_t retry_count) {

    cps_api_return_code_t rc;
    cps_api_get_params_t get_req;

    if ((rc=cps_api_get_request_init(&get_req))!=cps_api_ret_code_OK) return rc;

    cps_api_get_request_guard rg(&get_req);

    cps_api_object_t obj = cps_api_object_list_create_obj_and_append(get_req.filters);
    if (obj==nullptr) return cps_api_ret_code_ERR;
    if (!cps_api_object_clone(obj,filt)) return cps_api_ret_code_ERR;

    bool inf = retry_count==0;
    while (inf || (retry_count >0) ) {
        rc = cps_api_get(&get_req);
        if (rc==cps_api_ret_code_OK) break;
        if (!inf) --retry_count;
    }

    return rc;
}
