/* OPENSOURCELICENSE */
/*
 * cps_ns.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "private/cps_ns.h"
#include "cps_api_operation.h"
#include "cps_api_object.h"
#include "std_socket_service.h"
#include "std_rw_lock.h"
#include "std_mutex_lock.h"
#include "event_log.h"
#include "std_file_utils.h"
#include "std_thread_tools.h"
#include "private/cps_api_client_utils.h"

#include <string.h>
#include <stdio.h>
#include <vector>
#include <string>

#define CPS_API_NS_ID "/tmp/cps_api_ns"


#define DEF_KEY_PRINT_BUFF (100)

struct client_reg_t {
    cps_api_object_owner_reg_t details;
    int fd;
    size_t count;
};

typedef std::vector<client_reg_t> registrations_t;
static registrations_t active_registraitons;

enum cps_api_ns_reg_t {
    cps_api_ns_r_ADD,
    cps_api_ns_r_DEL,
    cps_api_ns_r_QUERY,
    cps_api_ns_r_QUERY_RESULTS,
    cps_api_ns_r_RETURN_CODE
};

void cps_api_create_process_address(std_socket_address_t *addr) {
    addr->type = e_std_sock_UNIX;
    addr->addr_type = e_std_socket_a_t_STRING;

    snprintf(addr->address.str,sizeof(addr->address.str)-1,
            "/tmp/cps_inst_%d_%d",(int)std_thread_id_get(),(int)std_process_id_get());
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

static bool process_registration(int fd,size_t len) {
    client_reg_t r;
    r.fd = fd;
    r.count = 0;
    if (len!=sizeof(r.details)) return false;
    if (!cps_api_receive_data(fd,&r.details,sizeof(r.details))) return false;
    active_registraitons.push_back(r);
    char buff[100];

    EV_LOG(INFO,DSAPI,0,"NS","Added registration for %s",cps_api_key_print(&r.details.key,buff,sizeof(buff)-1));
    return true;
}

static cps_api_object_owner_reg_t * find_owner(cps_api_key_t &key) {
    size_t ix = 0;
    size_t mx = active_registraitons.size();
    for ( ; ix < mx ; ++ix ) {
        if (cps_api_key_matches(&key,&active_registraitons[ix].details.key,false)==0) {
            ++active_registraitons[ix].count;
            return &active_registraitons[ix].details;
        }
    }
    return NULL;
}

static bool process_query(int fd, size_t len) {
    cps_api_key_t key;
    if (!cps_api_receive_key(fd,key)) return false;
    cps_api_object_owner_reg_t *p = find_owner(key);

    {
	char ink[DEF_KEY_PRINT_BUFF];//enough to handle key printing
	char matchk[DEF_KEY_PRINT_BUFF];//enough to handle key printing
	EV_LOG(TRACE,DSAPI,0,"NS","NS query for %s found %s",
    		cps_api_key_print(&key,ink,sizeof(ink)-1),
			p!=NULL ? cps_api_key_print(&p->key,matchk,sizeof(matchk)-1) : "missing");
    }

    if (p==NULL) {
        return cps_api_send_return_code(fd,cps_api_ns_r_RETURN_CODE,cps_api_ret_code_ERR);
    }

    if (cps_api_send_header(fd,cps_api_ns_r_QUERY_RESULTS,sizeof(*p))) {
        return cps_api_send_data(fd,p,sizeof(*p));
    }
    return false;
}

static bool  _some_data_( void *context, int fd ) {
    uint32_t op;
    size_t len;
    if (!cps_api_receive_header(fd,op,len)) return false;

    if (op == cps_api_ns_r_ADD) return process_registration(fd,len);
    if (op == cps_api_ns_r_QUERY) return process_query(fd,len);

    return false;
}

static bool  _client_closed_( void *context, int fd ) {
    size_t ix = 0;
    size_t mx = active_registraitons.size();
    char buff[DEF_KEY_PRINT_BUFF];//enough to handle key printing
    while (ix < mx) {
        if (active_registraitons[ix].fd!=fd) {
            ++ix;
            continue;
        }

        EV_LOG(INFO,DSAPI,0,"NS","Added registration removed %s",
                    cps_api_key_print(&active_registraitons[ix].details.key,buff,sizeof(buff)-1)
                    );

        active_registraitons.erase(active_registraitons.begin()+ix);
        ix = 0;
        mx = active_registraitons.size();
    }
    return true;
}

static std_socket_server_t service_data;
static std_socket_server_handle_t handle;

cps_api_return_code_t cps_api_ns_startup() {

    handle = NULL;
    memset(&service_data,0,sizeof(service_data));

    service_data.name = "CPS_API_name_service";

    cps_api_ns_get_address(&service_data.address);


    service_data.thread_pool_size = 1;
    service_data.some_data = _some_data_;
    service_data.del_client = _client_closed_;
    service_data.context = &service_data;

    if (std_socket_service_init(&handle,&service_data)!=STD_ERR_OK) {
        return cps_api_ret_code_ERR;
    }

    if (std_socket_service_run(handle)!=STD_ERR_OK) {
        std_socket_service_destroy(handle);
        return cps_api_ret_code_ERR;
    }

    return cps_api_ret_code_OK;
}
