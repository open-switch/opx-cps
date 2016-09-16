

#include "cps_api_db_interface.h"

#include "cps_api_node_set.h"
#include "cps_api_node.h"

#include "cps_api_db.h"

#include "cps_api_key_utils.h"
#include "cps_api_object_tools.h"
#include "cps_class_map.h"
#include "cps_api_operation.h"
#include "dell-cps.h"


#include "event_log.h"

#include <vector>
#include <string>


namespace {

std::string _get_node_name(const std::string &str) {
    std::string s;
    if(!cps_api_db_get_node_from_ip(str,s)) {
        s = str;
    }
    return s;
}

bool _conn_event_enabled(cps_api_object_t filter) {
    return *(const bool*)cps_api_object_get_data(filter,CPS_CONNECTION_ENTRY_CONNECTION_STATE);
}

cps_api_return_code_t __cps_api_db_operation_get(cps_api_object_t obj, cps_api_object_list_t results) {
    cps_api_return_code_t rc=cps_api_ret_code_ERR;
    const char * group = cps_api_key_get_group(obj);
    cps_api_node_set_iterate(group,[&obj,&results,&rc,group](const std::string &name,void *c){
        cps_db::connection_request r(cps_db::ProcessDBCache(),name.c_str());

        if (!r.valid()) return ;
        size_t _ix = cps_api_object_list_size(results);
        rc = cps_db::get_objects(r.get(),obj,results);

        std::string node_name = _get_node_name(name);

        if (rc==cps_api_ret_code_OK) {
            size_t _mx = cps_api_object_list_size(results);
            for ( ; _ix < _mx ; ++_ix ) {
                cps_api_object_t _o = cps_api_object_list_get(results,_ix);
                STD_ASSERT(_o!=nullptr);
                cps_api_key_set_node(_o,node_name.c_str());
            }
        } else {
            bool _con_state_needed = _conn_event_enabled(obj);

            if (_con_state_needed) {
                cps_api_object_t o = cps_api_object_list_create_obj_and_append(results);
                cps_api_key_from_attr_with_qual(cps_api_object_key(o),CPS_CONNECTION_ENTRY,cps_api_qualifier_OBSERVED);
                cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_NAME,node_name.c_str(),node_name.size()+1);
                cps_api_object_attr_add(o,CPS_CONNECTION_ENTRY_IP,name.c_str(),name.size()+1);
                cps_api_key_set_group(o,group);
                cps_api_key_set_node(o,node_name.c_str());
            }
        }
    },nullptr);

    return rc;
}

}

bool cps_api_db_get_filter_enable_connection(cps_api_object_t obj) {
    bool _enabled=true;
    return cps_api_object_attr_add(obj,CPS_CONNECTION_ENTRY_CONNECTION_STATE,&_enabled,sizeof(_enabled))
            ==cps_api_ret_code_OK;
}

cps_api_return_code_t cps_api_db_get(cps_api_object_t obj,cps_api_object_list_t found) {
    return __cps_api_db_operation_get(obj,found);
}

cps_api_return_code_t cps_api_db_commit(cps_api_object_t o, cps_api_object_t previous, bool publish) {
    cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(o));
    if (op==cps_api_oper_ACTION) return cps_api_ret_code_ERR;

    cps_api_return_code_t rc = cps_api_ret_code_ERR;
    std::vector<std::string> lst;
    if (!cps_api_db_get_node_group(cps_api_key_get_group(o),lst)) {
        EV_LOGGING(DSAPI,ERR,"CPS-DB-IF","Failed to get node details.");
        return rc;
    }
    cps_api_key_del_node_attrs(o);


    cps_api_object_list_guard _lg(cps_api_object_list_create());

    cps_api_object_t _update = o;
    cps_api_object_t _event_obj = o;

    // if the operation is set, then the _update should be a merge of present and new attributes
    //if the operation is a create, then the raw object should be stored entirely without any additions
    //if the operation is a delete, then there is no need to have cache either

    if (previous!=nullptr || op==cps_api_oper_SET) {
        cps_api_object_t _present = nullptr;
        for (auto &it : lst ) {
            cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());

            if (op!=cps_api_oper_SET) {
                cps_db::get_object(r.get(),previous);
            } else {
                _present = cps_api_object_list_create_obj_and_append(_lg.get());
                if (cps_db::get_object(r.get(),_present)==cps_api_ret_code_OK) {
                    if (previous) cps_api_object_clone(previous,_present);

                    if (cps_api_obj_tool_merge(_present,o)) {
                        _update = _present;
                        break;
                    }
                }
            }
        }

    }

    cps_api_key_element_raw_value_monitor _key_patch(cps_api_object_key(_update),
            CPS_OBJ_KEY_ATTR_POS,cps_api_oper_NULL);
    std::string _failed_nodes;

    std::function<void(cps_db::connection_request &)> _handle_success = [&](cps_db::connection_request &req) -> void {
        if (publish) {
            _key_patch.reset();    //incase the event_obj is equal the object being published
            cps_db::publish(req.get(),_event_obj);
            _key_patch.set(cps_api_oper_NULL);
        }
        rc = cps_api_ret_code_OK;
    };

    auto _handle_failure = [&](std::string &ip, cps_db::connection_request &req) -> void {
        EV_LOGGING(DSAPI,INFO,"CPS-DB","Failed to update db %s with object (no connectivity)",ip.c_str());
        _failed_nodes+=","+_get_node_name(ip);
    };

    for (auto &it : lst ) {
        cps_db::connection_request r(cps_db::ProcessDBCache(),it.c_str());

        if (op==cps_api_oper_CREATE || op ==cps_api_oper_SET) {
            if (cps_db::store_object(r.get(),_update)!=cps_api_ret_code_OK) {
                _handle_failure(it,r);

            } else  {    //if stored properly
                _handle_success(r);
            }
        }
        if (op==cps_api_oper_DELETE) {
            bool _success=false;
            if (cps_api_object_count_key_attrs(o)==0) {
                cps_api_object_list_guard lg(cps_api_object_list_create());
                if (cps_db::get_objects(r.get(),o,lg.get())) {
                    _success = cps_db::delete_objects(r.get(),lg.get());
                }
            } else {
                _success = cps_db::delete_object(r.get(),o);
            }
            if (_success) {
                _handle_success(r);
            } else {
                _handle_failure(it,r);
            }
        }
    }
    _key_patch.reset();

    if (_failed_nodes.size()!=0) cps_api_object_attr_add(o,CPS_OBJECT_GROUP_FAILED_NODES,_failed_nodes.c_str(),_failed_nodes.size()+1);

    return rc;
}

