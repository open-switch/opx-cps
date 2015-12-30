/* OPENSOURCELICENSE */
/*
 * cps_api_operation_service.cpp
 * (c) Copyright 2014 Dell Inc. All Rights Reserved.
 */


#include "private/cps_ns.h"
#include "private/cps_api_client_utils.h"

#include "cps_api_operation_stats.h"

#include "cps_api_operation.h"
#include "cps_api_object.h"
#include "std_socket_service.h"
#include "std_rw_lock.h"
#include "std_mutex_lock.h"
#include "event_log.h"
#include "std_file_utils.h"
#include "std_time_tools.h"
#include "std_assert.h"

#include <unistd.h>
#include <unordered_map>
#include <vector>

using reg_functions_t = std::vector<cps_api_registration_functions_t>;

struct cps_api_operation_data_t {
    std_socket_server_handle_t handle;
    std_socket_server_t service_data;
    std_rw_lock_t db_lock;
    std_mutex_type_t mutex;
    reg_functions_t db_functions;
    cps_api_channel_t ns_handle;
    std::unordered_map<uint64_t,uint64_t> stats;

    void insert_functions(cps_api_registration_functions_t*fun);

    uint64_t get_stat(cps_api_obj_stats_type_t stat);
    void set_stat(cps_api_obj_stats_type_t stat,uint64_t val);
    void inc_stat(cps_api_obj_stats_type_t stat,int64_t how_much=1);
    void make_ave(cps_api_obj_stats_type_t stat,cps_api_obj_stats_type_t count,uint64_t val);
};

uint64_t cps_api_operation_data_t::get_stat(cps_api_obj_stats_type_t stat) {
    std_mutex_simple_lock_guard lg(&mutex);
    return stats[stat];
}

void cps_api_operation_data_t::set_stat(cps_api_obj_stats_type_t stat,uint64_t val) {
    std_mutex_simple_lock_guard lg(&mutex);
    stats[stat] = val;
}

void cps_api_operation_data_t::inc_stat(cps_api_obj_stats_type_t stat,int64_t how_much) {
    std_mutex_simple_lock_guard lg(&mutex);
    set_stat(stat,get_stat(stat) + how_much);
}

void cps_api_operation_data_t::make_ave(cps_api_obj_stats_type_t stat,cps_api_obj_stats_type_t count,uint64_t val) {
    std_mutex_simple_lock_guard lg(&mutex);
    inc_stat(count);

    uint64_t cnt = get_stat(count);
    uint64_t tot = get_stat(stat) + val;

    set_stat(stat,(uint64_t)(tot/((double)cnt)));
}

class Function_Timer {
    cps_api_operation_data_t *_p;
    cps_api_obj_stats_type_t _min;
    cps_api_obj_stats_type_t _max;
    cps_api_obj_stats_type_t _ave;
    cps_api_obj_stats_type_t _count;
    bool started=false;
    uint64_t tm=0;
public:
    Function_Timer(cps_api_operation_data_t *p,
            cps_api_obj_stats_type_t min, cps_api_obj_stats_type_t max,
            cps_api_obj_stats_type_t ave, cps_api_obj_stats_type_t count) {
        _p = p;
        _min = min;
        _max = max;
        _ave = ave;
        _count = count;
    }
    void start() {
        tm = std_get_uptime(NULL);
        started = true;
    }
    ~Function_Timer() {
        if (started) {
            uint64_t diff = std_get_uptime(NULL) - tm;
            _p->make_ave(_ave,_count,diff);
            uint64_t min = _p->get_stat(_min);
            if (min==0 || diff < min) {
                _p->set_stat(_min,diff);
            }
            if (diff > _p->get_stat(_max)) {
                _p->set_stat(_max,diff);
            }
        } else {
            _p->inc_stat(_count);
        }
    }
};

static bool  cps_api_handle_get(cps_api_operation_data_t *op, int fd,size_t len) {
    Function_Timer tm(op,cps_api_obj_stat_GET_MIN_TIME,cps_api_obj_stat_GET_MAX_TIME,
            cps_api_obj_stat_GET_AVE_TIME,cps_api_obj_stat_GET_COUNT);

    cps_api_get_params_t param;
    if (cps_api_get_request_init(&param)!=cps_api_ret_code_OK) {
        op->inc_stat(cps_api_obj_stat_GET_INVALID);
        return false;
    }
    cps_api_get_request_guard grg(&param);

    cps_api_object_t filter = cps_api_receive_object(fd,len);
    if (filter==NULL) {
        EV_LOG(ERR,DSAPI,0,"CPS IPC","Get request missing filter object..");
        op->inc_stat(cps_api_obj_stat_GET_INVALID);
        return false;
    }

    if (!cps_api_object_list_append(param.filters,filter)) {
        cps_api_object_delete(filter);
        op->inc_stat(cps_api_obj_stat_GET_INVALID);
        return false;
    }

    std_rw_lock_read_guard g(&op->db_lock);

    param.key_count =1;
    param.keys = cps_api_object_key(filter);

    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();

    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_read_function!=NULL) &&
                (cps_api_key_matches(param.keys,&p->key,false)==0)) {

            tm.start();
            rc = p->_read_function(p->context,&param,0);

            break;
        }
    }

    if (rc!=cps_api_ret_code_OK) {
        op->inc_stat(cps_api_obj_stat_GET_FAILED);
        return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
    } else {
        //send all objects in the list
        size_t ix = 0;
        size_t mx = cps_api_object_list_size(param.list);
        for ( ; (ix < mx) ; ++ix ) {
            cps_api_object_t obj = cps_api_object_list_get(param.list,ix);
            if (obj==NULL) continue;
            if (!cps_api_send_one_object(fd,cps_api_msg_o_GET_RESP,obj)) return false;
        }
        if (!cps_api_send_header(fd,cps_api_msg_o_GET_DONE,0)) return false;
    }

    return true;
}


static bool cps_api_handle_commit(cps_api_operation_data_t *op, int fd, size_t len) {

    Function_Timer tm(op,cps_api_obj_stat_GET_MIN_TIME,cps_api_obj_stat_GET_MAX_TIME,
                cps_api_obj_stat_GET_AVE_TIME,cps_api_obj_stat_GET_COUNT);

    std_rw_lock_read_guard g(&op->db_lock);
    cps_api_return_code_t rc =cps_api_ret_code_OK;

    //receive changed object
    cps_api_object_guard o(cps_api_receive_object(fd,len));
    if (!o.valid()) {
        op->inc_stat(cps_api_obj_stat_SET_INVALID);
        return false;
    }

    //initialize transaction
    cps_api_transaction_params_t param;
    if (cps_api_transaction_init(&param)!=cps_api_ret_code_OK) {
        op->inc_stat(cps_api_obj_stat_SET_INVALID);
        return false;
    }

    cps_api_transaction_guard tr(&param);

    //add request to change list
    if (!cps_api_object_list_append(param.change_list,o.get())) return false;
    cps_api_object_t l = o.release();


    //check the previous data sent by client
    uint32_t act;
    if (!cps_api_receive_header(fd,act,len)) {
        op->inc_stat(cps_api_obj_stat_SET_INVALID);
        return false;
    }
    if(act!=cps_api_msg_o_COMMIT_PREV) {
        op->inc_stat(cps_api_obj_stat_SET_INVALID);
        return false;
    }

    //read and add to list the previous object if passed
    if (len > 0) {
        cps_api_object_guard prev(cps_api_receive_object(fd,len));
        if (!prev.valid()) {
            op->inc_stat(cps_api_obj_stat_SET_INVALID);
            return false;
        }
        if (!cps_api_object_list_append(param.prev,prev.get())) return false;
        prev.release();
    }

    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();
    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_write_function!=NULL) &&
                (cps_api_key_matches(cps_api_object_key(l),&p->key,false)==0)) {
            tm.start();
            rc = p->_write_function(p->context,&param,0);
            break;
        }
    }

    if (rc!=cps_api_ret_code_OK) {
        op->inc_stat(cps_api_obj_stat_SET_FAILED);
        return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
    } else {
        cps_api_object_t cur = cps_api_object_list_get(param.change_list,0);
        if (cps_api_object_list_size(param.prev)==0) {
            cps_api_object_list_create_obj_and_append(param.prev);
        }
        cps_api_object_t prev = cps_api_object_list_get(param.prev,0);

        if (cur==NULL || prev==NULL) {
            EV_LOG(ERR,DSAPI,0,"COMMIT-REQ","response to request was invalid - cur=%d,prev=%d.",
                    (cur!=NULL),(prev!=NULL));
            op->inc_stat(cps_api_obj_stat_SET_INVALID);
            return false;
        }

        if (!cps_api_send_one_object(fd,cps_api_msg_o_COMMIT_OBJECT,cur) ||
                !cps_api_send_one_object(fd,cps_api_msg_o_COMMIT_OBJECT,prev)) {
            EV_LOG(ERR,DSAPI,0,"COMMIT-REQ","Failed to send response to commit... ");
            op->inc_stat(cps_api_obj_stat_SET_INVALID);
            return false;
        }
    }

    return true;
}

static bool cps_api_handle_revert(cps_api_operation_data_t *op, int fd, size_t len) {
    std_rw_lock_read_guard g(&op->db_lock);

    cps_api_return_code_t rc =cps_api_ret_code_OK;

    cps_api_object_guard o(cps_api_receive_object(fd,len));
    if (!o.valid()) return false;

    cps_api_transaction_params_t param;
    if (cps_api_transaction_init(&param)!=cps_api_ret_code_OK) return false;
    cps_api_transaction_guard tr(&param);

    if (!cps_api_object_list_append(param.prev,o.get())) return false;

    cps_api_object_t l = o.release();

    size_t func_ix = 0;
    size_t func_mx = op->db_functions.size();
    for ( ; func_ix < func_mx ; ++func_ix ) {
        cps_api_registration_functions_t *p = &(op->db_functions[func_ix]);
        if ((p->_rollback_function!=NULL) &&
                (cps_api_key_matches(cps_api_object_key(l),&p->key,false)==0)) {
            rc = p->_rollback_function(p->context,&param,0);
            break;
        }
    }

    return cps_api_send_return_code(fd,cps_api_msg_o_RETURN_CODE,rc);
}

static bool cps_api_handle_stats(cps_api_operation_data_t *op, int fd, size_t len) {
    std_rw_lock_read_guard g(&op->db_lock);

    cps_api_object_guard og(cps_api_object_create());
    if (!og.valid()) return false;

    cps_api_key_init(cps_api_object_key(og.get()),cps_api_qualifier_OBSERVED,
            cps_api_obj_stat_e_OPERATIONS,0,0);

    cps_api_obj_stats_type_t cur = cps_api_obj_stat_BEGIN;
    for ( ; cur < cps_api_obj_stat_MAX ; cur= (cps_api_obj_stats_type_t)(cur+1) ) {
        cps_api_object_attr_add_u64(og.get(),cur,op->get_stat(cur));
    }

    if (!cps_api_send_one_object(fd,cps_api_msg_o_STATS,og.get())) return false;

    return true;
}

static bool  _some_data_( void *context, int fd ) {
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)context;

    uint32_t op;
    size_t len;

    if(!cps_api_receive_header(fd,op,len)) return false;

    if (op==cps_api_msg_o_GET) return cps_api_handle_get(p,fd,len);
    if (op==cps_api_msg_o_COMMIT_CHANGE) return cps_api_handle_commit(p,fd,len);
    if (op==cps_api_msg_o_REVERT) return cps_api_handle_revert(p,fd,len);
    if (op==cps_api_msg_o_STATS) return cps_api_handle_stats(p,fd,len);
    return true;
}

static bool register_one_key(cps_api_operation_data_t *p, cps_api_key_t &key) {
    cps_api_object_owner_reg_t r;
    r.addr = p->service_data.address;
    memcpy(&r.key,&key,sizeof(r.key));
    if (!cps_api_ns_register(p->ns_handle,r)) {
        close(p->ns_handle);
        return false;
    }
    return true;
}
static bool reconnect_with_ns(cps_api_operation_data_t *data) {
    if (!cps_api_ns_create_handle(&data->ns_handle)) {
        data->ns_handle = STD_INVALID_FD;
        return false;
    }
    if (std_socket_service_client_add(data->handle,data->ns_handle)!=STD_ERR_OK) {
        close(data->ns_handle);
        return false;
    }
    size_t ix = 0;
    size_t mx = data->db_functions.size();
    for ( ; ix < mx ; ++ix ) {
        if(!register_one_key(data,data->db_functions[ix].key)) {
            close(data->ns_handle); data->ns_handle=STD_INVALID_FD;
            return false;
        }
    }
    data->inc_stat(cps_api_obj_stat_NS_CONNECTS);
    return true;
}

void cps_api_operation_data_t::insert_functions(cps_api_registration_functions_t*fun) {
    auto it = db_functions.begin();
    auto end = db_functions.end();

    size_t key_size = cps_api_key_get_len(&fun->key);

    for ( ; it != end ; ++it ) {
        if (cps_api_key_matches(&fun->key,&it->key,false)==0) {
            size_t target_len = cps_api_key_get_len(&it->key);
            if (key_size < target_len) continue;
            char buffA[CPS_API_KEY_STR_MAX];
            char buffB[CPS_API_KEY_STR_MAX];
            EV_LOG(INFO,DSAPI,0,"NS","Inserting %s after %s",
                    cps_api_key_print(&it->key,buffA,sizeof(buffA)-1),
                    cps_api_key_print(&fun->key,buffB,sizeof(buffB)-1)
                    );
            db_functions.insert(it,*fun);
            return ;
        }
    }
    db_functions.push_back(*fun);
}

cps_api_return_code_t cps_api_register(cps_api_registration_functions_t * reg) {
    STD_ASSERT(reg->handle!=NULL);
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)reg->handle;
    std_rw_lock_write_guard g(&p->db_lock);

    if (p->ns_handle==STD_INVALID_FD) {
        reconnect_with_ns(p);
    }
    p->insert_functions(reg);

    if (p->ns_handle!=STD_INVALID_FD) {
        if (!register_one_key(p,reg->key)) {
            close(p->ns_handle);
            p->ns_handle = STD_INVALID_FD;
        }
    }
    return cps_api_ret_code_OK;
}


static void _timedout(void * context) {
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)context;
    STD_ASSRT(p!=nullptr);
    if (p->ns_handle==STD_INVALID_FD) {
        std_rw_lock_write_guard g(&p->db_lock);
        reconnect_with_ns(p);
    }
}


static bool _del_client(void * context, int fd) {
    cps_api_operation_data_t *p = (cps_api_operation_data_t *)context;
    STD_ASSRT(p!=nullptr);
    if (p->ns_handle==fd) {
        std_rw_lock_write_guard g(&p->db_lock);
        p->inc_stat(cps_api_obj_stat_NS_DISCONNECTS);
        p->ns_handle = STD_INVALID_FD;
    }
    return true;
}

cps_api_return_code_t cps_api_operation_subsystem_init(
        cps_api_operation_handle_t *handle, size_t number_of_threads) {

    cps_api_operation_data_t *p = new cps_api_operation_data_t;
    if (p==NULL) return cps_api_ret_code_ERR;

    p->handle = NULL;
    memset(&p->service_data,0,sizeof(p->service_data));
    std_mutex_lock_init_recursive(&p->mutex);

    std_rw_lock_create_default(&p->db_lock);

    for ( size_t ix = cps_api_obj_stat_BEGIN, mx =cps_api_obj_stat_MAX ; ix < mx ; ++ix ) {
        p->stats[ix] = 0;
    }
    p->service_data.name = "CPS_API_instance";
    cps_api_create_process_address(&p->service_data.address);
    p->service_data.thread_pool_size = number_of_threads;
    p->service_data.some_data = _some_data_;
    p->service_data.timeout = _timedout;
    p->service_data.del_client = _del_client;

    p->service_data.context = p;

    if (std_socket_service_init(&p->handle,&p->service_data)!=STD_ERR_OK) {
        delete p;
        return cps_api_ret_code_ERR;
    }

    if (std_socket_service_run(p->handle)!=STD_ERR_OK) {
        std_socket_service_destroy(p->handle);
        delete p;
        return cps_api_ret_code_ERR;
    }

    {
        std_rw_lock_write_guard g(&p->db_lock);
        reconnect_with_ns(p);
    }
    *handle = p;

    return cps_api_ret_code_OK;
}

