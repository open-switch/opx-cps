/* OPENSOURCELICENSE */
/*
 * cps_api_client_ipc.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */

#include "private/cps_api_client_utils.h"
#include "private/cps_ns.h"

#include "cps_api_object.h"
#include "event_log.h"
#include "std_file_utils.h"

#include <unistd.h>

struct cps_msg_hdr_t {
    uint32_t len;
    uint32_t operation;
    uint32_t version;
};

#define SCRATCH_LOG_BUFF (100)

bool cps_api_send(cps_api_channel_t handle, cps_api_msg_operation_t op,
        const struct iovec *iov,
        size_t count) {

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
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to read the header.");
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
    cps_api_object_t obj = cps_api_object_create();
    t_std_error rc = STD_ERR_OK;
    do {
        if (cps_api_object_get_reserve_len(obj) < len) {
            if (!cps_api_object_reserve(obj,len)) break;
        }

        int by = std_read(handle,cps_api_object_array(obj),len,true,&rc);
        if (by!=(int)len) break;

        if (!cps_api_object_received(obj,len)) break;
        return obj;
    } while (0);
    cps_api_object_delete(obj);
    EV_LOG(ERR,DSAPI,0,"CPS IPC","Was not able to read the object - MSG (%X)",rc);
    return NULL;
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

bool cps_api_get_handle(cps_api_key_t &key, cps_api_channel_t &handle) {
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
    return rc;
}


cps_api_return_code_t cps_api_process_get_request(cps_api_get_params_t *param, size_t ix) {
    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    char buff[SCRATCH_LOG_BUFF];
    cps_api_channel_t handle;
    if (!cps_api_get_handle(param->keys[ix],handle)) {
        EV_LOG(ERR,DSAPI,0,"NS","Failed to find owner for %s",
                cps_api_key_print(&param->keys[ix],buff,sizeof(buff)-1));
        return rc;
    }

    do {
        cps_api_object_t o = cps_api_object_list_get(param->filters,ix);
        if (o==NULL) {
            EV_LOG(ERR,DSAPI,0,"NS","Missing filters... %s",
                                       cps_api_key_print(&param->keys[ix],buff,sizeof(buff)-1));
            break;
        }
        if (!cps_api_send_one_object(handle,cps_api_msg_o_GET,o)) {
                   EV_LOG(ERR,DSAPI,0,"NS","Failed to send request %s",
                           cps_api_key_print(&param->keys[ix],buff,sizeof(buff)-1));
                   break;
        }

        uint32_t op;

        do {
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
    if (obj==NULL) return rc;

    cps_api_channel_t handle;
    cps_api_key_t *key = cps_api_object_key(obj);
    if (!cps_api_get_handle(*key,handle)) return rc;

    rc = cps_api_ret_code_ERR;

    do {
        if (!cps_api_send_header(handle,cps_api_msg_o_COMMIT,
                cps_api_object_to_array_len(obj))) break;

        if (!cps_api_send_object(handle,obj)) break;

        uint32_t op;
        size_t len;

        if (!cps_api_receive_header(handle,op,len)) break;

        if (op == cps_api_msg_o_COMMIT_OBJECT) {
            cps_api_object_guard og(cps_api_receive_object(handle,len));
            if (!og.valid()) {
                EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing cur object..");
                break;
            }
            cps_api_object_clone(obj,og.get());
            if (!cps_api_receive_header(handle,op,len)) break;
            if (op == cps_api_msg_o_COMMIT_OBJECT) {
                cps_api_object_guard og(cps_api_receive_object(handle,len));
                if (!og.valid()) {
                    EV_LOG(ERR,DSAPI,0,"CPS IPC","Transaction response missing resp object..");
                    break;
                }
                if (cps_api_object_list_append(param->prev,og.get())) {
                    og.release();
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
