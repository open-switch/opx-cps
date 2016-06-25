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

#include "cps_api_db_operations.h"
#include "cps_api_node.h"
#include "cps_api_node_private.h"
#include "cps_api_db.h"
#include "cps_api_operation.h"
#include "dell-cps.h"

#include "cps_dictionary.h"
#include "cps_api_object.h"
#include "cps_api_object_tools.h"

#include "cps_api_node_set.h"

namespace {

cps_api_return_code_t __cps_api_db_operation_get(cps_api_object_t obj, cps_api_object_list_t results, bool stop_at_first) {
    const char * node = cps_api_key_get_group(obj);
    bool result = false;
    cps_api_node_set_iterate(node,[&obj,&results,&result,stop_at_first,node](const std::string &name,void *c){
        if (result==true && stop_at_first==true) return;
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        result |= cps_db::get_objects(r.get(),obj,results);
        if (!result) {
        	cps_api_object_t o = cps_api_object_list_create_obj_and_append(results);
        	cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY,cps_api_qualifier_OBSERVED);
        	cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME,name.c_str(),name.size()+1);
        	cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP,name.c_str(),name.size()+1);
        	cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_GROUP,node,strlen(node)+1);
        }

    },nullptr);

    return result==true? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

}

cps_api_return_code_t cps_api_db_operation_get(cps_api_get_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->filters,ix);
    STD_ASSERT(o!=nullptr);
    return __cps_api_db_operation_get(o,param->list,false);
}

cps_api_return_code_t cps_api_db_operation_commit(cps_api_transaction_params_t * param, size_t ix) {

	///TODO add watch for data modifications between get and sets
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
    STD_ASSERT(o!=nullptr);

    CPS_API_OBJECT_OWNER_TYPE_t t = (cps_api_obj_get_ownership_type(o));
    if (t!=CPS_API_OBJECT_DB) return cps_api_ret_code_ERR;

    cps_api_operation_types_t op_type = cps_api_object_type_operation(cps_api_object_key(o));
    if (op_type==cps_api_oper_ACTION) return cps_api_ret_code_ERR;

    //fetch the previous object - all should be the same so get the first one
    cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);

    std::string _failed_nodes = "";

    if (op_type==cps_api_oper_SET || op_type==cps_api_oper_DELETE) {
        cps_api_object_list_guard lg(cps_api_object_list_create());
        if (__cps_api_db_operation_get(o,lg.get(),true)==cps_api_ret_code_OK) {
            cps_api_object_t _exp_prev = cps_api_object_list_get(lg.get(),0);
            if (cps_api_object_list_size(lg.get()) > 0 ) {
				if (!cps_api_object_list_append(param->prev,_exp_prev)) {
					return cps_api_ret_code_ERR;
				}
				prev = _exp_prev;
				cps_api_object_list_remove(lg.get(),0);
            }
        }
    }

    if (op_type==cps_api_oper_SET) {
        if (!cps_api_obj_tool_merge(o,prev)) {
            return cps_api_ret_code_ERR;
        }
    }

    const char * node = cps_api_key_get_group(o);
    bool result = false;

    if (!cps_api_node_set_iterate(node,[&o,&param,&result,&ix,&op_type,&_failed_nodes](const std::string &name,void *c){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());

        //fastest operation is just store it..
        if (op_type==cps_api_oper_CREATE) {
            ///TODO need to maybe check if object currently exists before allowing create..
            result |= cps_db::store_object(r.get(),o);
            if (!result) _failed_nodes+=name+",";
            return;
        }

        if (op_type==cps_api_oper_DELETE) {
            cps_api_object_list_guard lg(cps_api_object_list_create());
            cps_db::get_objects(r.get(),o,lg.get());
            result |= cps_db::delete_objects(r.get(),lg.get());
            if (!result) _failed_nodes+=name+",";
            return;
        }

        ///TODO need to remove the group details or node details
        result |= cps_db::store_object(r.get(),o);
        if (!result) _failed_nodes+=name+",";
    },nullptr)) {
        return cps_api_ret_code_ERR;
    }
    ///TODO add send event for changed events
    if (_failed_nodes.size()!=0) cps_api_object_attr_add(o,CPS_OBJECT_GROUP_FAILED_NODES,_failed_nodes.c_str(),_failed_nodes.size()+1);
    return result==true? cps_api_ret_code_OK : cps_api_ret_code_ERR;
}

cps_api_return_code_t cps_api_db_operation_rollback(cps_api_transaction_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
    STD_ASSERT(o!=nullptr);

    cps_api_operation_types_t op_type = cps_api_object_type_operation(cps_api_object_key(o));

    cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
    if (op_type==cps_api_oper_SET) STD_ASSERT(prev!=nullptr);

    const char * node = cps_api_key_get_group(o);

    cps_api_node_set_iterate(node,[&o,&prev,op_type,&ix](const std::string &name,void *c){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());
        if (op_type==cps_api_oper_CREATE) cps_db::delete_object(r.get(),o);
        if (op_type==cps_api_oper_SET) cps_db::store_object(r.get(),prev);
        if (op_type==cps_api_oper_DELETE && prev!=nullptr) cps_db::store_object(r.get(),prev);
    },nullptr);
    ///TODO should log failure conditions even if not returned
    return cps_api_ret_code_OK ;
}
