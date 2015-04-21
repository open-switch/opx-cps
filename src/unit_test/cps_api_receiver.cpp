/** OPENSOURCELICENSE */
/*
 * cps_api_receiver.cpp
 *
 *  Created on: Apr 21, 2015
 */

#include "cps_api_operation.h"
#include "std_error_codes.h"
#include "cps_api_service.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static cps_api_return_code_t db_read_function (void * context, cps_api_get_params_t * param, size_t key_ix) {
    STD_ASSERT(key_ix < param->key_count);
    cps_api_key_t * the_key = &param->keys[key_ix];

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_set_key(obj,the_key);
    cps_api_object_attr_add(obj,0,"key",strlen("key"));
    cps_api_object_attr_add(obj,1,"Interface",strlen("Interface"));
    cps_api_object_attr_add_u64(obj,2,(uint64_t)2);
    if (!cps_api_object_list_append(param->list,obj)) {
        cps_api_object_delete(obj);
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_write_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    cps_api_object_t obj = cps_api_object_list_get(param->change_list,index_of_element_being_updated);
    STD_ASSERT(obj!=NULL);

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj,&it);

    for ( ; cps_api_object_it_valid(&it) ; cps_api_object_it_next(&it) ) {
        char buff[100];
        printf("Set... Found attr %s \n",cps_api_object_attr_to_string(it.attr,buff,sizeof(buff)));
    }
    cps_api_object_t old = cps_api_object_create();
    if (!cps_api_object_clone(old,obj)) return cps_api_ret_code_ERR;
    if (!cps_api_object_list_append(param->prev,old)) {
        cps_api_object_delete(old);
        return cps_api_ret_code_ERR;
    }
    cps_api_object_attr_t attr = cps_api_object_attr_get(obj,4);
    if (attr!=NULL) {
        return cps_api_ret_code_ERR;
    }

    if (index_of_element_being_updated > 0) return cps_api_ret_code_ERR;

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t db_rollback_function(void * context, cps_api_transaction_params_t * param, size_t index_of_element_being_updated) {
    return cps_api_ret_code_OK;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Missing args...\n");
        exit(1);
    }

    if (!cps_api_unittest_init()) {
        printf("Failed to start ns\n");
        exit (1);
    }

    if (cps_api_services_start()!=cps_api_ret_code_OK) {
        printf("Failed to start event services\n");
        exit(1);
    }

    cps_api_operation_handle_t handle;
    if (cps_api_operation_subsystem_init(
            &handle, 2)!=STD_ERR_OK) {
        exit(1);
    }
    cps_api_registration_functions_t f;

    cps_api_key_t key;
    cps_api_key_from_string(&key,argv[1]);

    memset(&f,0,sizeof(f));
    f.handle = handle;
    f._read_function =db_read_function;
    f._write_function = db_write_function;
    f._rollback_function = db_rollback_function;
    cps_api_register(&f);

    while(1) sleep(1);
    return 0;
}
