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

/*
 * cps_api_event_channel.cpp
 */


#include "cps_api_event_init.h"
#include "std_mutex_lock.h"
#include "cps_api_events.h"
#include "cps_class_map.h"
#include "dell-cps.h"

#include "std_rw_lock.h"
#include "std_thread_tools.h"
#include "std_error_codes.h"

#include "event_log.h"
#include "std_time_tools.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

static std_mutex_lock_create_static_init_rec(mutex);
static cps_api_event_methods_reg_t m_method;


bool cps_api_event_object_exact_match(cps_api_object_t obj, bool match_flag) {
    if (match_flag) return cps_api_object_attr_add(obj,CPS_OBJECT_GROUP_EXACT_MATCH,&match_flag,sizeof(match_flag));
    cps_api_object_attr_delete(obj,CPS_OBJECT_GROUP_EXACT_MATCH);
    return true;
}

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
    cps_api_object_list_guard lg(cps_api_object_list_create());
    for ( size_t ix = 0 ; ix < req->number_of_objects; ++ix ) {
        cps_api_object_t o = cps_api_object_list_create_obj_and_append(lg.get());
        if (o==nullptr) return cps_api_ret_code_ERR;

        cps_api_key_copy(cps_api_object_key(o),&req->objects[ix]);
    }
    return m_method.register_function_objs(handle,lg.get());
}

cps_api_return_code_t cps_api_event_client_register_object(cps_api_event_service_handle_t handle,
        cps_api_object_list_t objects) {
    return m_method.register_function_objs(handle,objects);
}

cps_api_return_code_t cps_api_event_publish(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {
    cps_api_return_code_t rc = m_method.publish_function(handle,msg);
    if (rc!=cps_api_ret_code_OK) {
        const char *_qua = cps_class_qual_from_key(cps_api_object_key(msg));
        const char *_name = cps_class_string_from_key(cps_api_object_key(msg),1);
        EV_LOG(ERR,DSAPI,0,"CPS-PUB","Failed to publish messages %s:%s",_qua!=nullptr?_qua:"unk",
                _name!=nullptr ? _name :"unk");
    }
    return rc;
}

cps_api_return_code_t cps_api_event_client_disconnect(cps_api_event_service_handle_t handle) {
    return m_method.deregister_function(handle);
}

cps_api_return_code_t cps_api_wait_for_event(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {
    return m_method.wait_for_event_function(handle,msg,CPS_API_TIMEDWAIT_NO_TIMEOUT);
}

cps_api_return_code_t cps_api_timedwait_for_event(cps_api_event_service_handle_t handle,
        cps_api_object_t msg, ssize_t timeout) {
    return m_method.wait_for_event_function(handle,msg,timeout);
}

typedef struct {
    cps_api_object_t obj;
    cps_api_event_thread_callback_t cb;
    void * context;
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
            size_t ix = 0;
            size_t mx = cb_map.size();

            for ( ; ix < mx ; ++ix ) {
                cps_api_key_t * obj_key = cps_api_object_key(obj);
                cps_api_key_t * pref = cps_api_object_key(cb_map[ix].obj);
                if (cps_api_key_matches(obj_key,pref,false)==0) {
                    auto cb = cb_map[ix].cb;
                    auto param = cb_map[ix].context;
                    std_rw_unlock(&rw_lock);
                    bool _stop = (!cb(obj,param));
                    std_rw_rlock(&rw_lock);
                    if (_stop)  {
                        char _buff[1024];
                        EV_LOGGING(DSAPI,ERR,"CPS-EVNT-THREAD","Event processing has been stopped due to negative return from CB %s",
                                cps_api_object_to_string(obj,_buff,sizeof(_buff)));
                        break;
                    }

                    mx = cb_map.size();
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
    return cps_api_ret_code_OK;
}

static pthread_once_t _once_control = PTHREAD_ONCE_INIT;

void one_time_init() {
    init_event_thread_func();
    while (true) {
        if (cps_api_event_client_connect(&_thread_handle)==cps_api_ret_code_OK) break;
        EV_LOG(ERR,DSAPI,0,"CPS-EVT-THR","Failed to create thread event handle");
        std_usleep(1000*1000*1);
    }

    is_running = true;
    std_thread_init_struct(&params);
    params.name = "local-event-thread";
    params.thread_function = _thread_function_;
    params.param = NULL;

    STD_ASSERT(std_thread_create(&params)==STD_ERR_OK);
}

cps_api_return_code_t cps_api_event_thread_reg_object(cps_api_object_list_t objects,
        cps_api_event_thread_callback_t cb, void * context ) {
    pthread_once(&_once_control,one_time_init);

    if (cps_api_event_client_register_object(_thread_handle,objects)!=cps_api_ret_code_OK) {
        return cps_api_ret_code_ERR;
    }

    cps_api_event_thread_cbs_t p ;
    p.cb = cb;
    p.context = context;
    size_t ix = 0;
    size_t mx = cps_api_object_list_size(objects);

    for ( ; ix < mx ; ++ix ) {
        p.obj = cps_api_object_create();
        if (p.obj==nullptr) {
            //memory allocation failure
            return cps_api_ret_code_ERR;
        }
        if (!cps_api_object_clone(p.obj,cps_api_object_list_get(objects,ix))) {
            cps_api_object_delete(p.obj);
            return cps_api_ret_code_ERR;
        }
        std_rw_lock_write_guard wg(&rw_lock);
        cb_map.push_back(p);
    }

    return cps_api_ret_code_OK;
}


cps_api_return_code_t cps_api_event_thread_reg(cps_api_event_reg_t * reg,
        cps_api_event_thread_callback_t cb, void * context ) {
    pthread_once(&_once_control,one_time_init);

    cps_api_object_list_guard lg(cps_api_object_list_create());
    for ( size_t ix = 0, mx = reg->number_of_objects ; ix < mx ; ++ix ) {
        cps_api_object_t o = cps_api_object_list_create_obj_and_append(lg.get());
        if (o==nullptr) return cps_api_ret_code_ERR;
        cps_api_key_copy(cps_api_object_key(o),&reg->objects[ix]);
    }

    return cps_api_event_thread_reg_object(lg.get(),cb,context);
}

cps_api_return_code_t cps_api_event_thread_publish(cps_api_object_t object) {
    pthread_once(&_once_control,one_time_init);
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
