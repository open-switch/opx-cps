

#include "cps_api_operation.h"
#include "cps_api_key.h"
#include "cps_class_map.h"

#include "private/cps_class_map_query.h"

#include "cps_api_events.h"
#include "cps_api_object_key.h"

#include "cps_api_python.h"

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



static const std::map<std::string,CPS_CLASS_ATTR_TYPES_t> _attr_types = {
        {"leaf",CPS_CLASS_ATTR_T_LEAF},
        {"leaf-list",CPS_CLASS_ATTR_T_LEAF_LIST },
        {"container",CPS_CLASS_ATTR_T_CONTAINER },
        {"subsystem",CPS_CLASS_ATTR_T_SUBSYSTEM },
        {"list",CPS_CLASS_ATTR_T_LIST },
};

static const std::map<std::string,CPS_CLASS_DATA_TYPE_t> _data_types = {
        {"uint8_t",CPS_CLASS_DATA_TYPE_T_UINT8},
        {"uint16_t",CPS_CLASS_DATA_TYPE_T_UINT16},
        {"uint32_t",CPS_CLASS_DATA_TYPE_T_UINT32},
        {"uint64_t",CPS_CLASS_DATA_TYPE_T_UINT64},
        {"int8_t",CPS_CLASS_DATA_TYPE_T_INT8},
        {"int16_t",CPS_CLASS_DATA_TYPE_T_INT16},
        {"int32_t",CPS_CLASS_DATA_TYPE_T_INT32},
        {"int64_t",CPS_CLASS_DATA_TYPE_T_INT64},
        {"string",CPS_CLASS_DATA_TYPE_T_STRING},
        {"enum",CPS_CLASS_DATA_TYPE_T_ENUM},
        {"bool",CPS_CLASS_DATA_TYPE_T_BOOL},
        {"obj-id",CPS_CLASS_DATA_TYPE_T_OBJ_ID},
        {"date",CPS_CLASS_DATA_TYPE_T_DATE},
        {"ipv4",CPS_CLASS_DATA_TYPE_T_IPV4},
        {"ipv6",CPS_CLASS_DATA_TYPE_T_IPV6},
        {"ip",CPS_CLASS_DATA_TYPE_T_IP},
        {"bin",CPS_CLASS_DATA_TYPE_T_BIN},
        {"double",CPS_CLASS_DATA_TYPE_T_DOUBLE},
        {"embedded",CPS_CLASS_DATA_TYPE_T_EMBEDDED},
};

struct _attr_type_match {
    uint32_t _val;
    template <typename T> bool operator()(T &it) const {
        return (uint32_t)it->second == _val;
    }
};

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
    if (! PyArg_ParseTuple( args, "ssssO!s", &_id,&path,&name,&desc,&PyBool_Type,&has_emb, &attr_type,&data_type )) return NULL;

    long long id = strtoll(_id,NULL,0);

    std::vector<cps_api_attr_id_t> ids;
    cps_class_ids_from_string(ids,path);

    cps_class_map_node_details details;
    details.desc = desc;
    details.name = name;
    details.embedded = has_emb != Py_False;
    if (_attr_types.find(attr_type)==_attr_types.end()) {
        Py_RETURN_FALSE;
    }
    if (_data_types.find(data_type)==_data_types.end()) {
        Py_RETURN_FALSE;
    }
    details.attr_type = _attr_types.at(attr_type);
    details.data_type = _data_types.at(data_type);
    if (cps_class_map_init((cps_api_attr_id_t)id,&ids[0],ids.size(),&details)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}


static PyObject * py_cps_info(PyObject *self, PyObject *args) {
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s", &path)) return NULL;

    PyRef d(PyDict_New());
    if (d.get()==nullptr) return nullptr;

    std::vector<cps_api_attr_id_t> v;
    cps_class_ids_from_string(v,path);

    cps_class_node_detail_list_t lst;
    cps_class_map_level(&v[0],v.size(),lst);

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

    auto attr_it = std::find_if(_attr_types.begin(),_attr_types.end(),[&ref]
        (const std::map<std::string,CPS_CLASS_ATTR_TYPES_t>::value_type &it){
            return (it.second == ref->attr_type);
        });
    if (attr_it!=_attr_types.end()) {
        py_cps_util_set_item_to_dict(dict,"attribute_type", PyString_FromString(attr_it->first.c_str()));
    }

    auto data_it = std::find_if(_data_types.begin(),_data_types.end(),[&ref]
        (const std::map<std::string,CPS_CLASS_DATA_TYPE_t>::value_type &it){
            return (it.second == ref->data_type);
        });
    if (data_it!=_data_types.end()) {
        py_cps_util_set_item_to_dict(dict,"data_type", PyString_FromString(data_it->first.c_str()));
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

    static const std::map<std::string,cps_api_qualifier_t> _cat = {
            { "target",cps_api_qualifier_TARGET },
            { "observed",cps_api_qualifier_OBSERVED },
            { "proposed",cps_api_qualifier_PROPOSED},
            { "realtime",cps_api_qualifier_REALTIME}
    };
    auto i  = _cat.find(cat);
    if (i==_cat.end()) {
        return PyString_FromString("");
    }

    const cps_class_map_node_details_int_t *ent = cps_dict_find_by_name(path);
    if (ent==nullptr) return PyString_FromString("");

    cps_api_key_t k;
    if (!cps_api_key_from_attr_with_qual(&k,ent->id,i->second)) {
        return PyString_FromString("");
    }
    char buff[CPS_API_KEY_STR_MAX];

    return PyString_FromString(cps_api_key_print(&k,buff,sizeof(buff)-1));
}


PyDoc_STRVAR(cps_get__doc__, "Perform a CPS get using an list of keys. Currently the keys should be in x.y.z format.");
PyDoc_STRVAR(cps_trans__doc__, "Perform a CPS transaction operation the dictionary provided. "
        " The dictionary needs to contain a dictionary of settings.");
PyDoc_STRVAR(cps_doc__, "A python interface to the CPS API");

PyDoc_STRVAR(cps_cps_generic_doc__, "A CPS mapping function.");

/* A list of all the methods defined by this module. */
/* "METH_VARGS" tells Python how to call the handler */
static PyMethodDef cps_methods[] = {
    {"init",  py_cps_map_init, METH_VARARGS, cps_cps_generic_doc__},
    {"convarray",  py_cps_byte_array_to_obj, METH_VARARGS, cps_cps_generic_doc__},
    {"arraykey",  py_cps_byte_array_key, METH_VARARGS, cps_cps_generic_doc__},
    {"convdict",  py_cps_obj_to_array, METH_VARARGS, cps_cps_generic_doc__},

    {"info",  py_cps_info, METH_VARARGS, cps_cps_generic_doc__},
    {"type",  py_cps_types, METH_VARARGS, cps_cps_generic_doc__},

    {"config",py_cps_config, METH_VARARGS, cps_cps_generic_doc__ },
    {"key_from_name",py_cps_key_from_name, METH_VARARGS, cps_cps_generic_doc__ },
    {"get_keys",py_cps_get_keys, METH_VARARGS, cps_cps_generic_doc__ },

    //Event processing
    {"event_register",  py_cps_event_reg, METH_VARARGS, cps_cps_generic_doc__},
    {"event_close",  py_cps_event_close, METH_VARARGS, cps_cps_generic_doc__},
    {"event_connect",  py_cps_event_connect, METH_VARARGS, cps_cps_generic_doc__},
    {"event_wait",  py_cps_event_wait, METH_VARARGS, cps_cps_generic_doc__},
    {"event_send",  py_cps_event_send, METH_VARARGS, cps_cps_generic_doc__},

    //object registration..
    {"obj_init",  py_cps_obj_init, METH_VARARGS, cps_cps_generic_doc__},
    {"obj_register",  py_cps_obj_reg, METH_VARARGS, cps_cps_generic_doc__},
    {"obj_close",  py_cps_obj_close, METH_VARARGS, cps_cps_generic_doc__},

    {"get",  py_cps_get, METH_VARARGS, cps_get__doc__},
    {"transaction",  py_cps_trans, METH_VARARGS, cps_trans__doc__},

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
