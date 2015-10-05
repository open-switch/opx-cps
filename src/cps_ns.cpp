/* OPENSOURCELICENSE */
/*
 * cps_ns.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_object.h"

#include "private/cps_api_key_cache.h"
#include "private/cps_api_client_utils.h"
#include "cps_api_operation_stats.h"
#include "cps_api_events.h"
#include "cps_api_object_tools.h"

#include "std_socket_service.h"
#include "std_rw_lock.h"
#include "std_mutex_lock.h"
#include "event_log.h"
#include "std_file_utils.h"
#include "std_thread_tools.h"

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <set>

#define CPS_API_NS_ID "/tmp/cps_api_ns"

#define DEF_KEY_PRINT_BUFF (100)
#define MAX_NS_PENDING_REQUESTS (80)

struct client_reg_t {
    cps_api_object_owner_reg_t details;
    int fd;
    size_t count;
};

using reg_cache = cps_api_key_cache<client_reg_t>;
using ns_stats = std::unordered_map<uint64_t,uint64_t>;

static reg_cache registration;
static ns_stats _stats;

static std::set<int> reg_created_cache;
static std_mutex_lock_create_static_init_rec(cache_lock);

void cps_api_create_process_address(std_socket_address_t *addr) {
    addr->type = e_std_sock_UNIX;
    addr->addr_type = e_std_socket_a_t_STRING;
    static std_mutex_lock_create_static_init_fast(lock);
    std_mutex_simple_lock_guard mg(&lock);
    static uint32_t id = 0; //unique regardless of the number of times called in a
                            //process/thread

    snprintf(addr->address.str,sizeof(addr->address.str)-1,
            "/tmp/cps_inst_%d-%d-%d",(int)std_thread_id_get(),
            (int)std_process_id_get(),
            ++id);
}

void cps_api_ns_get_address(std_socket_address_t *addr) {
    addr->type = e_std_sock_UNIX;
    addr->addr_type = e_std_socket_a_t_STRING;
    std::string ns = cps_api_user_queue(CPS_API_NS_ID);
    memset(addr->address.str,0,sizeof(addr->address.str));
    strncpy(addr->address.str,ns.c_str(),
            sizeof(addr->address.str)-1);
}

bool cps_api_find_owners(cps_api_key_t *key, cps_api_object_owner_reg_t &owner) {
    std_socket_address_t addr;
    cps_api_ns_get_address(&addr);
    char buff[DEF_KEY_PRINT_BUFF];
    int sock;

    t_std_error rc = std_sock_connect(&addr,&sock);
    if (rc!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"NS","No connection to NS for %s",
                cps_api_key_print(key,buff,sizeof(buff)-1));
        return false;
    }

    bool result = false;
    do {
        if (!cps_api_send_key_request(sock,cps_api_ns_r_QUERY,*key)) {
            EV_LOG(ERR,DSAPI,0,"NS","Communication failed with NS for %s",
                    cps_api_key_print(key,buff,sizeof(buff)-1));
            break;
        }
        uint32_t op;
        size_t len;

        if (!cps_api_receive_header(sock,op,len)) {
            EV_LOG(ERR,DSAPI,0,"NS","Communication failed with NS header read for %s",
                    cps_api_key_print(key,buff,sizeof(buff)-1));
            break;
        }
        if (len!=sizeof(owner)) {
            EV_LOG(INFO,DSAPI,0,"NS","No owner found for %s",
                    cps_api_key_print(key,buff,sizeof(buff)-1));
            break;
        }
        if (!cps_api_receive_data(sock,&owner,sizeof(owner))) {
            EV_LOG(ERR,DSAPI,0,"NS","Communication failed - %s",
                    cps_api_key_print(key,buff,sizeof(buff)-1));
            break;
        }
        result = true;
    } while (0);
    close(sock);
    return result;
}

bool cps_api_ns_create_handle(cps_api_channel_t *handle) {
    std_socket_address_t addr;
    cps_api_ns_get_address(&addr);
    t_std_error rc = std_sock_connect(&addr,handle);
    if (rc!=STD_ERR_OK) return false;
    return true;
}

bool cps_api_ns_register(cps_api_channel_t handle, cps_api_object_owner_reg_t &reg) {
    if (!cps_api_send_header(handle,cps_api_ns_r_ADD,sizeof(reg))) {
        return false;
    }
    return cps_api_send_data(handle,&reg,sizeof(reg));
}

static void send_out_key_event(cps_api_key_t *key, bool how) {
    {
        std_mutex_simple_lock_guard lg(&cache_lock);
        ++_stats[cps_api_obj_stat_EVENT_SEND];
    }

    char buff[CPS_API_MIN_OBJ_LEN];
    memset(buff,0,sizeof(buff));
    cps_api_object_t obj = cps_api_object_init(buff,sizeof(buff));

    cps_api_key_copy(cps_api_object_key(obj),key);
    cps_api_key_insert_element(cps_api_object_key(obj),CPS_OBJ_KEY_INST_POS,cps_api_qualifier_REGISTRATION);

    cps_api_object_set_type_operation(cps_api_object_key(obj),
            (cps_api_operation_types_t)( how ? cps_api_oper_CREATE : cps_api_oper_DELETE));

    cps_api_event_thread_publish(obj);
}

static bool process_registration(int fd,size_t len) {
    client_reg_t r;
    r.fd = fd;
    r.count = 0;

    if (len!=sizeof(r.details)) return false;
    if (!cps_api_receive_data(fd,&r.details,sizeof(r.details))) return false;
    if (cps_api_key_get_len(&r.details.key)==0) return false;

    {
    std_mutex_simple_lock_guard lg(&cache_lock);
    ++_stats[cps_api_obj_stat_SET_COUNT];

    registration.insert(&r.details.key,r);
    reg_created_cache.insert(fd);
    }
    char buff[CPS_API_KEY_STR_MAX];
    EV_LOG(INFO,DSAPI,0,"NS","%s registration for %s at %s",
            "Added" ,
            cps_api_key_print(&r.details.key,buff,sizeof(buff)-1),
            r.details.addr.address.str);

    send_out_key_event(&r.details.key,true);
    return true;
}

static bool process_query(int fd, size_t len) {
    cps_api_key_t key;
    if (!cps_api_receive_key(fd,key)) return false;

    cps_api_object_owner_reg_t cpy;
    bool found = false;
    {
        std_mutex_simple_lock_guard lg(&cache_lock);
        ++_stats[cps_api_obj_stat_GET_COUNT];
        client_reg_t * c = registration.at(&key,false);
        if (c!=nullptr) {
            ++c->count;
            cpy = c->details;
            found = true;
        }
    }
    char ink[DEF_KEY_PRINT_BUFF];//enough to handle key printing
    char matchk[DEF_KEY_PRINT_BUFF];//enough to handle key printing
    EV_LOG(TRACE,DSAPI,0,"NS","NS query for %s found %s",
                cps_api_key_print(&key,ink,sizeof(ink)-1),
                found ? cps_api_key_print(&cpy.key,matchk,sizeof(matchk)-1) : "missing");

    if (!found) {
        return cps_api_send_return_code(fd,cps_api_ns_r_RETURN_CODE,cps_api_ret_code_ERR);
    }

    if (cps_api_send_header(fd,cps_api_ns_r_QUERY_RESULTS,sizeof(cpy))) {
        return cps_api_send_data(fd,&cpy,sizeof(cpy));
    }
    return false;
}

static bool process_stats(int fd, size_t len) {

    std_mutex_simple_lock_guard lg(&cache_lock);

    cps_api_object_guard og(cps_api_obj_tool_create(cps_api_qualifier_OBSERVED,cps_api_obj_stat_e_OPERATIONS,true));
    cps_api_object_t o = og.get();
    if (o==nullptr) return false;

    auto it = _stats.begin();
    auto end = _stats.end();
    for ( ; it != end ; ++it ) {
        cps_api_object_attr_add_u64(o,it->first,it->second);
    }

    auto fn = [fd,o](reg_cache::cache_data_iterator &it) {
        auto &v = it->second;
        size_t mx = v.size();
        for ( size_t ix = 0 ; ix < mx ; ++ix ) {
            cps_api_object_attr_add(o,cps_api_obj_stat_KEY,&v[ix].data.details.key,sizeof(v[ix].data.details.key));
            cps_api_object_attr_add_u64(o,cps_api_obj_stat_GET_COUNT,v[ix].data.count);
        }
        return true;
    };

    registration.walk(fn);
    len = o!=NULL ? cps_api_object_to_array_len(o) : 0;
    if (cps_api_send_header(fd,cps_api_msg_o_STATS,len) &&
            cps_api_send_object(fd,o)) {
        return true;
    }
    return false;
}

static bool  _some_data_( void *context, int fd ) {
    uint32_t op;
    size_t len;
    if (!cps_api_receive_header(fd,op,len)) return false;

    if (op == cps_api_ns_r_ADD) return process_registration(fd,len);
    if (op == cps_api_ns_r_QUERY) return process_query(fd,len);
    if (op==cps_api_msg_o_STATS) return process_stats(fd,len);
    return false;
}

static bool  _client_closed_( void *context, int fd ) {

    std_mutex_simple_lock_guard lg(&cache_lock);
    ++_stats[cps_api_obj_stat_CLOSE_COUNT];

    if (reg_created_cache.find(fd)==reg_created_cache.end()) return true;

    ++_stats[cps_api_obj_stat_CLOSE_CLEANUP_RUNS];

    auto fn = [fd](reg_cache::cache_data_iterator &it) {
        char buff[DEF_KEY_PRINT_BUFF];
        auto &v = it->second;
        size_t mx = v.size();
        for ( size_t ix = 0 ; ix < mx ; ++ix ) {
            if (v[ix].data.fd == fd) {
                send_out_key_event(&v[ix].key,false);
                EV_LOG(INFO,DSAPI,0,"NS","Added registration removed %s",
                            cps_api_key_print(&v[ix].key,buff,sizeof(buff)-1)
                            );

                v.erase(v.begin()+ix);

                ix = 0;
                mx = v.size();
            }
        }
        return true;
    };

    registration.walk(fn);
    reg_created_cache.erase(fd);
    return true;
}

static std_socket_server_t service_data;
static std_socket_server_handle_t handle;

cps_api_return_code_t cps_api_ns_startup() {

    if (!(cps_api_event_service_init()==cps_api_ret_code_OK &&
            cps_api_event_thread_init()==cps_api_ret_code_OK)) {
        return cps_api_ret_code_ERR;
    }

    handle = NULL;
    memset(&service_data,0,sizeof(service_data));

    service_data.name = "CPS_API_name_service";

    cps_api_ns_get_address(&service_data.address);


    service_data.thread_pool_size = 1;
    service_data.some_data = _some_data_;
    service_data.del_client = _client_closed_;
    service_data.context = &service_data;
    service_data.listeners = MAX_NS_PENDING_REQUESTS;

    if (std_socket_service_init(&handle,&service_data)!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }

    if (std_socket_service_run(handle)!=STD_ERR_OK) {
        std_socket_service_destroy(handle);
        return cps_api_ret_code_ERR;
    }

    return cps_api_ret_code_OK;
}

