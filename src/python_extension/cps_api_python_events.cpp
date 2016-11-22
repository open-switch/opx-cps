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
 * cps_api_python_events.cpp
 *
 *  Created on: Jul 28, 2015
 */

#include "cps_api_python.h"
#include "cps_api_events.h"

#include "Python.h"
#include <stdio.h>

const static size_t MAX_EVENT_BUFF=20000;

PyObject * py_cps_event_connect(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t handle=NULL;
    if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) {
        return PyByteArray_FromStringAndSize(NULL,0);
    }
    return PyByteArray_FromStringAndSize((const char *)&handle,sizeof(handle));
}

PyObject * py_cps_event_reg(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    const char *path;
    if (! PyArg_ParseTuple( args, "O!s",  &PyByteArray_Type, &o,&path)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return nullptr;
    }

    cps_api_event_reg_t reg;
    cps_api_key_t key;

    cps_api_key_from_string(&key,path);
    reg.number_of_objects =1;
    reg.objects = &key;
    reg.priority = 0;

    cps_api_return_code_t rc;

    {
        NonBlockingPythonContext l;
        rc = cps_api_event_client_register(*handle,&reg);
    }

    if (rc==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * py_cps_event_reg_object(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o, *dict;

    if (! PyArg_ParseTuple( args, "O!O!",  &PyByteArray_Type, &o,&PyDict_Type, &dict)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return nullptr;
    }
    cps_api_object_list_guard lg(cps_api_object_list_create());

    cps_api_object_t obj = dict_to_cps_obj(dict);
    if (!cps_api_object_list_append(lg.get(),obj)) {
        cps_api_object_delete(obj);
        return nullptr;
    }
    cps_api_return_code_t rc;

    {
        NonBlockingPythonContext l;
        rc = cps_api_event_client_register_object(*handle,lg.get());
    }

    if (rc==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

PyObject * py_cps_event_close(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return nullptr;
    }
    {
        NonBlockingPythonContext l;
        cps_api_event_client_disconnect(*handle);
    }
    Py_RETURN_TRUE;
}

PyObject * py_cps_event_wait(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return nullptr;
    }

    cps_api_object_t obj=cps_api_object_create();
    cps_api_object_guard og(obj);
    cps_api_object_reserve(obj,MAX_EVENT_BUFF);

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_wait_for_event(*handle,obj);
    }

    if (rc==cps_api_ret_code_OK) {
        return (cps_obj_to_dict(obj));
    }
    fprintf(stderr,"No object received.\n");
    return PyDict_New();
}

PyObject * py_cps_event_send(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *dict,*h;

    if (! PyArg_ParseTuple( args, "O!O!",  &PyByteArray_Type, &h,&PyDict_Type, &dict)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(h);
    if (PyByteArray_Size(h)!=sizeof(*handle)) {
        return nullptr;
    }

    cps_api_object_t obj = dict_to_cps_obj(dict);

    cps_api_object_guard og(obj);
    if (!og.valid()) {
        return nullptr;
    }

    cps_api_return_code_t rc;
    {
        NonBlockingPythonContext l;
        rc = cps_api_event_publish(*handle,obj);
    }
    if (rc ==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

