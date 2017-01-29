

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

cps_api_return_code_t cps_api_sync(void *context, cps_api_object_t dest, cps_api_object_t src,  cps_api_sync_callback_t cb, cps_api_sync_error_callback_t err_cb)
{
    cps_api_db_sync_cb_param_t params;
    cps_api_db_sync_cb_response_t res;
    cps_api_db_sync_cb_error_t er;

    params.src_node = cps_api_key_get_node(src);
    params.dest_node = cps_api_key_get_node(dest);

    res.change = cps_api_make_change;
    res.change_notify = cps_api_raise_event;

    const char *_src_addr;
    const char *_dest_addr;

    cps_api_nodes n;
    n.load();

    if(params.dest_node == nullptr && params.src_node == nullptr) {
        _src_addr = _dest_addr = DEFAULT_REDIS_ADDR;
    } else {
        _src_addr = n.addr(params.src_node);
        _dest_addr = n.addr(params.dest_node);
    }

    cps_db::connection_request _dest_conn(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn.valid() || !cps_db::ping(_dest_conn.get())) return cps_api_ret_code_ERR;

    cps_db::connection_request _dest_conn_meta(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn_meta.valid() || !cps_db::ping(_dest_conn_meta.get())) return cps_api_ret_code_ERR;

    std::vector<char> key;
    bool _is_wildcard =false;
    if (!cps_db::dbkey_instance_or_wildcard(key, src, _is_wildcard)) {
        return cps_api_ret_code_ERR;
    }

    const char *field = "dirty";
    const char *data = "false";
    if(!cps_db::for_each_store_field(_dest_conn_meta.get(), key, field, std::strlen(field), data, std::strlen(data)))
        return cps_api_ret_code_ERR;

    params.object_src = cps_api_object_create();
    params.object_dest = cps_api_object_create();

    bool is_cb = false;
    size_t count = 0;
    bool ret = true;

    std::vector<std::string> _key_cache;
    bool walked = false;

    {
        cps_db::connection_request _source_get(cps_db::ProcessDBCache(),_src_addr);
        if (!_source_get.valid() || !cps_db::ping(_source_get.get())) {
            return cps_api_ret_code_ERR;
        }
        auto _drain_queue = [&]() {
            for(size_t ix = 0; ix < count; ++ix) {
                bool _rc_a = cps_db::get_object_response(_dest_conn.get(), params.object_dest);
                bool _rc_b = cps_db::get_object_response(_source_get.get(), params.object_src);

                if (_rc_b  && _rc_a) {
                    //Set case
                    if( (!cps_db::dbkey_field_delete_request(_dest_conn_meta.get(), _key_cache[ix].c_str(), _key_cache[ix].size(), field, std::strlen(field) )) ||
                        (!cps_db::dbkey_field_delete_response(_dest_conn_meta.get()))) return;
                    params.opcode = cps_api_oper_SET;
                    if(!cps_api_obj_tool_matches_filter(params.object_src,params.object_dest,true)) is_cb = true;
                } else if (!_rc_a) {
                    //Create case
                    params.opcode = cps_api_oper_CREATE;
                    is_cb = true;
                } else {
                    continue;
                }
                if(is_cb) {
                    if((cb(context, &params, &res))) {
                        if(res.change == cps_api_make_change) {
                            cps_db::store_object(_dest_conn_meta.get(), params.object_src);
                            if(res.change_notify == cps_api_raise_event) {
                                cps_db::publish(_dest_conn_meta.get(), params.object_src);
                            }
                        }
                    }
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

    count = 0;
    ret = true;

    cps_db::connection_request _dest_conn_cleanup(cps_db::ProcessDBCache(),_dest_addr);
    if (!_dest_conn_cleanup.valid() || !cps_db::ping(_dest_conn_cleanup.get())) return cps_api_ret_code_ERR;

    auto _process_get_response = [&]() {
       for(size_t ix=0; ix < count; ++ix) {
           std::string val = cps_db::dbkey_field_get_response_string(_dest_conn_meta.get());
           if(val.compare("")) {
               //Delete case
                params.opcode = cps_api_oper_DELETE;
                std::vector<char> _k(_key_cache[ix].begin(), _key_cache[ix].end());
                cps_db::get_object(_dest_conn_cleanup.get(), _k, params.object_dest);
                if(cb(context, &params, &res)) {
                    if(res.change == cps_api_make_change) {
                        cps_db::delete_object(_dest_conn_cleanup.get(), params.object_dest);
                        if (cps_db::dbkey_field_delete_request(_dest_conn_cleanup.get(), _key_cache[ix].c_str(), _key_cache[ix].size(),
                                                           field, std::strlen(field))
                            && cps_db::dbkey_field_delete_response(_dest_conn_cleanup.get())) {
                            if(res.change_notify == cps_api_raise_event)
                                cps_db::publish(_dest_conn_cleanup.get(), params.object_dest);
                        }
                    }
                }
           }
       }
       count = 0;
       _key_cache.clear();
    };

    walked = cps_db::walk_keys(_dest_conn.get(),&key[0],key.size(),[&](const void *inst_key, size_t len) {
        if(!ret) return;
        if(!cps_db::dbkey_field_get_request(_dest_conn_meta.get(), (char *)inst_key, len, field, std::strlen(field))) {
            ret = false;
        } else ++count;
        _key_cache.push_back(std::string((char*)inst_key,len));
        if(count < cps_db::IN_THE_PIPE()) return;
        _process_get_response();
    });

    if (walked == false || !ret) return cps_api_ret_code_ERR;
    _process_get_response();

    cps_api_object_delete(params.object_src);
    cps_api_object_delete(params.object_dest);

    return cps_api_ret_code_OK;
}

