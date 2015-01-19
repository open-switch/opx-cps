/* OPENSOURCELICENSE */
/*
 * cps_api_operation_example.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "cps_api_errors.h"
#include "cps_api_operation.h"
#include "std_error_codes.h"
#include "cps_api_events.h"
#include "dell-cps.h"

#include <stdlib.h>

static cps_api_operation_handle_t _handle;


static cps_api_return_code_t _read_function (void * context, cps_api_get_params_t * param,
        size_t key_ix) {
	cps_api_object_t obj = cps_api_object_create();

	cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_OBJ_LIST_IX,0);
	cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_OBJ_ID,0);
	cps_api_object_attr_add_u16(obj,OBJECT_LIST_TYPE_UINT16_TYPE_FIELD,0);
	cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_GET_ATTRIBUTE_ONLY,0);
	const char *p = "Cliff...";
	cps_api_object_attr_add(obj,OBJECT_LIST_TYPE_NAME,p,strlen(p)+1);

    cps_api_key_init(cps_api_object_key(obj),cps_api_qualifier_TARGET,cps_api_obj_cat_RESERVED,DELL_CPS_OBJECT_LIST_TYPE,0);

	if (!cps_api_object_list_append(param->list,obj)) {
		cps_api_object_delete(obj);
		return cps_api_ret_code_ERR;
	}

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _del(void * context, cps_api_object_t obj, cps_api_object_t prev) {

	return cps_api_ret_code_ERR;
}
static cps_api_return_code_t _cre(void * context, cps_api_object_t obj, cps_api_object_t prev) {

	return cps_api_ret_code_ERR;
}
static cps_api_return_code_t _set(void * context, cps_api_object_t obj, cps_api_object_t prev) {

	return cps_api_ret_code_ERR;
}
static cps_api_return_code_t _act(void * context, cps_api_object_t obj, cps_api_object_t prev) {

	return cps_api_ret_code_ERR;
}

struct {
	cps_api_operation_types_t op;
	cps_api_return_code_t (*fn)(void * context, cps_api_object_t obj, cps_api_object_t prev) ;
} list[] = {
	    {cps_api_oper_DELETE,_del },
	    {cps_api_oper_CREATE,_cre},
	    { cps_api_oper_SET, _set},    //!< set operation
	    {cps_api_oper_ACTION,_act}
};

static cps_api_return_code_t _write_function(void * context, cps_api_transaction_params_t * param,size_t ix) {
	cps_api_object_t obj = cps_api_object_list_get(param->change_list,ix);
	if (obj==NULL) return cps_api_ret_code_ERR;
	cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(obj));
	size_t fn_ix = 0;
	size_t fn_mx = sizeof(list)/sizeof(*list);
	for ( ; fn_ix < fn_mx ; ++fn_ix  ) {
		if (list[fn_ix].op==op) {
			cps_api_object_t prev = cps_api_object_create();
			if (prev==NULL) return cps_api_ret_code_ERR;
			if (!cps_api_object_list_append(param->prev,prev)) {
				cps_api_object_delete(prev);
				return cps_api_ret_code_ERR;
			}
			return list[fn_ix].fn(context,obj,prev);
		}
	}
	return cps_api_ret_code_ERR;
}

static cps_api_return_code_t _rollback_function(void * context,
		cps_api_transaction_params_t * param,
		size_t index_of_element_being_updated) {
	return cps_api_ret_code_ERR;
}

t_std_error cps_api_local_proces_init(void) {

    if (cps_api_operation_subsystem_init(&_handle,1)!=cps_api_ret_code_OK) {
        return STD_ERR(CPSNAS,FAIL,0);
    }

    cps_api_registration_functions_t f;
    memset(&f,0,sizeof(f));

    f.handle = _handle;
    f._read_function = _read_function;
    f._write_function = _write_function;
    f._rollback_function = _rollback_function;

    cps_api_key_init(&f.key,cps_api_qualifier_TARGET,cps_api_obj_cat_RESERVED,DELL_CPS_OBJECT_LIST_TYPE,0);

    cps_api_return_code_t rc = cps_api_register(&f);

    return STD_ERR_OK_IF_TRUE(rc==cps_api_ret_code_OK,STD_ERR(ROUTE,FAIL,rc));
}

int main() {
	if (cps_api_local_proces_init()!=STD_ERR_OK) {
		exit(1);
	}

    cps_api_event_service_handle_t handle;
    if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) return false;

	while (true) {
		char buff[1024];

		//used stack based for events...
		cps_api_object_t obj = cps_api_object_init(buff,sizeof(buff));

		cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_OBJ_LIST_IX,0);
		cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_OBJ_ID,0);
		cps_api_object_attr_add_u16(obj,OBJECT_LIST_TYPE_UINT16_TYPE_FIELD,0);
		cps_api_object_attr_add_u32(obj,OBJECT_LIST_TYPE_GET_ATTRIBUTE_ONLY,0);
		const char *p = "Cliff...";
		cps_api_object_attr_add(obj,OBJECT_LIST_TYPE_NAME,p,strlen(p)+1);

	    cps_api_key_init(cps_api_object_key(obj),cps_api_qualifier_TARGET,cps_api_obj_cat_RESERVED,DELL_CPS_OBJECT_LIST_TYPE,0);

	    if (cps_api_event_publish(handle,obj)!=cps_api_ret_code_OK) exit(1);

	    //stack based object no need to clean up

	}
	return 0;
}
