/*
 * Copyright (c) 2016 Dell Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 *  LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
 * FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
 *
 * See the Apache Version 2.0 License for specific language governing
 * permissions and limitations under the License.
 */



#include "cps_api_operation.h"
#include "cps_api_key.h"

#include "cps_class_map.h"
#include "private/cps_class_map_query.h"

#include "cps_api_events.h"
#include "cps_api_object_key.h"

#include "cps_api_python.h"
#include "cps_api_node.h"
#include "private/cps_string_utils.h"

#include <stdlib.h>
#include "python2.7/Python.h"
#include <stdio.h>
#include <vector>
#include <memory>
#include <map>
#include <string>
#include <algorithm>

PyObject *cps_error_exception = nullptr;

void py_set_error_string(const char * msg) {
    PyErr_SetString(cps_error_exception, msg);
}

static PyObject * py_cps_map_init(PyObject *self, PyObject *args) {
    cps_api_class_map_init();
    Py_RETURN_TRUE;
}

static PyObject * py_cps_byte_array_key(PyObject *self, PyObject *args) {
    PyObject *array;
    if (! PyArg_ParseTuple( args, "O!", &PyByteArray_Type, &array)) return NULL;
    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    if (!cps_api_array_to_object(PyByteArray_AsString(array), PyByteArray_Size(array),obj)) {
        return PyString_FromString("");
    }
    if (!cps_api_object_received(obj,PyByteArray_Size(array))) {
        return PyString_FromString("");
    }

    return PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str());
}

static PyObject * py_cps_byte_array_to_obj(PyObject *self, PyObject *args) {
    PyObject *array;
    if (! PyArg_ParseTuple( args, "O!", &PyByteArray_Type, &array)) return NULL;

    cps_api_object_t obj = cps_obj_from_array(array);
    cps_api_object_guard og(obj);
    if (!og.valid()) return PyDict_New();

    return cps_obj_to_dict(obj);
}

static PyObject * py_cps_obj_to_array(PyObject *self, PyObject *args) {
    const char *path="";
    PyObject *d;
    if (! PyArg_ParseTuple( args, "sO!",&path,&PyDict_Type, &d)) return NULL;

    cps_api_object_t obj = dict_to_cps_obj(path,d);
    cps_api_object_guard og(obj);

    if (!og.valid()) return PyByteArray_FromStringAndSize((const char *)NULL,0);

    return PyByteArray_FromStringAndSize((const char *)cps_api_object_array(obj),
            cps_api_object_to_array_len(obj));
}


static PyObject * py_cps_config(PyObject *self, PyObject *args) {
    const char * path=NULL, *name, *desc,*_id,*attr_type,*data_type;
    PyObject *has_emb;
    //ID
    //path
    //name
    //desc
    //embedded
    //attr_type
    //data_type
    if (! PyArg_ParseTuple( args, "ssssO!ss", &_id,&path,&name,&desc,&PyBool_Type,&has_emb, &attr_type,&data_type )) return NULL;

    long long id = strtoll(_id,NULL,0);

    std::vector<cps_api_attr_id_t> ids;
    cps_class_ids_from_string(ids,path);

    cps_class_map_node_details details;
    details.desc = desc;
    details.name = name;
    details.embedded = has_emb != Py_False;

    const CPS_CLASS_DATA_TYPE_t *_data_type = cps_class_data_type_from_string(data_type);
    const CPS_CLASS_ATTR_TYPES_t *_attr_type = cps_class_attr_type_from_string(attr_type);
    if (_data_type==nullptr || _attr_type==nullptr) {
        Py_RETURN_FALSE;
    }
    details.attr_type = *_attr_type;
    details.data_type = *_data_type;
    if (cps_class_map_init((cps_api_attr_id_t)id,&ids[0],ids.size(),&details)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}


static PyObject * py_cps_info(PyObject *self, PyObject *args) {
    PyObject *bool_cur_level=nullptr;
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s|O!", &path, &PyBool_Type, &bool_cur_level)) return NULL;

    PyRef d(PyDict_New());
    if (d.get()==nullptr) return nullptr;

    std::vector<cps_api_attr_id_t> v;

    const cps_class_map_node_details_int_t * ref = cps_dict_find_by_name(path);
    if (ref!=nullptr) {
        //then assume that it is a valid object id
        v = ref->ids;
    } else {
        cps_class_ids_from_string(v,path);
    }

    cps_class_node_detail_list_t lst;

    cps_class_map_level(&v[0],v.size(),lst,bool_cur_level!=Py_False);

    PyRef names(PyDict_New());
    PyRef ids(PyDict_New());

    if (names.get()==nullptr || ids.get()==nullptr) {
        return nullptr;
    }

    if (!py_cps_util_set_item_to_dict(d.get(),"names",names.get(),false)) return nullptr;
    if (!py_cps_util_set_item_to_dict(d.get(),"ids",ids.get(),false)) return nullptr;

    size_t ix = 0;
    size_t mx = lst.size();
    for ( ; ix < mx ; ++ix ) {
        char buff[40]; //just enough for the attribute id
        snprintf(buff,sizeof(buff),"%lld",(long long)lst[ix].id);
        py_cps_util_set_item_to_dict(ids.get(),buff,
                PyString_FromString(lst[ix].full_path.c_str()));
        py_cps_util_set_item_to_dict(names.get(),lst[ix].full_path.c_str(),
                PyString_FromString(buff));
    }
    names.release();
    ids.release();

    return d.release();
}

static PyObject * py_cps_enabled(PyObject *self, PyObject *args) {
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s",&path)) return NULL;

    cps_api_key_t key;

    cps_api_key_from_string(&key,path);
    if (cps_api_is_registered(&key,NULL)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}


static PyObject * py_cps_stats(PyObject *self, PyObject *args) {
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s",&path)) return NULL;

    cps_api_key_t key;
    cps_api_key_from_string(&key,path);

    cps_api_object_guard og(cps_api_object_create());
    if (cps_api_object_stats(&key,og.get())==cps_api_ret_code_OK) {
        return cps_obj_to_dict(og.get());
    }

    return PyDict_New();
}

static PyObject * py_cps_types(PyObject *self, PyObject *args) {
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s", &path)) return NULL;

    PyRef d(PyDict_New());

    if (d.get()==NULL) return NULL;

    const cps_class_map_node_details_int_t * ref = cps_dict_find_by_name(path);

    if (ref==nullptr) {
        std::vector<cps_api_attr_id_t> v;
        cps_class_ids_from_string(v,path);
        if (v.size()> 0 ) {
            ref = cps_dict_find_by_id(v[v.size()-1]);
        }
    }
    if (ref==nullptr) {
        py_set_error_string("Invalid key specified - no dictionary entry found.");
        return NULL;
    }
    PyObject *dict = d.get();
    d.release();

    py_cps_util_set_item_to_dict(dict,"name", PyString_FromString(ref->full_path.c_str()));
    py_cps_util_set_item_to_dict(dict,"key", PyString_FromString(cps_class_ids_to_string(ref->ids).c_str()));
    py_cps_util_set_item_to_dict(dict,"description", PyString_FromString(ref->desc.c_str()));
    py_cps_util_set_item_to_dict(dict,"embedded", PyString_FromString(ref->embedded?"True":"False"));

    {
    static const size_t ID_BUFF_LEN=100;
    char buff[ID_BUFF_LEN];
    snprintf(buff,sizeof(buff),"%" PRId64,(int64_t)ref->id);
    py_cps_util_set_item_to_dict(dict,"id", PyString_FromString(buff));
    }

    const char *_attr_type = cps_class_attr_type_to_string(ref->attr_type);
    const char *_data_type = cps_class_data_type_to_string(ref->data_type);
    if (_attr_type!=nullptr) {
        py_cps_util_set_item_to_dict(dict,"attribute_type", PyString_FromString(_attr_type));
    }
    if (_data_type!=nullptr) {
        py_cps_util_set_item_to_dict(dict,"data_type", PyString_FromString(_data_type));
    }

    return dict;
}

static PyObject * py_cps_get_keys(PyObject *self, PyObject *args) {
    PyObject *d = NULL;
    if (! PyArg_ParseTuple( args, "O!",&PyDict_Type, &d)) return NULL;

    PyObject *new_dict = PyDict_New();
    if(new_dict==NULL) return NULL;

    cps_api_object_t obj = dict_to_cps_obj(d);
    cps_api_object_guard g(obj);
    if (!g.valid()) return new_dict;

    cps_api_key_t *k = cps_api_object_key(obj);

    size_t mx = cps_api_key_get_len(k);
    size_t ix = 0;
    for ( ; ix < mx ; ++ix ) {
        cps_api_attr_id_t id = cps_api_key_element_at(k,ix);
        cps_api_object_attr_t a = cps_api_get_key_data(obj,id);
        if (a==NULL) continue;
        py_dict_set_from_attr(new_dict,id,a);
    }
    return new_dict;
}

static PyObject * py_cps_key_from_name(PyObject *self, PyObject *args) {
    const char * path=NULL, *cat=NULL;
    if (! PyArg_ParseTuple( args, "ss", &cat, &path)) return NULL;

    const cps_api_qualifier_t * _op_type = cps_class_qual_from_string(cat);
    if (_op_type==nullptr) return PyString_FromString("");


    const cps_class_map_node_details_int_t *ent = cps_dict_find_by_name(path);
    if (ent==nullptr) return PyString_FromString("");

    cps_api_key_t k;
    if (!cps_api_key_from_attr_with_qual(&k,ent->id,*_op_type)) {
        return PyString_FromString("");
    }
    char buff[CPS_API_KEY_STR_MAX];

    return PyString_FromString(cps_api_key_print(&k,buff,sizeof(buff)-1));
}

static PyObject * py_key_from_qualifer(PyObject *self, PyObject *args) {
    const char *cat=NULL;
    if (! PyArg_ParseTuple( args, "s", &cat)) return NULL;

    const cps_api_qualifier_t * _op_type = cps_class_qual_from_string(cat);
    if (_op_type==nullptr) return PyString_FromString("");

    std::string _key = cps_string::sprintf("%d.",(int)*_op_type);
    return PyString_FromString(_key.c_str());
}

static PyObject * py_cps_node_set_update(PyObject *self, PyObject *args) {
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

static PyObject * py_cps_name_from_key(PyObject *self, PyObject *args) {
    const char * key=NULL;
    int offset = 0;

    if (! PyArg_ParseTuple( args, "si", &key, &offset)) return NULL;

    cps_api_key_t k;
    cps_api_key_from_string(&k, key);

    const char *path = NULL;
    path = cps_class_string_from_key(&k, offset);
    if (path == NULL)
        path = "";

    return PyString_FromString(path);

}

static PyObject * py_cps_qual_from_key(PyObject *self, PyObject *args) {

    const char * key=NULL;
    if (! PyArg_ParseTuple( args, "s", &key)) return NULL;

    cps_api_key_t k;
    cps_api_key_from_string(&k, key);

    const char *qualifier = NULL;
    qualifier = cps_class_qual_from_key(&k);
    if (qualifier == NULL)
        qualifier = "";

    return PyString_FromString(qualifier);
}

#define CPS_FN_DOC(x) x##_doc

PyDoc_STRVAR(cps_get__doc__, "get(fl_list,resp_list)\n\n"
    "Perform a get using an list of objects which contains keys and filters\n"
    "@fl_list - list which contains the CPS objects along with CPS keys\n"
    "           and filter data\n"
    "@resp_list - list which is will contain returned the CPS objects\n"
    "             when get operation was successful\n"
    "@return - True if successful otherwise False");

PyDoc_STRVAR(cps_trans__doc__, "transaction(op_list)\n\n"
    "Perform CPS transactions with operations and objects specified\n"
    "in the op_list\n"
    "@tr_list - list of transactions in form of python dictionary\n"
    "           which contains the operation to be performed and\n"
    "           the object which has the relevant data for that\n"
    "           operation\n"
    "@return - True if successful otherwise False");

PyDoc_STRVAR(cps_doc__, "A Python interface to the CPS API");

PyDoc_STRVAR(cps_cps_generic_doc__, "A CPS mapping function.");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_byte_array_key), "arraykey(ba)\n\n"
    "Return the CPS key from bytearray 'ba'.\n"
    "Return empty string if the bytearray does not have a valid CPS key.\n"
    "@ba - byte array");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_map_init), "init()\n\n"
    "Initialize the CPS class map API. This API is deprecated.");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_byte_array_to_obj), "convarray(ba)\n\n"
    "Return a python dictionary containing both 'key' and 'data' elements\n"
    "from bytearray 'ba'.\n"
    "Return an empty python dictionary if bytearray\n"
    "does not have a valid key or data.\n"
    "@ba - byte array");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_obj_to_array), "convdict(key,data)\n\n"
    "Returns a bytearray with a CPS key and Python dictionary containing\n"
    "valid 'data' elements\n"
    "@key - CPS key\n"
    "@data - data");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_info), "info(key,no_child_info=True)\n\n"
    "Given either a key string or a object element name, return the list\n"
    "of attributes and ids that it contains. Optionally pass False to\n"
    "get contained attributes and its id of object's children as well.");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_types), "type(key)\n\n"
    "Return extended details on a specific attribute including type,id,\n"
    "description,key,name and whether attribute is embedded.\n"
    "The key can be a numeric string or a full attribute name.\n"
    "@key - CPS key");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_enabled), "enabled(key)\n\n"
    "Given a key, determine whether there is an object registration.\n"
    "Returns True if key is registered else False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_stats), "stats(key)\n\n"
    "Retrieve the CPS statistics object(number of set/get requests,\n"
    "connection failure invalid requests...) for the given key.\n"
    "@key - CPS key");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_config), "config(id,key,name,description,is_embedded,yang_type,data_type)\n\n"
    "Configure a custom attribute to class mapping\n"
    "@id - numeric id of the attribute\n"
    "@key - key of the attribute in 'x.x.x.x....' format\n"
    "@name - name of the attribute\n"
    "@description - description of the attribute\n"
    "@is_embedded - whether attribute is embedded (True) or not (False)\n"
    "@yang_type - YANG type of the attribute (leaf,leaf-list,container..)\n"
    "@data_type  - data type of the attribute (bool,bin,unt8_t,...)\n"
    "@return - True if added to class mapping otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_key_from_name), "key_from_name(qualifier,object_path)\n\n"
    "Returns a CPS key for a given qualifier and object path\n"
    "@qualifier - qualifier for the key(target,observed..) \n"
    "@object_path - Complete object path\n"
    "@return - CPS key if successful otherwise returns empty string\n");

PyDoc_STRVAR(CPS_FN_DOC(py_key_from_qualifer), "key_from_qual(qualifier)\n\n"
    "Returns a CPS wildcard key for a given qualifier\n"
    "@qualifier - qualifier for the key(target,observed..) \n"
    "@return - CPS key if successful otherwise returns empty string\n");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_name_from_key), "name_from_key(key, offset)\n\n"
    "Take a CPS key, search the CPS meta data for a appropriate key path and return the string if found\n"
    "@key - the key for which the string path is required \n"
    "@offset - the offset from where the raw key starts"
    "@return - complete object path if successful otherwise returns empty string\n");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_qual_from_key), "qual_from_key(key)\n\n"
    "Take a CPS key, search the CPS qualifier and return the qualifier as a string if found\n"
    "@key - the key for which the qualifier string is required \n"
    "@return - the character string holding the qualifier if successful otherwise returns empty string\n");


PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_connect), "event_connect()\n\n"
    "Returns a CPS handle to register send/receive events");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_wait), "event_wait(handle)\n\n"
    "Wait for the registered event to be published\n"
    "@handle - CPS event handle\n"
    "@return - published CPS object");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_close), "event_close(handle)\n\n"
    "Close the CPS event handle\n"
    "@handle - CPS event handle\n"
    "@return - True if event handle was closed successfully otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_reg), "event_reg(handle,key)\n\n"
    "Register to be notified when an event for the given CPS key is published.\n"
    "@handle - CPS event handle\n"
    "@key - CPS key of object\n"
    "@return - True if event handle was closed successfully otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_reg_object), "event_register_object(handle,object)\n\n"
    "Register to be notified when an event for the given object details are discovered - only support registering for key attributes or groups.\n"
    "@handle - CPS event handle\n"
    "@object - the python dictionary containing the \"key\" and \"data\" fields of an object\n"
    "@return - True if event handle was closed successfully otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_event_send), "event_send(handle,obj)\n\n"
    "Send an event object using the CPS event handle.\n"
    "@handle - CPS event handle\n"
    "@obj - object to be send with valid CPS key\n"
    "@return - True if event handle was closed successfully otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_obj_init), "obj_init()\n\n"
    "Return a CPS object registration handle to register callback.\n"
    "functions related to CPS operations");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_obj_reg), "obj_register(handle,key,callbacks)\n\n"
    "Register a get/set callback for the CPS key.\n"
    "@handle - CPS object registration handle\n"
    "@key - CPS key for the object\n"
    "@callbacks - Python dictionary with 'get' and 'transaction' as key\n"
    "             and its callback function as its value\n"
    "@return - True if successful otherwise False");

PyDoc_STRVAR(CPS_FN_DOC(py_cps_node_set_update), "node_set_update(set_name,list_of_addresses)\n\n"
    "Using the addresses (and ports) specified, create a node set.\n"
    "@set_name - the name of the collection of nodes\n"
    "@list_of_addresses - Python list of IP addresses and ports.\n"
    "@return - True if successful otherwise False");


/* A list of all the methods defined by this module. */
/* "METH_VARGS" tells Python how to call the handler */
static PyMethodDef cps_methods[] = {
    {"init",  py_cps_map_init, METH_VARARGS, CPS_FN_DOC(py_cps_map_init)},
    {"convarray",  py_cps_byte_array_to_obj, METH_VARARGS, CPS_FN_DOC(py_cps_byte_array_to_obj)},
    {"arraykey",  py_cps_byte_array_key, METH_VARARGS, CPS_FN_DOC(py_cps_byte_array_key)},
    {"convdict",  py_cps_obj_to_array, METH_VARARGS, CPS_FN_DOC(py_cps_obj_to_array)},

    {"info",  py_cps_info, METH_VARARGS, CPS_FN_DOC(py_cps_info)},
    {"type",  py_cps_types, METH_VARARGS, CPS_FN_DOC(py_cps_types)},
    {"enabled", py_cps_enabled, METH_VARARGS, CPS_FN_DOC(py_cps_enabled)},


    {"config",py_cps_config, METH_VARARGS, CPS_FN_DOC(py_cps_config) },
    {"key_from_name",py_cps_key_from_name, METH_VARARGS, CPS_FN_DOC(py_cps_key_from_name) },
    {"get_keys",py_cps_get_keys, METH_VARARGS, cps_cps_generic_doc__ },
    {"key_from_qual",py_key_from_qualifer,METH_VARARGS,CPS_FN_DOC(py_key_from_qualifer) },

    {"name_from_key",py_cps_name_from_key, METH_VARARGS, CPS_FN_DOC(py_cps_name_from_key) },
    {"qual_from_key",py_cps_qual_from_key, METH_VARARGS, CPS_FN_DOC(py_cps_qual_from_key) },

    //Event processing
    {"event_register",  py_cps_event_reg, METH_VARARGS, CPS_FN_DOC(py_cps_event_reg)},
    {"event_register_object",py_cps_event_reg_object,METH_VARARGS, CPS_FN_DOC(py_cps_event_reg_object)},
    {"event_close",  py_cps_event_close, METH_VARARGS, CPS_FN_DOC(py_cps_event_close)},
    {"event_connect",  py_cps_event_connect, METH_VARARGS, CPS_FN_DOC(py_cps_event_connect)},
    {"event_wait",  py_cps_event_wait, METH_VARARGS, CPS_FN_DOC(py_cps_event_wait)},
    {"event_send",  py_cps_event_send, METH_VARARGS, CPS_FN_DOC(py_cps_event_send)},

    //object registration..
    {"obj_init",  py_cps_obj_init, METH_VARARGS, CPS_FN_DOC(py_cps_obj_init)},
    {"obj_register",  py_cps_obj_reg, METH_VARARGS, CPS_FN_DOC(py_cps_obj_reg)},
    {"obj_close",  py_cps_obj_close, METH_VARARGS, cps_cps_generic_doc__},
    {"obj_stats", py_cps_stats, METH_VARARGS, CPS_FN_DOC(py_cps_stats)},

    {"get",  py_cps_get, METH_VARARGS, cps_get__doc__},
    {"transaction",  py_cps_trans, METH_VARARGS, cps_trans__doc__},

    {"node_set_update",  py_cps_node_set_update, METH_VARARGS, CPS_FN_DOC(py_cps_node_set_update)},

    {NULL, NULL}      /* sentinel */
};

PyMODINIT_FUNC initcps(void) {
    PyObject *m = Py_InitModule3("cps", cps_methods, cps_doc__);
    if (m==NULL) return;

    if (! PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }

    cps_error_exception = PyErr_NewException((char*)"cps.error", NULL, NULL);
    Py_INCREF(cps_error_exception);
    PyModule_AddObject(m, "error", cps_error_exception);
    cps_api_event_service_init();
}
