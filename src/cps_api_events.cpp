/* OPENSOURCELICENSE */
/*
 * cps_api_event_channel.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "private/cps_api_event_init.h"
#include "std_mutex_lock.h"
#include "cps_api_events.h"

#include "std_rw_lock.h"
#include "std_thread_tools.h"
#include "std_error_codes.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>

static std_mutex_lock_create_static_init_rec(mutex);
static cps_api_event_methods_reg_t m_method;

extern "C" {

cps_api_return_code_t cps_api_event_method_register( cps_api_event_methods_reg_t * what )  {
    std_mutex_simple_lock_guard l(&mutex);
    m_method = *what;
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_event_init(void) {
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_event_client_connect(cps_api_event_service_handle_t * handle) {
    return m_method.connect_function(handle);
}

cps_api_return_code_t cps_api_event_client_register(cps_api_event_service_handle_t handle,
        cps_api_event_reg_t * req) {
    return m_method.register_function(handle,req);
}

cps_api_return_code_t cps_api_event_publish(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {
    return m_method.publish_function(handle,msg);
}

cps_api_return_code_t cps_api_event_client_disconnect(cps_api_event_service_handle_t handle) {
    return m_method.deregister_function(handle);
}

cps_api_return_code_t cps_api_wait_for_event(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {
    return m_method.wait_for_event_function(handle,msg);
}


typedef struct {
    cps_api_key_t key;
    cps_api_event_thread_callback_t cb;
    void * context;
    cps_api_event_reg_prio_t prio;
} cps_api_event_thread_cbs_t;

static std_rw_lock_t rw_lock;
static bool initied = false;
static bool is_running = false;
static cps_api_event_service_handle_t _thread_handle;
static std_thread_create_param_t params;

typedef std::vector<cps_api_event_thread_cbs_t> key_list_t;
static key_list_t cb_map;

static bool init_event_thread_func() {
    std_mutex_simple_lock_guard l(&mutex);
    if (!initied) {
        if (std_rw_lock_create_default(&rw_lock)!=STD_ERR_OK) return false;
        initied = true;
    }
    return true;
}

static  void * _thread_function_(void * param) {
    cps_api_object_t obj=cps_api_object_create();
    cps_api_object_reserve(obj,CPS_API_EVENT_THREAD_MAX_OBJ);
    while (is_running) {
        if (cps_api_wait_for_event(_thread_handle,obj)==cps_api_ret_code_OK) {
            std_rw_lock_read_guard rg(&rw_lock);
            key_list_t::iterator it = cb_map.begin();
            key_list_t::iterator end = cb_map.end();
            for ( ; it != end ; ++it ) {
                cps_api_key_t * obj_key = cps_api_object_key(obj);
                cps_api_key_t * pref = &it->key;
                if (cps_api_key_matches(obj_key,pref,false)==0) {
                    if (!it->cb(obj,it->context)) break;
                }
            }
        } else {
            //if there is a failure reading from the client - disconnect and reconnect
            //ideally this will be hidden beneath the handle itself in the very near future
            cps_api_event_client_disconnect(_thread_handle);
            cps_api_event_client_connect(&_thread_handle);
        }
    }
    return NULL;
}

cps_api_return_code_t cps_api_event_thread_init(void) {
    if (!init_event_thread_func()) return cps_api_ret_code_ERR;

    {
        std_mutex_simple_lock_guard l(&mutex);
        if (is_running) return cps_api_ret_code_OK;
        if (cps_api_event_client_connect(&_thread_handle)!=cps_api_ret_code_OK) {
            return cps_api_ret_code_ERR;
        }
        is_running=true;
    }


    std_thread_init_struct(&params);
    params.name = "local-event-thread";
    params.thread_function = _thread_function_;
    params.param = NULL;

    if (std_thread_create(&params)!=STD_ERR_OK) {
        cps_api_event_client_disconnect(_thread_handle);
        std_thread_destroy_struct(&params);
        is_running = false;
        return cps_api_ret_code_ERR;
    }

    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_event_thread_reg(cps_api_event_reg_t * reg,
        cps_api_event_thread_callback_t cb, void * context ) {

    if (cps_api_event_client_register(_thread_handle,reg)!=cps_api_ret_code_OK) {
        return cps_api_ret_code_ERR;
    }
    cps_api_event_thread_cbs_t p ;
    p.cb = cb;
    p.prio = reg->priority;
    p.context = context;
    size_t ix = 0;
    size_t mx = reg->number_of_objects;
    for ( ; ix < mx ; ++ix ) {
        cps_api_key_copy(&p.key,reg->objects+ix);
        std_rw_lock_write_guard wg(&rw_lock);
        cb_map.push_back(p);
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_event_thread_publish(cps_api_object_t object) {
    std_mutex_simple_lock_guard l(&mutex);
    return cps_api_event_publish(_thread_handle,object);
}

cps_api_return_code_t cps_api_event_thread_shutdown(void) {
    std_mutex_simple_lock_guard l(&mutex);
    std_thread_destroy_struct(&params);
    is_running=false;
    std_rw_lock_write_guard wg(&rw_lock);
    cb_map.clear();
    return cps_api_ret_code_OK;
}

}
