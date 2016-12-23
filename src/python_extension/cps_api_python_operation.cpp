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
 * cps_api_python_operation.cpp
 *
 *  Created on: Jul 29, 2015
 */

#include "cps_api_operation.h"
#include "cps_api_operation_tools.h"
#include "cps_api_node.h"
#include "cps_api_python.h"
#include "private/cps_class_map_query.h"

#include "Python.h"
#include <map>
#include <memory>

PyObject * py_cps_get(PyObject *self, PyObject *args) {
    PyObject * param_list;

    cps_api_get_params_t gr;
    if (cps_api_get_request_init (&gr)==cps_api_ret_code_ERR) {
        py_set_error_string("Failed to initialize the get req");
        return nullptr;
    }

    cps_api_get_request_guard rg(&gr);

    PyObject *res_obj;

    if (! PyArg_ParseTuple( args, "O!O", &PyList_Type, &param_list,&res_obj)) {
        py_set_error_string("Failed to parse input args.");
        return nullptr;
    }

    PyObject * lst = NULL;

    if (PyDict_Check(res_obj)) {
        PyObject *l = PyList_New(0);
        PyRef _l(l);
        if (l==NULL) {
            py_set_error_string("Can not create a list.");
            return nullptr;
        }
        PyObject *_prev = PyDict_GetItemString(res_obj,"list");
        if (_prev!=NULL) {
            PyDict_DelItemString(res_obj,"list");
        }

        if (!py_cps_util_set_item_to_dict(res_obj,"list",l)) {
            py_set_error_string("Can not create a list.");
            return nullptr;
        }
        lst = l;
    }
    if (PyList_Check(res_obj)) {
        lst = res_obj;
    }
    if (lst==NULL) {
        py_set_error_string("The return args are invalid.");
        return nullptr;
    }
    Py_ssize_t str_keys = PyList_Size(param_list);
    {
        Py_ssize_t ix = 0;
        for ( ;ix < str_keys ; ++ix ) {
            PyObject *strObj = PyList_GetItem(param_list, ix);
            if (PyString_Check(strObj)) {
                //
                cps_api_object_t o = cps_api_object_list_create_obj_and_append(gr.filters);
                if (o==NULL) {
                    py_set_error_string("Memory allocation error.");
                    return nullptr;
                }
                if (!cps_api_key_from_string(cps_api_object_key(o),PyString_AsString(strObj))) {
                    py_set_error_string("Memory allocation error.");
                    return nullptr;
                }
            }
            if (PyDict_Check(strObj)) {
                cps_api_object_t o = dict_to_cps_obj(strObj);
                if (o==NULL) {
                    py_set_error_string("Can't convert from a python to internal object");
                    return nullptr;
                }
                if (!cps_api_object_list_append(gr.filters,o)) {
                    cps_api_object_delete(o);
                    py_set_error_string("Memory allocation error.");
                    return nullptr;
                }
            }
        }
    }
    gr.keys = NULL;
    gr.key_count = 0;

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_get(&gr);
    }

    if (rc!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(gr.list);
    for ( ; ix < mx ; ++ix) {
        cps_api_object_t obj = cps_api_object_list_get(gr.list,ix);
        PyObject *d = cps_obj_to_dict(obj);
        PyRef r(d);
        if (d==NULL) {
            py_set_error_string("Memory allocation error.");
            return nullptr;
        }
        if (PyList_Append(lst,d)) {
            py_set_error_string("Memory allocation error.");
            return nullptr;
        }
    }

    Py_RETURN_TRUE;
}

static bool py_add_object_to_trans( cps_api_transaction_params_t *tr, PyObject *dict) {
    PyObject *_req = PyDict_GetItemString(dict,"change");
    PyObject *_op = PyDict_GetItemString(dict,"operation");

    if (_op==NULL || _req==NULL) {
        return false;
    }
    PyObject *_prev = PyDict_GetItemString(dict,"previous");
    if (_prev!=NULL) {
        if (cps_api_object_list_size(tr->change_list)!=
                cps_api_object_list_size(tr->prev)) {
            return false;
        }
        cps_api_object_guard og(dict_to_cps_obj(_req));
        if (!og.valid()) return false;
        if (!cps_api_object_list_append(tr->prev,og.get())) {
            return false;
        }
        og.release();
    }

    using cps_oper = cps_api_return_code_t (*)(cps_api_transaction_params_t * trans,
            cps_api_object_t object);

    static std::map<std::string,cps_oper> trans = {
            {"delete",cps_api_delete },
            {"create",cps_api_create},
            {"set",cps_api_set},
            {"rpc",cps_api_action}
    };

    if (trans.find(PyString_AsString(_op))==trans.end()) {
        return false;
    }

    cps_oper oper = trans[PyString_AsString(_op)];

    cps_api_object_t obj = dict_to_cps_obj(_req);
    cps_api_object_guard og(obj);
    if (!og.valid()) return false;

    if (oper(tr,obj)!=cps_api_ret_code_OK) {
        return false;
    }
    og.release();
    return true;
}

PyObject * py_cps_trans(PyObject *self, PyObject *args) {
    PyObject *list;
    if (! PyArg_ParseTuple( args, "O!",  &PyList_Type, &list)) {
        return nullptr;
    }

    cps_api_transaction_params_t tr;
    if (cps_api_transaction_init(&tr)!=cps_api_ret_code_OK) {
        py_set_error_string("Memory allocation error.");
        return nullptr;
    }
    cps_api_transaction_guard pg(&tr);

    size_t ix = 0;
    size_t mx = PyList_Size(list);
    for ( ; ix < mx ; ++ix ) {
        PyObject *dict = PyList_GetItem(list,ix);
        if (!py_add_object_to_trans(&tr,dict)) {
            py_set_error_string("Could not translate the request to a transaction");
            return nullptr;
        }
    }

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_commit(&tr);
    }
    if (rc!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }

    ix = 0;
    mx = PyList_Size(list);
    for ( ; ix < mx ; ++ix ) {
        PyObject *dict = PyList_GetItem(list,ix);
        if (dict==NULL || !PyDict_Check(dict)) {
            py_set_error_string("Failed to convert the transaction response.");
            return nullptr;
        }
        PyObject *_req = PyDict_GetItemString(dict,"change");
        if (_req==NULL || !PyDict_Check(_req)) {
            py_set_error_string("Failed to convert the transaction response.");
            return nullptr;
        }

        cps_api_object_t obj = cps_api_object_list_get(tr.change_list,ix);
        if (obj==NULL) {
            py_set_error_string("Failed to convert the transaction response.");
            return nullptr;
        }
        py_cps_util_set_item_to_dict(_req,"key",
                PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));

        if (PyDict_GetItemString(dict,"change")!=NULL)
            PyDict_DelItemString(dict,"change");

        py_cps_util_set_item_to_dict(dict,"change",cps_obj_to_dict(obj));
    }

    cps_api_transaction_close(&tr);

    Py_RETURN_TRUE;
}

PyObject * py_cps_obj_init(PyObject *self, PyObject *args) {
    cps_api_operation_handle_t handle;

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_operation_subsystem_init(&handle,1);
    }
    if (rc!=cps_api_ret_code_OK) {
        py_set_error_string("Failed to initialize the infrastructure.");
        return nullptr;
    }
    return PyByteArray_FromStringAndSize((const char *)&handle,sizeof(handle));
}

static cps_api_return_code_t _read_function (void * context, cps_api_get_params_t * param,
        size_t key_ix) {
    py_callbacks_t *cb = (py_callbacks_t*)context;

    GILLock gil;

    PyObject *p = PyDict_New();
    PyRef dict(p);

    py_cps_util_set_item_to_dict(p,"filter",cps_obj_to_dict(cps_api_object_list_get(
            param->filters,key_ix)));

    py_cps_util_set_item_to_dict(p,"list",PyList_New(0));

    PyObject * res = cb->execute("get",p);
    PyRef ret(res);
    if (res==NULL || !PyBool_Check(res) || (Py_False == (res))) {
        return cps_api_ret_code_ERR;
    }

    PyObject *d = PyDict_GetItemString(p,"list");
    if (d!=NULL && PyList_Check(d)) {
        size_t ix = 0;
        size_t mx = PyList_Size(d);
        for ( ; ix < mx ; ++ix ) {
            PyObject *o = PyList_GetItem(d,ix);
            if (o==NULL || !PyDict_Check(o)) continue;
            cps_api_object_guard og(dict_to_cps_obj(o));
            if (og.valid() && cps_api_object_list_append(param->list,og.get())) {
                og.release();
                continue;
            }
            return cps_api_ret_code_ERR;
        }
    }
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _write_function(void * context, cps_api_transaction_params_t * param,size_t ix) {
    GILLock gil;

    cps_api_object_t obj = cps_api_object_list_get(param->change_list,ix);
    if (obj==NULL) return cps_api_ret_code_ERR;

    cps_api_object_t prev = cps_api_object_list_get(param->prev,ix);
    if(prev==NULL) {
        prev = cps_api_object_list_create_obj_and_append(param->prev);
    }
    if (prev==NULL) return cps_api_ret_code_ERR;

    cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(obj));

    PyObject *p = PyDict_New();
    PyRef dict(p);

    py_cps_util_set_item_to_dict(p,"change",cps_obj_to_dict(obj));
    py_cps_util_set_item_to_dict(p,"previous",cps_obj_to_dict(prev));

    static const std::map<int,std::string> trans = {
            {cps_api_oper_DELETE,"delete" },
            {cps_api_oper_CREATE,"create"},
            {cps_api_oper_SET, "set"},
            {cps_api_oper_ACTION,"rpc"}
    };

    py_cps_util_set_item_to_dict(p,"operation",PyString_FromString(trans.at(op).c_str()));

    py_callbacks_t *cb = (py_callbacks_t*)context;
    PyObject *res = cb->execute("transaction",p);

    if (res==NULL || !PyBool_Check(res) || (Py_False==(res))) {
        return cps_api_ret_code_ERR;
    }
    PyRef ret(res);

    PyObject *ch = PyDict_GetItemString(p,"change");
    if (ch==NULL) return cps_api_ret_code_ERR;

    cps_api_object_t o = dict_to_cps_obj(ch);
    cps_api_object_guard og(o);
    if(og.valid()) {
        cps_api_object_clone(obj,og.get());
    }

    PyObject *pr = PyDict_GetItemString(p,"previous");
    if (pr==NULL) return cps_api_ret_code_ERR;
    og.set(dict_to_cps_obj(pr));
    if (og.valid()) {
        cps_api_object_clone(prev,og.get());
    }
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _rollback_function(void * context,
        cps_api_transaction_params_t * param,
        size_t index_of_element_being_updated) {
    GILLock gil;
    return cps_api_ret_code_ERR;
}

PyObject * py_cps_obj_reg(PyObject *self, PyObject *args) {
    cps_api_operation_handle_t *handle=NULL;
    PyObject *h,*o;
    const char *path;
    if (! PyArg_ParseTuple( args, "O!sO!",  &PyByteArray_Type, &h,&path,&PyDict_Type, &o)) return NULL;

    handle = (cps_api_operation_handle_t*)PyByteArray_AsString(h);
    if (PyByteArray_Size(h)!=sizeof(*handle)) {
        py_set_error_string("Invalid handle");
        return nullptr;
    }
    std::unique_ptr<py_callbacks_t> p (new py_callbacks_t);
    if (p.get()==NULL) {
        py_set_error_string("Memory allocation error");
        return nullptr;
    }
    p->_methods = o;

    cps_api_registration_functions_t f;
    memset(&f,0,sizeof(f));
    f.handle = *handle;
    f.context = p.get();
    f._read_function = _read_function;
    f._write_function = _write_function;
    f._rollback_function = _rollback_function;

    if (!cps_api_key_from_string(&f.key,path)) {
        py_set_error_string("Key translation error");
        return nullptr;
    }

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_register(&f);
    }

    if (rc!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    p.release();
    Py_INCREF(o);

    Py_RETURN_TRUE;
}

PyObject * py_cps_obj_close(PyObject *self, PyObject *args) {
    cps_api_operation_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!s",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_operation_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}

static bool _sync_function(void *context, cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_response_t *res) {
    
    PyObject *p = PyDict_New();
    //PyRef dict(p);  
    
    static const std::map<int,std::string> op = {
        {cps_api_oper_DELETE,"delete" },
        {cps_api_oper_CREATE,"create"},
        {cps_api_oper_SET, "set"},
        {cps_api_oper_ACTION,"rpc"}
    };
    py_cps_util_set_item_to_dict(p,"opcode",PyString_FromString(op.at(params->opcode).c_str()));
    py_cps_util_set_item_to_dict(p,"src_node", PyString_FromString(params->src_node));
    py_cps_util_set_item_to_dict(p,"dest_node", PyString_FromString(params->dest_node)); 
    py_cps_util_set_item_to_dict(p,"object_dest",cps_obj_to_dict(params->object_dest));
    py_cps_util_set_item_to_dict(p,"object_src",cps_obj_to_dict(params->object_src));
    
    PyObject *r = PyDict_New();
    //PyRef dict(r);

    static const std::map<int, std::string> change = {
        {cps_api_make_change, "make_change"},
        {cps_api_no_change, "no_change"}            
    };
    
    static const std::map<int, std::string> change_notify = {
        {cps_api_raise_event, "raise_event" },
        {cps_api_raise_no_event, "raise_no_event"}
    };
    
    py_cps_util_set_item_to_dict(r,"change",PyString_FromString(change.at(res->change).c_str()));
    py_cps_util_set_item_to_dict(r,"change_notify",PyString_FromString(change_notify.at(res->change_notify).c_str()));
    
    py_callbacks_t *cb = (py_callbacks_t*)context;
    
    PyObject *result = cb->execute("sync", p, r);
    
    if (result==NULL || !PyBool_Check(result) || (Py_False==(result))) {
        return false;
    }

    PyObject* ch = PyDict_GetItem(r, PyString_FromString("change"));
    PyObject* ch_notify = PyDict_GetItem(r, PyString_FromString("change_notify"));

    for(auto const& it : change) {
        if (it.second == PyString_AsString(ch))
            res->change = (cps_api_dbchange_t)it.first;
    }

    for(auto const& it : change_notify) {
        if (it.second == PyString_AsString(ch_notify))
            res->change_notify = (cps_api_dbchange_notify_t)it.first;
    }
    
    return true;    
}


static bool _error_function(void *context, cps_api_db_sync_cb_param_t *params, cps_api_db_sync_cb_error_t *err) {
    PyObject *p = PyDict_New();
    //PyRef dict(p); 
    
    py_cps_util_set_item_to_dict(p,"src_node", PyString_FromString(params->src_node));
    py_cps_util_set_item_to_dict(p,"dest_node", PyString_FromString(params->dest_node));
    
    PyObject *e = PyDict_New();
    //PyRef dict(e);
    
    static const std::map<int,std::string> error = {
        {cps_api_db_no_connection,"no_connection" }    
    };
    py_cps_util_set_item_to_dict(e,"error", PyString_FromString(error.at(err->err_code).c_str()));
    
    py_callbacks_t *cb = (py_callbacks_t*)context;
    PyObject *result = cb->execute("error", p, e);
    
    if (result==NULL || !PyBool_Check(result) || (Py_False==(result))) {
        return false;
    }
    return true;
}


PyObject * py_cps_sync(PyObject *self, PyObject *args) {

    PyObject *dest_dict, *src_dict;
    PyObject *cb;
    
    if (! PyArg_ParseTuple( args, "O!O!O!",  &PyDict_Type, &dest_dict,&PyDict_Type, &src_dict, &PyDict_Type, &cb)) return NULL;
    
    
    cps_api_object_t dest_obj = dict_to_cps_obj(dest_dict);
    cps_api_object_t src_obj = dict_to_cps_obj(src_dict);
    
    std::unique_ptr<py_callbacks_t> p (new py_callbacks_t);
    if (p.get()==NULL) {
        py_set_error_string("Memory allocation error");
        return nullptr;
    }
    p->_methods = cb;
    
    void *context = p.get();

    cps_api_return_code_t rc;
    rc = cps_api_sync(context, dest_obj, src_obj, _sync_function, _error_function);
    if (rc!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    
    p.release();
    
    Py_RETURN_TRUE;
}
