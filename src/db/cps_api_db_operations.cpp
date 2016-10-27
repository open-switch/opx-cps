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
#include "cps_api_db_interface.h"

#include "cps_dictionary.h"
#include "cps_api_object.h"
#include "cps_api_object_tools.h"

#include "cps_api_node_set.h"

#include "cps_api_key_utils.h"

#include "event_log.h"


cps_api_return_code_t cps_api_db_operation_get(cps_api_get_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->filters,ix);
    STD_ASSERT(o!=nullptr);
    return cps_api_db_get(o,param->list);
}

cps_api_return_code_t cps_api_db_operation_commit(cps_api_transaction_params_t * param, size_t ix) {
    ///TODO add building of updates (and watch for changes between them)
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
    STD_ASSERT(o!=nullptr);

    //fetch the previous object - all should be the same so get the first one
    cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
    if (prev==nullptr) prev = cps_api_object_list_create_obj_and_append(param->prev);

    bool _send_event = cps_api_obj_has_auto_events(o);

    cps_api_object_set_type_operation(cps_api_object_key(o),cps_api_oper_CREATE);

    return cps_api_db_commit_one(
            cps_api_object_type_operation(cps_api_object_key(o)),
            o,prev,_send_event);
}

cps_api_return_code_t cps_api_db_operation_rollback(cps_api_transaction_params_t * param, size_t ix) {
    cps_api_object_t o = cps_api_object_list_get(param->change_list,ix);
    STD_ASSERT(o!=nullptr);

    cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
    if (prev==nullptr) return cps_api_ret_code_ERR;

    bool _send_event = cps_api_obj_has_auto_events(prev);

    cps_api_key_element_raw_value_monitor _key_patch(cps_api_object_key(prev),
            CPS_OBJ_KEY_ATTR_POS);

    cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(o));
    if (op==cps_api_oper_DELETE)op = cps_api_oper_CREATE;
    if (op==cps_api_oper_CREATE) op = (cps_api_oper_DELETE);

    return cps_api_db_commit_one(op,prev,nullptr,_send_event);
}
