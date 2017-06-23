

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

#include "cps_api_python.h"


#include "cps_api_node.h"
#include "cps_api_operation_tools.h"
#include "cps_api_db_interface.h"

#include "private/cps_class_map_query.h"


#include "python2.7/Python.h"

#include <vector>
#include <functional>

PyObject * py_cps_api_db_commit(PyObject *self, PyObject *args, PyObject *_keydict) {
    PyObject *__obj=nullptr,*__prev=nullptr,*__pub=nullptr;

    const char *_keywords[]={"obj","prev","publish",NULL };

    if (! PyArg_ParseTupleAndKeywords( args, _keydict, "O!|OO!", (char**)_keywords, &PyDict_Type,&__obj,&__prev,&PyBool_Type,&__pub)) {
        PySys_WriteStdout("Failed to parse args.\n");
        return PyInt_FromLong(cps_api_ret_code_ERR);
    }

    if (__pub==nullptr) __pub = Py_True;
    cps_api_object_guard _obj(nullptr);
    cps_api_object_guard _prev(nullptr);

    if (PyDict_Check(__obj)) {
        _obj.set(dict_to_cps_obj(__obj));
    }

    if (!_obj.valid()) {
        PySys_WriteStdout("Passed in an invalid object.  Missing \'key\' or \'data' elements.\n");
        return PyInt_FromLong(cps_api_ret_code_ERR);
    }

    if (__prev!=nullptr && __prev!=Py_None && PyDict_Check(__prev)) {
        _prev.set(cps_api_object_create());
    }
    cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(_obj.get()));
    cps_api_return_code_t rc = cps_api_db_commit_one(op,_obj.get(),_prev.get(),__pub==Py_True);
    if (_prev.valid()) {
        PyRef r(cps_obj_to_dict(_prev.get()));
        PyDict_Clear(__prev);
        PyDict_Merge(__prev,r.get(),1);
    }
    return PyInt_FromLong(rc);
}

PyObject * py_cps_api_db_get(PyObject *self, PyObject *args) {
    PyObject * __obj= nullptr,*__list=nullptr;

    if (! PyArg_ParseTuple( args, "O!O!", &PyDict_Type, &__obj,&PyList_Type,&__list)) {
        py_set_error_string("Failed to parse input args.");
        return PyInt_FromLong(cps_api_ret_code_ERR);
    }

    cps_api_object_guard _filter(nullptr);

    if (PyDict_Check(__obj)) {
        _filter.set(dict_to_cps_obj(__obj));
        if (!_filter.valid()) {
            PySys_WriteStdout("Invalid filter object (missing key and or data fields).");
            return PyInt_FromLong(cps_api_ret_code_ERR);
        }
    } else {
        PySys_WriteStdout("Invalid object type... really should be impossible.");
        return PyInt_FromLong(cps_api_ret_code_ERR);
    }

    if (!PyList_Check(__list)) {
        PySys_WriteStdout("Invalid list object");
        return PyInt_FromLong(cps_api_ret_code_ERR);
    }

    cps_api_object_list_guard _lg(cps_api_object_list_create());

    cps_api_return_code_t rc = cps_api_db_get(_filter.get(),_lg.get());
    if (rc!=cps_api_ret_code_OK) {
        return PyInt_FromLong(rc);
    }

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(_lg.get());
    for ( ; ix < mx ; ++ix ) {
        cps_api_object_t obj = cps_api_object_list_get(_lg.get(),ix);
        PyObject *d = cps_obj_to_dict(obj);
        PyRef r(d);
        if (d==NULL) {
            PySys_WriteStdout("Memory allocation error.");
            return PyInt_FromLong(cps_api_ret_code_ERR);
        }
        if (PyList_Append(__list,d)) {
            py_set_error_string("Memory allocation error.");
            return PyInt_FromLong(cps_api_ret_code_ERR);
        }
    }

    return PyInt_FromLong(rc);
}

PyObject * py_cps_node_set_update(PyObject *self, PyObject *args) {
    const char *id=NULL, *node_type;
    PyObject *_list=nullptr;
    if (! PyArg_ParseTuple( args, "ssO!", &id, &node_type, &PyList_Type, &_list)) Py_RETURN_FALSE;

    auto _node_type = cps_node_type_from_string(node_type);
    if (_node_type==nullptr) {
        Py_RETURN_FALSE;
    }

    cps_api_node_group_t _group;
    _group.id = id;
    _group.data_type = *_node_type;

    std::vector<cps_api_node_ident> _ids;

    Py_ssize_t str_keys = PyList_Size(_list);
    {
        Py_ssize_t ix = 0;
        for ( ;ix < str_keys ; ++ix ) {
            PyObject *tupObj = PyList_GetItem(_list, ix);
            if (tupObj==nullptr) Py_RETURN_FALSE;
            PyObject *nameObj = PyTuple_GetItem(tupObj,0);
            PyObject *addrObj = PyTuple_GetItem(tupObj,1);
            if (nameObj==nullptr || addrObj==nullptr) Py_RETURN_FALSE;
            if (PyString_Check(nameObj) && PyString_Check(addrObj)) {
                cps_api_node_ident _id;
                _id.addr = PyString_AsString(addrObj);
                _id.node_name = PyString_AsString(nameObj);
                _ids.push_back(_id);
            }
        }
    }
    _group.addrs = &_ids[0];
    _group.addr_len = _ids.size();
    if (cps_api_set_node_group(&_group)==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


PyObject * py_cps_node_delete_group(PyObject *self, PyObject *args) {
    const char *id=NULL;
    if (! PyArg_ParseTuple( args, "s", &id)) Py_RETURN_FALSE;

    if (cps_api_delete_node_group(id)==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


PyObject * py_cps_node_set_master(PyObject *self, PyObject *args) {
    const char *id=NULL;
    const char *node_id=NULL;
    if (! PyArg_ParseTuple( args, "ss", &id,&node_id)) Py_RETURN_FALSE;

    if (cps_api_set_master_node(id,node_id)==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


PyObject * py_cps_node_set_ownership_type(PyObject *self, PyObject *args) {
    const char *key=NULL;
    const char *o_type=NULL;
    if (! PyArg_ParseTuple( args, "ss", &key,&o_type)) Py_RETURN_FALSE;

    auto _o_type = cps_class_owner_type_from_string(o_type);
    if (_o_type==nullptr) {
        Py_RETURN_FALSE;
    }

    cps_api_key_t k;
    cps_api_key_from_string(&k, key);

    cps_api_obj_set_ownership_type(&k,*_o_type);
    Py_RETURN_TRUE;
}


