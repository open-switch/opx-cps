/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */

/*
 * cps_api_client_ipc.cpp
 */

#include "private/cps_api_client_utils.h"
#include "private/cps_api_key_cache.h"
#include "private/cps_ns.h"

#include "cps_api_object.h"
#include "cps_api_object_tools.h"
#include "cps_class_map.h"

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
        rc = cps_api_ret_code_SERVICE_CONNECT_FAIL;
    }
    return (cps_api_return_code_t) rc;
}

static bool cache_connect(cps_api_key_t &key, cps_api_channel_t &handle) {
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
        EV_LOG(TRACE,DSAPI,0,"CPS IPC","Was not able to send the full message header.");
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
    if (by == 0) {
        return false;
    }
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
        EV_LOG(TRACE,DSAPI,0,"CPS IPC","Was not able to read the data. (%X)",msg_rc);
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
        EV_LOG(ERR,DSAPI,0,op,"CPS Operation failed - application close");
        return cps_api_ret_code_ERR;
    }
    if (rc==0) {
        EV_LOG(ERR,DSAPI,0,op,"CPS Operation failed - application time out");
        return cps_api_ret_code_TIMEOUT;
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
        return cps_api_ret_code_NO_SERVICE;
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
                    EV_LOG(ERR,DSAPI,0,"CPS-OP-GET","Send request %s timed out",
                           cps_api_key_print(key,buff,sizeof(buff)-1));
                    break;
                }
            }
            size_t len;
            if (!cps_api_receive_header(handle,op,len)) break;
            if (op == cps_api_msg_o_RETURN_CODE) {
                cps_api_receive_data(handle, &rc, sizeof(rc));
                break;
            }
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
    if (obj==NULL) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","No Commit Object");
        return rc;
    }

    cps_api_channel_t handle;
    cps_api_key_t *key = cps_api_object_key(obj);

    char buff[CPS_API_KEY_STR_MAX];
    EV_LOG(TRACE,DSAPI,0, "CPS IPC", "Object for commit request: %s ",
                                    cps_api_key_name_print(key, buff, sizeof(buff)));

    if (!cps_api_get_handle(*key,handle)) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","No Service");
        return cps_api_ret_code_NO_SERVICE;
    }

    rc = cps_api_ret_code_ERR;

    fd_set rset;
    FD_ZERO(&rset);
    FD_SET(handle,&rset);

    do {
        if (!cps_api_send_one_object(handle,cps_api_msg_o_COMMIT_CHANGE,obj)) {
            EV_LOG(ERR,DSAPI,0,"CPS IPC","Could not send COMMIT CHANGE ");
            break;
        }

        if (!cps_api_send_one_object(handle,cps_api_msg_o_COMMIT_PREV,pre)) {
            EV_LOG(ERR,DSAPI,0,"CPS IPC","Could not send COMMIT PREV ");
            break;
        }

        uint32_t op;
        size_t len;
        if (param->timeout > 0) {
            if ((rc=cps_api_timeout_wait(handle,&rset,param->timeout,"CPS-OP-TR"))!=cps_api_ret_code_OK) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction Response timed out ");
                break;
            }
        }
        rc = cps_api_ret_code_ERR;

        if (!cps_api_receive_header(handle,op,len)) {
            EV_LOG(ERR,DSAPI,0,"CPS IPC","Failed to read the receive header ");
            break;
        }

        if (op == cps_api_msg_o_COMMIT_OBJECT) {
            cps_api_object_guard og(cps_api_receive_object(handle,len));
            if (!og.valid()) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing cur object..");
                break;
            }
            cps_api_object_swap(obj,og.get());

            if (!cps_api_receive_header(handle,op,len)) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Failed to read the receive header for prev object ");
                break;
            }

            if (op!=cps_api_msg_o_COMMIT_OBJECT) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response header incorrect for prev object" );
                break;
            }

            if (len>0) {
                cps_api_object_guard og(cps_api_receive_object(handle,len));
                if (!og.valid()) {
                    EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing resp object..");
                    break;
                }
                if (pre==nullptr) {
                    if (cps_api_object_list_set(param->prev,ix,og.get(),true)) {
                        og.release();
                        rc = cps_api_ret_code_OK;
                    }
                } else {
                    //take the previous provided by the client if the key is valid
                    //assume that the data is valid too
                    if (cps_api_key_get_len(cps_api_object_key(og.get()))>0) {
                        cps_api_object_swap(pre,og.get());
                    }
                    rc = cps_api_ret_code_OK;
                }
            }
            const t_std_error *_rc = cps_api_object_return_code(obj);
            if (_rc!=nullptr) rc = *_rc;

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
    if (obj==NULL) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","No Revert Object");
        return rc;
    }

    cps_api_channel_t handle;
    cps_api_key_t *key = cps_api_object_key(obj);

    char buff[CPS_API_KEY_STR_MAX];
    EV_LOG(TRACE,DSAPI,0, "CPS IPC", "Object for rollback request: %s ",
                                    cps_api_key_name_print(key, buff, sizeof(buff)));

    if (!cps_api_get_handle(*key,handle)) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","No Service");
        return cps_api_ret_code_NO_SERVICE;
    }

    cps_api_channel_handle_guard hg(handle);

    rc = cps_api_ret_code_ERR;

    do {
        if (!cps_api_send_one_object(handle,cps_api_msg_o_REVERT,obj)) {
            EV_LOG(ERR,DSAPI,0,"CPS IPC","Could not send REVERT header");
            break;
        }

        uint32_t op;
           size_t len;
           if (!cps_api_receive_header(handle,op,len)) {
               EV_LOG(ERR,DSAPI,0,"CPS IPC","Failed to read the receive header for revert object");
               break;
           }

           if (op == cps_api_msg_o_RETURN_CODE) {
               if (!cps_api_receive_data(handle,&rc,sizeof(rc))) break;
           }

    } while (0);

    cps_api_disconnect_owner(handle);

    return rc;
}

extern "C" cps_api_return_code_t cps_api_object_stats(cps_api_key_t *key, cps_api_object_t stats_obj) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;

    cps_api_channel_t handle = -1;

    if (key==nullptr || cps_api_key_get_len(key)==0) {
        std_socket_address_t addr;
        cps_api_ns_get_address(&addr);

        int sock;
        t_std_error _rc = std_sock_connect(&addr,&sock);
        if (_rc!=STD_ERR_OK) {
            return rc;
        }
        handle = sock;
    } else {
        if (!cps_api_get_handle(*key,handle)) {
            char buff[CPS_API_KEY_STR_MAX];
            EV_LOG(ERR,DSAPI,0,"NS","Failed to find owner for %s",
                    cps_api_key_print(key,buff,sizeof(buff)-1));
            return cps_api_ret_code_NO_SERVICE;
        }
    }
    if (handle==-1) {
        EV_LOG(ERR,DSAPI,0,"NS-STATS","No connection to the NS for stats.");
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
