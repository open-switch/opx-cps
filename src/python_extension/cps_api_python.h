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
 * cps_api_python.h
 *
 *  Created on: Jul 28, 2015
 */

#ifndef CPS_API_SRC_PYTHON_EXTENSION_CPS_API_PYTHON_H_
#define CPS_API_SRC_PYTHON_EXTENSION_CPS_API_PYTHON_H_

#include "cps_api_object.h"

#include "Python.h"

#include <string>

class PyRef {
    PyObject *_o;
public:
    PyRef(PyObject *o,bool inc=false) {
        _o = o;
        if (_o!=NULL && inc) Py_INCREF(_o);
    }
    void decref() {
        if (_o!=NULL) Py_DECREF(_o);
        release();
    }

    bool valid() { return _o != NULL; }
    PyObject * release() {
        PyObject *r =_o;
        _o = NULL;
        return r;
    }
    void set(PyObject *o) {
        decref();
        _o = o;
    }
    PyObject * get() { return _o; }
    ~PyRef() { decref(); }
};

class GILLock {
    PyGILState_STATE _gstate;
public:
    GILLock() {
        _gstate=PyGILState_Ensure();
    }
    ~GILLock() {
        PyGILState_Release(_gstate);
    }
};

struct py_callbacks_t {
    PyObject * _methods;

    bool contains(const char *method) {
        PyObject *o = PyDict_GetItemString(_methods,method);
        return ((o!=NULL) && (PyCallable_Check(o)));
    }

    PyObject *func(const char * method, PyObject *args) {
        if (!contains(method)) {
            return NULL;
        }

        PyObject *o = PyDict_GetItemString(_methods,method);
        PyObject *ret = PyObject_CallObject(o,args);
        Py_DECREF(args);
        return ret;
    }

    PyObject *execute(const char * method, PyObject *param) {
        PyObject * args = Py_BuildValue("OO", _methods,param);
        return func(method, args);
    }

    PyObject *execute(const char * method, PyObject *sync_param, PyObject *param) {
        PyObject * args = Py_BuildValue("OOO", _methods,sync_param,param);
        return func(method, args);
    }

};


class NonBlockingPythonContext {
     PyThreadState *_save = nullptr;
public:

     NonBlockingPythonContext() {
        _save = PyEval_SaveThread();
    }

    ~NonBlockingPythonContext() {
        PyEval_RestoreThread(_save);
    }
};

/**
 * Error messages can be set with the following method
 */
void py_set_error_string(const char * msg) ;


/**
 * Events
 */
PyObject * py_cps_event_connect(PyObject *self, PyObject *args);
PyObject * py_cps_event_reg(PyObject *self, PyObject *args) ;
PyObject * py_cps_event_reg_object(PyObject *self,PyObject *args);
PyObject * py_cps_event_close(PyObject *self, PyObject *args);
PyObject * py_cps_event_wait(PyObject *self, PyObject *args);
PyObject * py_cps_event_send(PyObject *self, PyObject *args);


/**
 * CPS Operations
 */
PyObject * py_cps_obj_close(PyObject *self, PyObject *args);
PyObject * py_cps_obj_reg(PyObject *self, PyObject *args);
PyObject * py_cps_obj_init(PyObject *self, PyObject *args);
PyObject * py_cps_trans(PyObject *self, PyObject *args);
PyObject * py_cps_get(PyObject *self, PyObject *args);

/**
 * CPS DB specifc functions
 */
PyObject * py_cps_node_set_update(PyObject *self, PyObject *args);
PyObject * py_cps_node_delete_group(PyObject *self, PyObject *args);
PyObject * py_cps_node_set_ownership_type(PyObject *self, PyObject *args);
PyObject * py_cps_node_set_master(PyObject *self, PyObject *args);
PyObject * py_cps_sync(PyObject *self, PyObject *args);
PyObject * py_cps_api_db_commit(PyObject *self, PyObject *args, PyObject *_keydict) ;
PyObject * py_cps_api_db_get(PyObject *self, PyObject *args);


/**
 * Utilities
 */
bool py_cps_util_set_item_to_dict(PyObject *d, const char * item, PyObject *o, bool gc=true) ;
bool py_cps_util_set_item_to_list(PyObject *l, size_t ix , PyObject *o , bool gc=true);
bool py_cps_util_append_item_to_list(PyObject *l, PyObject *o , bool gc = true);


cps_api_object_t cps_obj_from_array(PyObject *array);
cps_api_object_t dict_to_cps_obj(PyObject *dict);
cps_api_object_t dict_to_cps_obj(const char *path, PyObject *dict) ;
PyObject * cps_obj_to_dict(cps_api_object_t obj) ;

void py_dict_set_from_attr(PyObject *d, cps_api_attr_id_t id, cps_api_object_attr_t attr);

std::string py_str_from_attr_id(cps_api_attr_id_t id);



#endif /* CPS_API_SRC_PYTHON_EXTENSION_CPS_API_PYTHON_H_ */
