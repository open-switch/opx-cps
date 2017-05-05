#include "cps_api_db_interface.h"

#include "cps_api_object_tools.h"
#include "cps_api_operation_tools.h"

#include "cps_api_node.h"
#include "cps_api_node_private.h"
#include "cps_api_db_connection.h"
#include "cps_api_db.h"
#include "dell-cps.h"

#include <string>
#include <cstring>
#include <unordered_map>


constexpr static const char * __get_default_node_name() {
    return "localhost";
}

constexpr static const char *__get_dirty_field() {
    return "dirty";
}

extern "C" {
cps_api_return_code_t cps_api_db_config_write(cps_api_object_t obj) {
    cps_api_return_code_t rc = cps_api_ret_code_OK;
    cps_api_key_t *key = cps_api_object_key(obj);

    cps_api_qualifier_t _qual = cps_api_key_get_qual(key);

    CPS_CONFIG_TYPE_t type = cps_api_object_get_config_type(obj);
    cps_api_operation_types_t op = cps_api_object_type_operation(key);

    if ((type == CPS_CONFIG_TYPE_RUNNING_CONFIG) ||
            (type == CPS_CONFIG_TYPE_STARTUP_AND_RUNNING) ||
            (type == CPS_CONFIG_TYPE_RUNNING_CONFIG_ONLY)) {
        cps_api_key_set_qualifier(key, cps_api_qualifier_RUNNING_CONFIG);
        rc = cps_api_db_commit_one(op, obj, NULL, true);
        if (rc != cps_api_ret_code_OK) return rc;
    }
    if ((type == CPS_CONFIG_TYPE_STARTUP_CONFIG) ||
            (type == CPS_CONFIG_TYPE_STARTUP_AND_RUNNING)) {
        cps_api_key_set_qualifier(key, cps_api_qualifier_STARTUP_CONFIG);
        rc = cps_api_db_commit_one(op, obj, NULL, true);
        if (rc != cps_api_ret_code_OK) return rc;
    }
    cps_api_key_set_qualifier(key,_qual);
    return rc;
}
}

static bool _mark_dirty_dbkey(cps_api_object_t obj, cps_db::connection &conn){

    std::vector<char> key;
    bool _is_wildcard = false;
    if (!cps_db::dbkey_instance_or_wildcard(key, obj, _is_wildcard))
        return false;

    const char *field = __get_dirty_field();
    const char *data = "false";
    if(!cps_db::for_each_store_field(conn, key, field, std::strlen(field), data, std::strlen(data)))
        return false;

    return true;
}

static void _process_cb_response(cps_db::connection &conn, cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_response_t *res, std::string key ) {
    if(params->opcode == cps_api_oper_DELETE) {
        const char *_field = __get_dirty_field();
        if(res->change == cps_api_make_change) {
            cps_db::delete_object(conn, params->object_dest);
            if (cps_db::dbkey_field_delete_request(conn, key.c_str(), key.size(),
                                              _field, std::strlen(_field))
                && cps_db::dbkey_field_delete_response(conn)) {
                if(res->change_notify == cps_api_raise_event)
                    cps_db::publish(conn, params->object_dest);
            }
        }
    }
    else {
        if(res->change == cps_api_make_change) {
            cps_db::store_object(conn, params->object_src);
            if(res->change_notify == cps_api_raise_event) {
                cps_db::publish(conn, params->object_src);
            }
        }
    }
}


static void _get_addr_info(cps_api_object_t obj, char **node_name, char **addr, cps_api_nodes &n){
    if(obj) {
        *node_name = (char *)cps_api_key_get_node(obj);
        if(*node_name) {
            *addr = (char *)(n.addr(*node_name));
        } else {
            *node_name = (char *)__get_default_node_name();
            *addr = (char *)DEFAULT_REDIS_ADDR;
        }
    }
    else {
        *node_name = (char *)__get_default_node_name();
        *addr = (char *)DEFAULT_REDIS_ADDR;
    }
}

using cps_dbkey_obj_map = std::unordered_map<std::string, cps_api_object_t>;

static bool _get_obj_dbkey_map(cps_api_object_list_t src_objs, cps_dbkey_obj_map &src_obj_map) {
    size_t mx = cps_api_object_list_size(src_objs);

    for(size_t ix = 0; ix < mx; ++ix){
        cps_api_object_t temp_obj = cps_api_object_list_get(src_objs, ix);
        STD_ASSERT(temp_obj!=nullptr);

        std::vector<char> src_inst_key;
        bool _is_wildcard =false;
        if (!cps_db::dbkey_instance_or_wildcard(src_inst_key, temp_obj, _is_wildcard)) return false;
        std::string k(&src_inst_key[0],src_inst_key.size());
        src_obj_map[k] = temp_obj;
    }
    return true;
}

static bool _handle_create_set_case(const char *_dest_addr, const char *key, size_t key_len, cps_api_db_sync_cb_param_t &params, 
                                    bool &cb, bool src=false, bool dst=false)
{

    const char *field = __get_dirty_field();
    cps_db::connection_request _dest_conn_meta(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn_meta.valid() || !cps_db::ping(_dest_conn_meta.get())) return cps_api_ret_code_ERR;

    if (src  && dst) {
        //Set case
        if( (!cps_db::dbkey_field_delete_request(_dest_conn_meta.get(), key, key_len, field, std::strlen(field) )) ||
            (!cps_db::dbkey_field_delete_response(_dest_conn_meta.get()))) return false;
        params.opcode = cps_api_oper_SET;
        if(!cps_api_obj_tool_matches_filter(params.object_src,params.object_dest,true)) cb = true;

        return true;

     } else if (src) {
        //Create case
        params.opcode = cps_api_oper_CREATE;
        cb = true;

        return true;
     } 

     return false;
}

static cps_api_return_code_t _handle_delete_case(const char *addr, cps_api_object_t src, cps_api_sync_callback_t cb, void *context,
                                          cps_api_db_sync_cb_param_t &params, cps_api_db_sync_cb_response_t &res ) {

    cps_db::connection_request _dest_conn(cps_db::ProcessDBCache(),addr);
    if (!_dest_conn.valid() || !cps_db::ping(_dest_conn.get())) return cps_api_ret_code_ERR;

    cps_db::connection_request _dest_conn_meta(cps_db::ProcessDBCache(),addr);
    if (!_dest_conn_meta.valid() || !cps_db::ping(_dest_conn_meta.get())) return cps_api_ret_code_ERR;

    cps_db::connection_request _dest_conn_cleanup(cps_db::ProcessDBCache(),addr);
    if (!_dest_conn_cleanup.valid() || !cps_db::ping(_dest_conn_cleanup.get())) return cps_api_ret_code_ERR;

    std::vector<char> key;
    bool _is_wildcard = false;
    if (!cps_db::dbkey_instance_or_wildcard(key, src, _is_wildcard)) {
        return cps_api_ret_code_ERR;
    }

    size_t count = 0;
    std::vector<std::string> _key_cache;
    bool ret = true;
    bool walked = false;

    const char *field = __get_dirty_field();

    auto _process_get_response = [&]() {
       for(size_t ix=0; ix < count; ++ix) {
            std::string val = cps_db::dbkey_field_get_response_string(_dest_conn_meta.get());
            if(val.compare("")) {
                //Delete case
                params.opcode = cps_api_oper_DELETE;
                std::vector<char> _k(_key_cache[ix].begin(), _key_cache[ix].end());
                cps_db::get_object(_dest_conn_cleanup.get(), _k, params.object_dest);
                if((cb(context, &params, &res)))
                    _process_cb_response(_dest_conn_cleanup.get(), &params, &res,  _key_cache[ix] );
            }
       }
       count = 0;
       _key_cache.clear();
    };

    walked = cps_db::walk_keys(_dest_conn.get(), &key[0],key.size(),[&](const void *dest_inst_key, size_t len) {
    if(!ret) return;
    if(!cps_db::dbkey_field_get_request(_dest_conn_meta.get(), (char *)dest_inst_key, len, field, std::strlen(field))) {
        ret = false;
    } else ++count;
    _key_cache.push_back(std::string((char*)dest_inst_key,len));
    if(count < cps_db::IN_THE_PIPE()) return;
    _process_get_response();
    });

    if (walked == false || !ret) return cps_api_ret_code_ERR;
    _process_get_response();

    return cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_reconcile(void *context, cps_api_object_list_t src_objs,  cps_api_object_t dest_obj, cps_api_sync_callback_t cb, cps_api_sync_error_callback_t err_cb)
{
    cps_api_db_sync_cb_param_t params = {};
    cps_api_db_sync_cb_response_t res = {cps_api_make_change, cps_api_raise_event};
    cps_api_db_sync_cb_error_t er;

    cps_dbkey_obj_map src_obj_map;
    _get_obj_dbkey_map(src_objs, src_obj_map);

    const char *_src_addr = nullptr;
    const char *_dest_addr = nullptr;

    cps_api_nodes n;
    n.load();

    _get_addr_info(cps_api_object_list_get(src_objs, 0), (char **)(&params.src_node) , (char **)(&_src_addr), n);
    _get_addr_info(dest_obj, (char **)(&params.dest_node) , (char **)(&_dest_addr), n);

    if (!_dest_addr || !_src_addr) {
        er.err_code = cps_api_db_invalid_address;
        err_cb(context, &params, &er);
        return cps_api_ret_code_ERR;
    }


    cps_db::connection_request _dest_conn(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn.valid() || !cps_db::ping(_dest_conn.get())) return cps_api_ret_code_ERR;

    cps_db::connection_request _dest_conn_meta(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn_meta.valid() || !cps_db::ping(_dest_conn_meta.get())) return cps_api_ret_code_ERR;

    if(! _mark_dirty_dbkey(dest_obj, _dest_conn_meta.get())) return cps_api_ret_code_ERR;


    params.object_src = cps_api_object_create();
    params.object_dest = cps_api_object_create();

    bool is_cb = false;
    size_t count = 0;
    std::vector<std::string> _key_cache;

    auto _drain_queue = [&]() {
        for(size_t ix = 0; ix < count; ++ix) {
            bool _rc_a = cps_db::get_object_response(_dest_conn.get(), params.object_dest);
            std::unordered_map< std::string, cps_api_object_t>::const_iterator match = src_obj_map.find(_key_cache[ix]);
            if (match != src_obj_map.end()) {
                cps_api_object_clone(params.object_src, match->second);
                _handle_create_set_case(_dest_addr, _key_cache[ix].c_str(), _key_cache[ix].size(), params, is_cb, true, _rc_a);
                if(is_cb) {
                    if((cb(context, &params, &res)))
                        _process_cb_response(_dest_conn_meta.get(), &params, &res,  _key_cache[ix] );
                }
            }
            is_cb = false;
        }
        count = 0;
        _key_cache.clear();
    };

    size_t mx = cps_api_object_list_size(src_objs);
    for(size_t ix = 0; ix < mx; ++ix){
        cps_api_object_t temp_obj = cps_api_object_list_get(src_objs, ix);
        STD_ASSERT(temp_obj!=nullptr);

        std::vector<char> inst_key;
        bool _is_wildcard =false;
        if (!cps_db::dbkey_instance_or_wildcard(inst_key,temp_obj,_is_wildcard)) return cps_api_ret_code_ERR;

        bool _rc_a = cps_db::get_object_request(_dest_conn.get(), &inst_key[0], inst_key.size()) ;
        if (!_rc_a) continue;
        else ++count;
        std::string kk(&inst_key[0],inst_key.size());
        _key_cache.push_back(kk);
        if (count < cps_db::IN_THE_PIPE()) continue;
        _drain_queue();
    }
    _drain_queue();

    cps_api_return_code_t result = _handle_delete_case(_dest_addr, dest_obj, cb, context, params, res);
    cps_api_object_delete(params.object_src);
    cps_api_object_delete(params.object_dest);

    return result;
}

cps_api_return_code_t cps_api_sync(void *context, cps_api_object_t dest, cps_api_object_t src,  cps_api_sync_callback_t cb, cps_api_sync_error_callback_t err_cb)
{
    cps_api_db_sync_cb_param_t params = {};
    cps_api_db_sync_cb_response_t res = {cps_api_make_change, cps_api_raise_event};
    cps_api_db_sync_cb_error_t er;

    const char *_src_addr;
    const char *_dest_addr;

    cps_api_nodes n;
    n.load();

    _get_addr_info(src, (char **)(&params.src_node) , (char **)(&_src_addr), n);
    _get_addr_info(dest, (char **)(&params.dest_node) , (char **)(&_dest_addr), n);

    if (!_dest_addr || !_src_addr) {
        er.err_code = cps_api_db_invalid_address;
        err_cb(context, &params, &er);
        return cps_api_ret_code_ERR;
    }

    cps_db::connection_request _dest_conn(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn.valid() || !cps_db::ping(_dest_conn.get())) return cps_api_ret_code_ERR;

    cps_db::connection_request _dest_conn_meta(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn_meta.valid() || !cps_db::ping(_dest_conn_meta.get())) return cps_api_ret_code_ERR;

    if(! _mark_dirty_dbkey(src, _dest_conn_meta.get())) return cps_api_ret_code_ERR;

    params.object_src = cps_api_object_create();
    params.object_dest = cps_api_object_create();

    bool is_cb = false;
    size_t count = 0;
    bool ret = true;
    std::vector<std::string> _key_cache;
    bool walked = false;

    std::vector<char> key;
    bool _is_wildcard = false;
    if (!cps_db::dbkey_instance_or_wildcard(key, src, _is_wildcard)) {
        return cps_api_ret_code_ERR;
    }

    {
        cps_db::connection_request _source_get(cps_db::ProcessDBCache(),_src_addr);
        if (!_source_get.valid() || !cps_db::ping(_source_get.get())) {
            return cps_api_ret_code_ERR;
        }
        auto _drain_queue = [&]() {
            for(size_t ix = 0; ix < count; ++ix) {
                bool _rc_a = cps_db::get_object_response(_dest_conn.get(), params.object_dest);
                bool _rc_b = cps_db::get_object_response(_source_get.get(), params.object_src);

                bool result = _handle_create_set_case(_dest_addr, _key_cache[ix].c_str(), _key_cache[ix].size(), params, is_cb, _rc_b, _rc_a);
                if(!result) continue;

                if(is_cb) {
                    if((cb(context, &params, &res)))
                        _process_cb_response(_dest_conn_meta.get(), &params, &res, _key_cache[ix] );
                }
                is_cb = false;
            }
            count = 0;
            _key_cache.clear();
        };
        cps_db::connection_request _source_walk(cps_db::ProcessDBCache(),_src_addr);
        if (!_source_walk.valid() || !cps_db::ping(_source_walk.get())) {
            er.err_code = cps_api_db_no_connection;
            err_cb(context, &params, &er);
            return cps_api_ret_code_ERR;
        }

        walked = cps_db::walk_keys(_source_walk.get(),&key[0],key.size(),[&](const void *remote_inst_key, size_t len) {
            if(!ret) return;
            bool _rc_a = cps_db::get_object_request(_dest_conn.get(), (char *)remote_inst_key, len) ;
            bool _rc_b = cps_db::get_object_request(_source_get.get(), (char *)remote_inst_key, len);
            if (!_rc_a || !_rc_b) {
                ret = false;
            } else ++count;
            _key_cache.push_back(std::string((char*)remote_inst_key,len));
            if (count < cps_db::IN_THE_PIPE()) return;
            _drain_queue();
        });

        if (walked==false || !ret) return cps_api_ret_code_ERR;
        _drain_queue();
    }

    cps_api_return_code_t result = _handle_delete_case(_dest_addr, src, cb, context, params, res);
    cps_api_object_delete(params.object_src);
    cps_api_object_delete(params.object_dest);

    return result;
}

