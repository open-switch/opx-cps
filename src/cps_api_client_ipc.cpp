/* OPENSOURCELICENSE */
/*
 * cps_api_client_ipc.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "private/cps_api_client_utils.h"
#include "private/cps_api_key_cache.h"
#include "private/cps_ns.h"

#include "cps_api_object.h"


#include "event_log.h"
#include "std_file_utils.h"
#include "std_select_tools.h"
#include "std_time_tools.h"
#include "std_mutex_lock.h"

#include <unistd.h>

#define SCRATCH_LOG_BUFF (100)

struct cps_msg_hdr_t {
    uint32_t version;
    uint32_t len;
    uint32_t operation;
};

struct cache_entry {
    cps_api_object_owner_reg_t owner;
    uint64_t last_updated;
};
using key_cache = cps_api_key_cache<cache_entry>;

static key_cache _cache;
static std_mutex_lock_create_static_init_fast(cache_lock);


static void fill_cache(cps_api_key_t *key,const cps_api_object_owner_reg_t &owner) {
    std_mutex_simple_lock_guard lg(&cache_lock);
    _cache.erase(key);
    cache_entry ce ;
    ce.owner = owner;
    ce.last_updated = std_get_uptime(nullptr);
    _cache.insert(key,ce);
}

void cps_api_disconnect_owner(cps_api_channel_t handle) {
    close(handle);
}

cps_api_return_code_t cps_api_connect_owner(cps_api_object_owner_reg_t*o,cps_api_channel_t &handle) {
    t_std_error rc = std_sock_connect(&o->addr,&handle);
    if (rc!=STD_ERR_OK) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","not able to connect to owner");
    }
    return (cps_api_return_code_t) rc;
}

static bool cache_connect(cps_api_key_t &key, cps_api_channel_t &handle) {
    return false;
    std_mutex_simple_lock_guard lg(&cache_lock);
    cache_entry ce;
    if (_cache.find(&key,ce,true)) {
        const static int TM = MILLI_TO_MICRO(1000) * 20; //20 seconds
        bool expired = (std_get_uptime(nullptr) - ce.last_updated) > TM;
        if (!expired && (cps_api_connect_owner(&ce.owner, handle)==cps_api_ret_code_OK)) {
            return true;
        } else {
            char buff[SCRATCH_LOG_BUFF];
            EV_LOG(TRACE,DSAPI,0,"CPS-NS-CACHE","cache expired for key:%s",
                    cps_api_key_print(&key,buff,sizeof(buff)));
            _cache.erase(&key);
        }
    }
    return false;
}

bool cps_api_get_handle(cps_api_key_t &key, cps_api_channel_t &handle) {
    if (cache_connect(key,handle)) {
        return true;
    }

    cps_api_object_owner_reg_t owner;
    if (!cps_api_find_owners(&key,owner)) return false;
    bool rc = (cps_api_connect_owner(&owner, handle))==cps_api_ret_code_OK;
    if (!rc) {
        char buff[SCRATCH_LOG_BUFF];
        EV_LOG(ERR,DSAPI,0,"NS","Could not connect with owner for %s (%s)",
                cps_api_key_print(&key,buff,sizeof(buff)-1),
                owner.addr.addr_type== e_std_socket_a_t_STRING ?
                        owner.addr.address.str: "unk");
    }
    if (rc) {
        fill_cache(&key,owner);
    }
    return rc;
}

bool cps_api_send(cps_api_channel_t handle, cps_api_msg_operation_t op,
        const struct iovec *iov, size_t count) {

    cps_msg_hdr_t hdr;
    hdr.len =0;
    size_t ix = 0;
    for ( ; ix < count ; ++ix ) {
        hdr.len+=iov[ix].iov_len;
    }
    hdr.version = 0;
    hdr.operation = op;

    t_std_error rc = STD_ERR_OK;
    int by = std_write(handle,&hdr,sizeof(hdr),true,&rc);
    if (by!=sizeof(hdr)) return false;
    for ( ix = 0; ix < count ; ++ix ) {
        by = std_write(handle,iov[ix].iov_base,iov[ix].iov_len,true,&rc);
        if (by!=(int)iov[ix].iov_len) return false;
    }
    return true;
}

bool cps_api_send_header(cps_api_channel_t handle, uint32_t op,
        size_t len) {
    cps_msg_hdr_t hdr;
    hdr.len =len;
    hdr.operation = op;
    hdr.version = 0;
    t_std_error rc = STD_ERR_OK;
    int by = std_write(handle,&hdr,sizeof(hdr),true,&rc);
    if (by!=sizeof(hdr)) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to send the full message header.");
    }
    return (by==sizeof(hdr)) ;
}

bool cps_api_send_data(cps_api_channel_t handle, void *data, size_t len) {
    t_std_error rc = STD_ERR_OK;
    int by = std_write(handle,data,len,true,&rc);
    if (by != (int)len) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to send the full message body.");
    }
    return (by==(int)len) ;
}

 bool cps_api_send_key(cps_api_channel_t handle, cps_api_key_t &key) {
    return cps_api_send_data(handle,&key,sizeof(key));
}

 bool cps_api_send_object(cps_api_channel_t handle, cps_api_object_t obj) {
    t_std_error rc = STD_ERR_OK;
    size_t len = cps_api_object_to_array_len(obj);
    int by = std_write(handle,cps_api_object_array(obj),len, true,&rc);
    if (by != (int)len) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to send the full object.");
    }
    return (by==(int)len) ;
}

 bool cps_api_receive_header(cps_api_channel_t handle, uint32_t &op,
        size_t &len) {
    cps_msg_hdr_t hdr;

    t_std_error rc = STD_ERR_OK;
    int by = std_read(handle,&hdr,sizeof(hdr),true,&rc);
    if (by!=sizeof(hdr)) {
        EV_LOG(TRACE,DSAPI,0,"CPS IPC","Was not able to read the header.");
        return false;
    }
    op = hdr.operation;
    len = hdr.len;
    return true;
}

 bool cps_api_receive_data(cps_api_channel_t handle, void *data, size_t len) {
    t_std_error msg_rc = STD_ERR_OK;
    int by = std_read(handle,data,len,true,&msg_rc);
    bool rc = (by==(int)len) ;
    if (!rc) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to read the data. (%X)",msg_rc);
    }
    return rc;
}

 bool cps_api_receive_key(cps_api_channel_t handle, cps_api_key_t &key) {
    return cps_api_receive_data(handle,&key,sizeof(key));
}

 cps_api_object_t cps_api_receive_object(cps_api_channel_t handle,size_t len) {
    if (len==0) return NULL;

    cps_api_object_guard og(cps_api_object_create());
    t_std_error rc = STD_ERR_OK;

    do {
        if (cps_api_object_get_reserve_len(og.get()) < len) {
            if (!cps_api_object_reserve(og.get(),len)) break;
        }

        int by = std_read(handle,cps_api_object_array(og.get()),len,true,&rc);
        if (by!=(int)len) break;

        if (!cps_api_object_received(og.get(),len)) break;
        return og.release();

    } while (0);
    EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to read the object - MSG (%X)",rc);
    return NULL;
}


 cps_api_object_t cps_api_recv_one_object(cps_api_channel_t handle,
         cps_api_msg_operation_t expected, bool &valid) {
     uint32_t act;
     size_t len;

     if (!cps_api_receive_header(handle,act,len)) {
         valid = false;
         return NULL;
     }
     if (act!=expected) return NULL;

     if (len == 0) {
         valid = true;
         return NULL;
     }

     //receive an object and set valid true if the object was returned
     cps_api_object_t ptr = cps_api_receive_object(handle,len);
     valid = ptr!=NULL;
     return ptr;
 }

cps_api_return_code_t cps_api_timeout_wait(int handle, fd_set *r_template, size_t timeout_ms,const char *op) {
    fd_set _rset = *r_template;
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = MILLI_TO_MICRO(timeout_ms % 1000);    //just the milliseconds portion
    int rc = std_select_ignore_intr(handle+1,&_rset,nullptr,nullptr,&tv,nullptr);
    if (rc==-1) {
        EV_LOG(ERR,DSAPI,0,"CPS-OP-GET","CPS Operation failed - application close");
        return cps_api_ret_code_ERR;
    }
    if (rc==0) {
        EV_LOG(ERR,DSAPI,0,"CPS-OP-GET","CPS Operation failed - application time out");
        return cps_api_ret_code_ERR;
    }
    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_process_get_request(cps_api_get_params_t *param, size_t ix) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    char buff[SCRATCH_LOG_BUFF];
    cps_api_channel_t handle;
    cps_api_key_t *key = cps_api_object_key(cps_api_object_list_get(param->filters,ix));
    if (!cps_api_get_handle(*key,handle)) {
        EV_LOG(ERR,DSAPI,0,"NS","Failed to find owner for %s",
                cps_api_key_print(key,buff,sizeof(buff)-1));
        return rc;
    }

    do {
        cps_api_object_t o = cps_api_object_list_get(param->filters,ix);
        if (o==NULL) {
            EV_LOG(ERR,DSAPI,0,"NS","Missing filters... %s",
                                       cps_api_key_print(key,buff,sizeof(buff)-1));
            break;
        }
        if (!cps_api_send_one_object(handle,cps_api_msg_o_GET,o)) {
                   EV_LOG(ERR,DSAPI,0,"NS","Failed to send request %s",
                           cps_api_key_print(key,buff,sizeof(buff)-1));
                   break;
        }

        uint32_t op;
        do {
            if (param->timeout > 0) {
                fd_set rset;
                FD_ZERO(&rset);
                FD_SET(handle,&rset);
                if ((rc=cps_api_timeout_wait(handle,&rset,param->timeout,"CPS-OP-GET"))!=cps_api_ret_code_OK) {
                    break;
                }
            }
            size_t len;
            if (!cps_api_receive_header(handle,op,len)) break;
            if (op!=cps_api_msg_o_GET_RESP) break;
            cps_api_object_guard og (cps_api_receive_object(handle,len));

            if (og.valid() && cps_api_object_list_append(param->list,og.get())) {
                og.release();
            } else break;
        } while (op == cps_api_msg_o_GET_RESP);

        if (op!=cps_api_msg_o_GET_DONE) break; //leave an error code
        rc = cps_api_ret_code_OK;
    } while (0);

    cps_api_disconnect_owner(handle);

    return rc;
}

cps_api_return_code_t cps_api_process_commit_request(cps_api_transaction_params_t *param, size_t ix) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;

    cps_api_object_t obj = cps_api_object_list_get(param->change_list,ix);
    cps_api_object_t pre = cps_api_object_list_get(param->prev,ix);
    if (obj==NULL) return rc;

    cps_api_channel_t handle;
    cps_api_key_t *key = cps_api_object_key(obj);
    if (!cps_api_get_handle(*key,handle)) return rc;

    rc = cps_api_ret_code_ERR;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(handle,&rset);

    do {
        if (!cps_api_send_one_object(handle,cps_api_msg_o_COMMIT_CHANGE,obj)) break;
        if (!cps_api_send_one_object(handle,cps_api_msg_o_COMMIT_PREV,pre)) break;

        uint32_t op;
        size_t len;
        if (param->timeout > 0) {
            if ((rc=cps_api_timeout_wait(handle,&rset,param->timeout,"CPS-OP-TR"))!=cps_api_ret_code_OK) {
                break;
            }
        }
        rc = cps_api_ret_code_ERR;

        if (!cps_api_receive_header(handle,op,len)) break;

        if (op == cps_api_msg_o_COMMIT_OBJECT) {
            cps_api_object_guard og(cps_api_receive_object(handle,len));
            if (!og.valid()) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing cur object..");
                break;
            }
            cps_api_object_clone(obj,og.get());

            if (!cps_api_receive_header(handle,op,len)) break;
            if (op!=cps_api_msg_o_COMMIT_OBJECT) break;

            if (len>0) {
                cps_api_object_guard og(cps_api_receive_object(handle,len));
                if (!og.valid()) {
                    EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing resp object..");
                    break;
                }
                obj = cps_api_object_list_get(param->prev,ix);

                if (obj==NULL) { //if there was no previous object passed by client override
                    if (cps_api_object_list_append(param->prev,og.get())) {
                        og.release();
                        rc = cps_api_ret_code_OK;
                    }
                } else {
                    //take the previous provided by the client if the key is valid
                    //assume that the data is valid too
                    if (cps_api_key_get_len(cps_api_object_key(og.get()))>0) {
                        cps_api_object_clone(obj,og.get());
                    }
                    rc = cps_api_ret_code_OK;
                }
            }
        }
        if (op == cps_api_msg_o_RETURN_CODE) {
            if (!cps_api_receive_data(handle,&rc,sizeof(rc))) break;
        }
    } while (0);

    cps_api_disconnect_owner(handle);

    return rc;
}

cps_api_return_code_t cps_api_process_rollback_request(cps_api_transaction_params_t *param,
        size_t ix) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;

    cps_api_object_t obj = cps_api_object_list_get(param->prev,ix);
    if (obj==NULL) return rc;

    cps_api_channel_t handle;
    if (!cps_api_get_handle(*cps_api_object_key(obj),handle)) return rc;
    cps_api_channel_handle_guard hg(handle);

    rc = cps_api_ret_code_ERR;

    do {
        if (!cps_api_send_one_object(handle,cps_api_msg_o_REVERT,obj)) break;

        uint32_t op;
           size_t len;
           if (!cps_api_receive_header(handle,op,len)) break;

           if (op == cps_api_msg_o_RETURN_CODE) {
               if (!cps_api_receive_data(handle,&rc,sizeof(rc))) break;
           }

    } while (0);

    cps_api_disconnect_owner(handle);

    return rc;
}

extern "C" cps_api_return_code_t cps_api_object_stats(cps_api_key_t *key, cps_api_object_t stats_obj) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    char buff[CPS_API_KEY_STR_MAX];
    cps_api_channel_t handle;
    if (!cps_api_get_handle(*key,handle)) {
        EV_LOG(ERR,DSAPI,0,"NS","Failed to find owner for %s",
                cps_api_key_print(key,buff,sizeof(buff)-1));
        return rc;
    }
    do {
        if (!cps_api_send_header(handle,cps_api_msg_o_STATS,0)) {
            break;
        }
        size_t len;
        uint32_t op=0;
        if (!cps_api_receive_header(handle,op,len)) break;
        if (op!=cps_api_msg_o_STATS) break;

        cps_api_object_guard og (cps_api_receive_object(handle,len));
        if(!og.valid()) return false;

        if (!cps_api_object_clone(stats_obj,og.get())) {
            break;
        }
        rc = cps_api_ret_code_OK;
    } while (0);
    cps_api_disconnect_owner(handle);
    return rc;
}
