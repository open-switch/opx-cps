
#include "cps_api_db_operations.h"
#include "cps_api_node.h"
#include "cps_api_node_private.h"
#include "cps_api_db.h"
#include "cps_api_operation.h"

#include "cps_dictionary.h"
#include "cps_api_object.h"
#include "cps_api_object_tools.h"

#include "cps_api_node_set.h"

cps_api_return_code_t cps_api_db_operation_get(cps_api_get_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->filters,ix);

    STD_ASSERT(o!=nullptr);
    const char * node = cps_api_key_get_group(o);

    bool result = false;

    cps_api_node_set_iterate(node,[&o,&param,&result](const std::string &name,void *c){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        result |= cps_db::get_objects(r.get(),o,param->list);
    },nullptr);

    return result==true? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

cps_api_return_code_t cps_api_db_operation_commit(cps_api_transaction_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);

    STD_ASSERT(o!=nullptr);
    const char * node = cps_api_key_get_group(o);

    bool result = false;

    CPS_API_OBJECT_STORAGE_TYPE_t t = (cps_api_obj_get_storage_type(o));
    if (t!=CPS_API_OBJECT_STORE_DB) return cps_api_ret_code_ERR;

    cps_api_node_set_iterate(node,[&o,&param,&result,&ix](const std::string &name,void *c){
        cps_api_object_guard og(cps_api_object_create());

        if (!cps_api_object_clone(og.get(),o)) { return ;}

        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());

        if (!cps_db::get_object(r.get(),og.get())) {
            return ;
        }
        cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
        if (prev==nullptr) {
            cps_api_object_list_create_obj_and_append(param->prev);
            if (!cps_api_object_clone(prev,og.get())) return ;
        }

        if (!cps_api_obj_tool_merge(og.get(),o)) {
            return ;
        }
        ///TODO need to remove the group details or node details
        result |= cps_db::store_object(r.get(),og.get());
    },nullptr);

    return result==true? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

cps_api_return_code_t cps_api_db_operation_rollback(cps_api_transaction_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);

    STD_ASSERT(o!=nullptr);
    const char * node = cps_api_key_get_group(o);

    bool result = false;

    CPS_API_OBJECT_STORAGE_TYPE_t t = (cps_api_obj_get_storage_type(o));
    if (t!=CPS_API_OBJECT_STORE_DB) return cps_api_ret_code_ERR;

    cps_api_node_set_iterate(node,[&o,&param,&result,&ix](const std::string &name,void *c){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());

        cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
        if (prev==nullptr) {
            ///ERROR log
            return;
        }
        result |= cps_db::store_object(r.get(),prev);
    },nullptr);

    return result==true? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}
