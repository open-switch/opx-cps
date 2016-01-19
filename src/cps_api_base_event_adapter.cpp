/*
 * filename: hal_event_service.c
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

/** OPENSOURCELICENSE */

#include "cps_api_event_init.h"
#include "private/cps_api_client_utils.h"

#include "std_event_service.h"
#include "cps_api_operation.h"
#include "cps_api_service.h"
#include "private/cps_ns.h"
#include "cps_api_service.h"
#include "std_mutex_lock.h"

#include "event_log.h"

#include "std_time_tools.h"


#include <stdlib.h>
#include <vector>
#include <unordered_map>

#define DEFAULT_DATA_LEN (1000)

using _key_t = std::array<uint8_t,sizeof(cps_api_key_t)>;

struct KeyHash {
    size_t operator()(const _key_t &a) const {
        return cps_api_key_hash((cps_api_key_t*)&(a[0])) < 0;
    }
};

struct KeyEqual {
    bool operator()(const _key_t &a,const _key_t &b) const{
        return a == b;
    }
};

static cps_api_key_t * _int_key_to_cps_key(_key_t &ek) {
    return (cps_api_key_t *)&(ek[0]);
}
static cps_api_key_t * _int_key_to_cps_key(const _key_t &ek) {
    return (cps_api_key_t *)&(ek[0]);
}

static void cps_api_to_std_key(std_event_key_t *key,const cps_api_key_t *cps_key) {
    memcpy(key->event_key,cps_api_key_elem_start((cps_api_key_t *)cps_key),
            cps_api_key_get_len((cps_api_key_t *)cps_key)* sizeof (key->event_key[0]));
    key->len = cps_api_key_get_len((cps_api_key_t *)cps_key);
}

static void _int_key_to_std_key(std_event_key_t *key,const _key_t &int_key) {
    cps_api_key_t *cps_key = _int_key_to_cps_key(int_key);
    return cps_api_to_std_key(key,cps_key);
}

static void _make_key(_key_t &ek, const cps_api_key_t *ck) {
    cps_api_key_copy(_int_key_to_cps_key(ek),(cps_api_key_t *)ck);
}

struct _reg_data {
    bool sync = false;
};

typedef struct {
    std_event_client_handle handle;
    size_t retry = 5;

    std::vector<char> buff;
    bool connected = false;
    std::unordered_map<_key_t,_reg_data,KeyHash> keys;

    std_mutex_type_t lock;

    char * get_buff(size_t len);

} cps_api_to_std_event_map_t;

char * cps_api_to_std_event_map_t::get_buff(size_t len) {
    try {
        buff.resize(len);
    } catch (...) {
        return NULL;
    }
    return &(buff[0]);
}

static inline std_event_client_handle handle_to_std_handle(cps_api_event_service_handle_t handle) {
    cps_api_to_std_event_map_t *p = (cps_api_to_std_event_map_t*) handle;
    return p->handle;
}

static inline cps_api_to_std_event_map_t* handle_to_data(cps_api_event_service_handle_t handle) {
    return (cps_api_to_std_event_map_t*) handle;
}

/*
 * The following APIs should be called by locked function calls and therefore do not
 * need additional locks
 * */

static void __close_channel(cps_api_event_service_handle_t handle) {
    if (handle_to_data(handle)->connected) {
        std_server_client_disconnect(handle_to_data(handle)->handle);
        handle_to_data(handle)->handle = -1;
        handle_to_data(handle)->connected = false;
    }
}

static void __resync_regs(cps_api_event_service_handle_t handle) {
    auto it = handle_to_data(handle)->keys.begin();
    auto end = handle_to_data(handle)->keys.end();
    std_event_key_t key;
    for ( ; it != end ; ++it ) {
        if (it->second.sync) continue;
        _int_key_to_std_key(&key,it->first);
        if (std_client_register_interest(handle_to_std_handle(handle),
                &key,1)!=STD_ERR_OK) {
            __close_channel(handle);
            break;
        }
        it->second.sync = true;
    }
}

static void __clear_reg_states(cps_api_event_service_handle_t handle) {
    auto it = handle_to_data(handle)->keys.begin();
    auto end = handle_to_data(handle)->keys.end();
    for ( ; it != end ; ++it ) {
        it->second.sync = false;
    }
}

static bool __connect_to_service(cps_api_event_service_handle_t handle) {
    if (handle_to_data(handle)->connected) return true;

    std::string ns = cps_api_user_queue(CPS_API_EVENT_CHANNEL_NAME);

    cps_api_to_std_event_map_t *p = (cps_api_to_std_event_map_t*) handle;
    if (std_server_client_connect(&p->handle,ns.c_str()) == STD_ERR_OK) {
        __clear_reg_states(p);
        p->connected = true;
    }

    if (p->connected) {
        __resync_regs(handle);
    }

    return p->connected;
}

static cps_api_return_code_t _cps_api_event_service_client_connect(cps_api_event_service_handle_t * handle) {

    cps_api_to_std_event_map_t *p = new cps_api_to_std_event_map_t;
    if (p==NULL) return cps_api_ret_code_ERR;

    std_mutex_lock_init_non_recursive(&p->lock);
    std_mutex_simple_lock_guard lg(&p->lock);
    bool rc = __connect_to_service(p);
    if (!rc) {
        EV_LOG(INFO,DSAPI,0,"CPS-EV-CONN","Not able to connect to event service - will retry based on use.");
    }
    *handle = p;
    return cps_api_ret_code_OK;
}


static cps_api_return_code_t _cps_api_event_service_client_register(
        cps_api_event_service_handle_t handle,
        cps_api_event_reg_t * req) {

    cps_api_to_std_event_map_t *p = handle_to_data(handle);
    std_mutex_simple_lock_guard lg(&p->lock);

    char _key_s[CPS_API_KEY_STR_MAX];

    size_t ix = 0;
    size_t mx = req->number_of_objects;

    for ( ; ix < mx ; ++ix ) {
        _key_t _k;
        _make_key(_k,&(req->objects[ix]));
        if (p->keys.find(_k)!=p->keys.end()) {
            EV_LOG(INFO,DSAPI,0,"CPS-EV-REG","Skipping key %s - already registered",
                    cps_api_key_print(&req->objects[ix],_key_s,sizeof(_key_s)));
            continue;
        }
        p->keys[_k].sync = false;
    }

    if (p->connected) {
        __resync_regs(p);
    }

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_event_service_publish_msg(cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    STD_ASSERT(msg!=NULL);
    STD_ASSERT(handle!=NULL);

    cps_api_key_t *_okey = cps_api_object_key(msg);

    if (cps_api_key_get_len(_okey) < CPS_OBJ_KEY_SUBCAT_POS) {
        //likely invalid message
        return cps_api_ret_code_ERR;
    }

    std_event_key_t key;
    cps_api_to_std_key(&key,_okey);

    cps_api_to_std_event_map_t *p = handle_to_data(handle);

    int retry = (int)p->retry;
    for (; retry > 0 ; --retry) {
        std_mutex_simple_lock_guard lg(&p->lock);
        if (!__connect_to_service(handle)) {
            std_usleep(MILLI_TO_MICRO(1));
            continue;
        }

        t_std_error rc = std_client_publish_msg_data(
                handle_to_std_handle(handle), &key,
                cps_api_object_array(msg),
                cps_api_object_to_array_len(msg));
        if (rc!=STD_ERR_OK) {
            __close_channel(handle);
        } else {
            return cps_api_ret_code_OK;
        }
    }
    return cps_api_ret_code_ERR;
}

static cps_api_return_code_t _cps_api_event_service_client_deregister(cps_api_event_service_handle_t handle) {
    cps_api_to_std_event_map_t * h = handle_to_data(handle);

    //no point in locking the handle on close..
    //clients will be messed up anyway if they try to have multiple threads
    //using and destroying event channels

    __close_channel(handle);
    delete h;

    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _cps_api_wait_for_event(
        cps_api_event_service_handle_t handle,
        cps_api_object_t msg) {

    std_event_msg_t m;
    while (true) {
        std_event_client_handle h;
        cps_api_to_std_event_map_t *p = handle_to_data(handle);

        {
            std_mutex_simple_lock_guard lg(&p->lock);

            if (!__connect_to_service(handle)) {
                //retry every 50ms
                std_usleep(MILLI_TO_MICRO(50));
                continue;
            }

            h = handle_to_std_handle(handle);
        }

        if (std_client_wait_for_event_data(h,&m, cps_api_object_array(msg),
                cps_api_object_get_reserve_len(msg))!=STD_ERR_OK) {
            __close_channel(handle);
            continue;
        }

        if (!cps_api_object_received(msg,m.data_len)) {
            EV_LOG(ERR,DSAPI,0,"CPS-EV-RX","Invalid message received... returning to client");
            __close_channel(handle);
            continue;
        }
        break;
    }
    return cps_api_ret_code_OK;

}

extern "C" {

static cps_api_event_methods_reg_t functions = {
        NULL,
        0,
        _cps_api_event_service_client_connect,
        _cps_api_event_service_client_register,
        _cps_api_event_service_publish_msg,
        _cps_api_event_service_client_deregister,
        _cps_api_wait_for_event
};

cps_api_return_code_t cps_api_event_service_init(void) {
    cps_api_event_method_register(&functions);
    return cps_api_ret_code_OK;
}

static std_event_server_handle_t _handle=NULL;

cps_api_return_code_t cps_api_services_start() {
    std::string ns = cps_api_user_queue(CPS_API_EVENT_CHANNEL_NAME);
    if (std_event_server_init(&_handle,ns.c_str(),CPS_API_EVENT_THREADS )!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }

    cps_api_set_cps_file_perms(ns.c_str());
    return cps_api_ret_code_OK;
}

}
