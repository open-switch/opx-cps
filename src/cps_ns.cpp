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
 * cps_ns.cpp
 */


#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_object.h"
#include "cps_class_map.h"
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
#include <unordered_map>
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

using reg_cache = cps_api_key_cache<std::vector<client_reg_t>>;
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
            if (op == cps_api_ns_r_RETURN_CODE) {
                cps_api_return_code_t rc;
                if (cps_api_receive_data(sock, &rc, sizeof(rc))) {
                    EV_LOG(INFO, DSAPI, 0, "NS", "Return code: %d", rc);
                }
            }
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

    std::vector<client_reg_t> *lst = registration.at(&r.details.key,true);
    if (lst==nullptr) {
        std::vector<client_reg_t> l;
        l.push_back(r);
        registration.insert(&r.details.key,std::move(l));
    } else {
        lst->push_back(r);
    }

    reg_created_cache.insert(fd);
    }

    char buff[CPS_API_KEY_STR_MAX];
    // The raw key starts at offset 1 ( "0" being the component qualifier)
    const char *str = cps_class_string_from_key(&r.details.key, 1);
    const char *qual = cps_class_qual_from_key(&r.details.key);
    if (str!=nullptr)
      EV_LOG(INFO,DSAPI,0,"NS","%s registration for %s %s at %s",
            "Added" ,
            qual,
            str,
            r.details.addr.address.str);
    else
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
        std::vector<client_reg_t> * c = registration.at(&key,false);
        if (c!=nullptr) {
            ++((*c)[0].count);
            cpy = (*c)[0].details;
            found = true;
        }
    }
    char ink[DEF_KEY_PRINT_BUFF];//enough to handle key printing
    char matchk[DEF_KEY_PRINT_BUFF];//enough to handle key printing
    // The raw key starts at offset 1 ( "0" being the component qualifier)
    const char *str = cps_class_string_from_key(&key, 1);
    const char *qual = cps_class_qual_from_key(&key);
    if (str!=nullptr)
        EV_LOG(TRACE,DSAPI,0,"NS","NS query for %s %s found %s",
                qual,
                str,
                cps_api_key_print(&key,ink,sizeof(ink)-1),
                found ? cps_api_key_print(&cpy.key,matchk,sizeof(matchk)-1) : "missing");
    else
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
        for ( auto _key_list : it->second ) {
            for ( auto _reg_list : _key_list.data ) {
                cps_api_object_attr_add(o,cps_api_obj_stat_KEY,&_reg_list.details.key,sizeof(_reg_list.details.key));
                cps_api_object_attr_add_u64(o,cps_api_obj_stat_GET_COUNT,_reg_list.count);
            }
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
        auto &v = it->second;
        char buff[DEF_KEY_PRINT_BUFF];
        for ( size_t _keys_ix = 0, _keys_mx = v.size() ; _keys_ix < _keys_mx ; ++_keys_ix ) {
            auto &_one_key = v[_keys_ix];
            size_t _entry=0,_entry_max = _one_key.data.size();
            for ( ; _entry < _entry_max ; ++_entry ) {
                if (_one_key.data[_entry].fd == fd) {
                    send_out_key_event(&_one_key.key,false);
                    if (_one_key.data.size()>0) {
                        send_out_key_event(&_one_key.key,true);
                    }
                    // The raw key starts at offset 1 ( "0" being the component qualifier)
                    const char *str = cps_class_string_from_key(&_one_key.key, 1);
                    const char *qual = cps_class_qual_from_key(&_one_key.key);
                    if (str!=nullptr)
                        EV_LOG(INFO,DSAPI,0,"NS","Added registration removed for %s %s ",
                               qual,
                               str);
                   else
                       EV_LOG(INFO,DSAPI,0,"NS","Added registration removed %s",
                              cps_api_key_print(&_one_key.key,buff,sizeof(buff)-1));

                    _one_key.data.erase(_one_key.data.begin()+_entry);
                }
                _entry = 0;

            }
            if (_one_key.data.size()==0) {
                v.erase(v.begin()+_keys_ix);
                _keys_ix = 0;
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

    //@TODO restructure when adding the communication layer in the CPS
    if (service_data.address.type==e_std_sock_UNIX) {
        cps_api_set_cps_file_perms(service_data.address.address.str);
    }

    if (std_socket_service_run(handle)!=STD_ERR_OK) {
        std_socket_service_destroy(handle);
        return cps_api_ret_code_ERR;
    }

    return cps_api_ret_code_OK;
}

