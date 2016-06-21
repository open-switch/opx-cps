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

/*
 * cps_api_python_utils.cpp
 *
 *  Created on: Jul 28, 2015
 */

#include "cps_api_python.h"
#include "private/cps_class_map_query.h"
#include "private/cps_dictionary.h"
#include "cps_class_map.h"

#include "Python.h"
#include <vector>
#include <map>
#include <algorithm>

const static unsigned int CUSTOM_KEY_POS = 2;

bool py_cps_util_set_item_to_dict(PyObject *d, const char * item, PyObject *o, bool gc) {
    //always clean up the element after inserting into the dict since the dict will increase the ref count
    PyRef r(o);
    if (!gc) r.release();

    if (!PyDict_Check(d)) return false;
    if (PyDict_SetItemString(d,item,o)) {
        return false;
    }
    return true;
}

bool py_cps_util_set_item_to_list(PyObject *l, size_t ix , PyObject *o , bool gc) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_SetItem(l,ix,o)) {
        return false;
    }
    return true;
}

bool py_cps_util_append_item_to_list(PyObject *l, PyObject *o , bool gc ) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_Append(l,o)) {
        return false;
    }
    return true;
}


cps_api_object_t cps_obj_from_array(PyObject *array) {
    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    if (!cps_api_array_to_object(PyByteArray_AsString(array), PyByteArray_Size(array),obj)) {
        return NULL;
    }
    if (!cps_api_object_received(obj,PyByteArray_Size(array))) {
        return NULL;
    }
    return og.release();
}

std::string py_str_from_attr_id(cps_api_attr_id_t id) {
    const cps_class_map_node_details_int_t * ent = cps_dict_find_by_id(id);
    if (ent==nullptr) {
        static const size_t ID_BUFF_LEN=100;
        char buff[ID_BUFF_LEN];
        snprintf(buff,sizeof(buff),"%" PRId64,(int64_t)id);
        return std::string(buff);
    }
    return ent->full_path;
}

void py_dict_set_from_attr(PyObject *d, cps_api_attr_id_t id, cps_api_object_attr_t attr) {
    std::string key = py_str_from_attr_id(id);

    PyObject *by = PyByteArray_FromStringAndSize(
            (const char *)cps_api_object_attr_data_bin(attr),
                            cps_api_object_attr_len(attr));
    py_cps_util_set_item_to_dict(d,key.c_str(),by);
}

static void py_obj_dump_level(PyObject * d, cps_api_object_it_t *it, std::vector<cps_api_attr_id_t> &ids) {
    cps_api_key_t key;
    memset(&key,0,sizeof(key));

    while (cps_api_object_it_valid(it)) {
        do {

            cps_api_attr_id_t id = cps_api_object_attr_id(it->attr);

            const cps_class_map_node_details_int_t * ent = cps_dict_find_by_id(id);
            const cps_class_map_node_details_int_t * par_ent =
                    ids.size()>0 ? cps_dict_find_by_id(ids[ids.size()-1]) : nullptr;

            std::string name = py_str_from_attr_id(id);

            if (ent==nullptr && par_ent!=nullptr) {
                if (par_ent->attr_type==CPS_CLASS_ATTR_T_LIST) {
                    cps_api_object_it_t contained_it = *it;
                    cps_api_object_it_inside(&contained_it);
                    if (cps_api_object_it_valid(&contained_it)) {
                        ent = par_ent;
                    }
                }
            }

            if (ent==nullptr) {
                PyObject *by  = PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                        cps_api_object_attr_len(it->attr));
                py_cps_util_set_item_to_dict(d,name.c_str(),by);
                break;
            }

            if (ent->embedded) {
                PyObject *container = (PyDict_New());
                if (container==nullptr) break;

                cps_api_object_it_t contained_it = *it;
                cps_api_object_it_inside(&contained_it);

                std::vector<cps_api_attr_id_t> cpy = ids;
                cpy.push_back(id);
                py_obj_dump_level(container,&contained_it,cpy);
                py_cps_util_set_item_to_dict(d,name.c_str(),container);
                break;
            }

            if ((ent->attr_type & CPS_CLASS_ATTR_T_LEAF_LIST)==CPS_CLASS_ATTR_T_LEAF_LIST) {
                bool _created = false;
                PyObject *o = PyDict_GetItemString(d,name.c_str());
                if (o == NULL) {
                    o = PyList_New(0);
                    _created=true;
                }
                PyObject *_tmp = PyByteArray_FromStringAndSize(
                        (const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr));
                PyList_Append(o,_tmp);
                Py_DECREF(_tmp);

                PyDict_SetItemString(d,name.c_str(),o);

                if (_created) Py_DECREF(o);

                break;
            }

            py_cps_util_set_item_to_dict(d,name.c_str(),
                 PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
        } while (0);
        cps_api_object_it_next(it);
    }
}


PyObject * cps_obj_to_dict(cps_api_object_t obj) {
    if (obj==NULL) return PyDict_New();

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj, &it);

    PyObject *d = PyDict_New();

    if (d==NULL) return NULL;
    std::vector<cps_api_attr_id_t> ids;
    py_obj_dump_level(d,&it,ids);

    PyObject *o = PyDict_New();
    py_cps_util_set_item_to_dict(o,"data",d);

    PyObject *_key_str = PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str());
    py_cps_util_set_item_to_dict(o,"key",_key_str);


    cps_api_operation_types_t type = cps_api_object_type_operation(cps_api_object_key(obj));
    const char * _type = cps_operation_type_to_string(type);

    if (_type!=nullptr) {
        py_cps_util_set_item_to_dict(o,"operation",PyString_FromString(_type));
    }

    return o;
}

static bool add_bytearray_to_obj(cps_api_object_t obj, cps_api_attr_id_t *ids, size_t len, PyObject *o) {
    if (!PyByteArray_Check(o)) return false;
    return cps_api_object_e_add(obj,ids,len,cps_api_object_ATTR_T_BIN,
                        PyByteArray_AsString(o), PyByteArray_Size(o));
}

static void add_to_object(std::vector<cps_api_attr_id_t> &level,
        cps_api_object_t obj, PyObject *d, size_t start_level ) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    size_t depth = level.size();
    level.resize(depth+1);
    while (PyDict_Next(d, &pos, &key, &value)) {
        const char * k = PyString_AS_STRING(key);

        const cps_class_map_node_details_int_t *ent = cps_dict_find_by_name(k);
        if (ent!=nullptr) {
            level[depth] = ent->id;
        } else {
            level[depth] = strtoll(k,NULL,0);
        }

        if (PyDict_Check(value)) {
            std::vector<cps_api_attr_id_t> v = level;
            add_to_object(v,obj,value,start_level);
            continue;
        }

        cps_api_attr_id_t *ids =&level[start_level];
        size_t id_len = level.size()-start_level;

        if (PyList_Check(value)) {
            size_t ix = 0;
            size_t mx = PyList_Size(value);
            for ( ; ix < mx ; ++ix ) {
                PyObject *o = PyList_GetItem(value,ix);
                if (o==NULL) continue;
                add_bytearray_to_obj(obj,ids,id_len,o);
            }
        }
        if (PyByteArray_Check(value)) {
            add_bytearray_to_obj(obj,ids,id_len,value);
            continue;
        }
        if (PyString_Check(value)) {
            cps_api_object_e_add(obj,ids,id_len,cps_api_object_ATTR_T_BIN,
                    PyString_AS_STRING(value), PyString_GET_SIZE(value));
            continue;
        }
    }
}

cps_api_object_t dict_to_cps_obj(const char *path, PyObject *dict) {

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    std::vector<cps_api_attr_id_t> level;

    add_to_object(level,obj,dict,level.size());

    cps_api_key_from_string(cps_api_object_key(obj),path);

    return og.release();
}

cps_api_object_t dict_to_cps_obj(PyObject *dict) {
    if (!PyDict_Check(dict)) return NULL;

    PyObject * key = PyDict_GetItemString(dict,"key");
    if (key==NULL || !PyString_Check(key)) return NULL;

    PyObject * d = PyDict_GetItemString(dict,"data");
    if (d==NULL || !PyDict_Check(d)) return NULL;

    PyObject * op = PyDict_GetItemString(dict,"operation");
    if (op!=NULL && !PyString_Check(op)) return NULL;

    cps_api_operation_types_t _op = (cps_api_operation_types_t)(0);
    if (op!=nullptr) {
    	const cps_api_operation_types_t* __op = cps_operation_type_from_string(PyString_AsString(op));
    	if (__op==nullptr) _op = *__op;
    }

    cps_api_object_guard og(dict_to_cps_obj(PyString_AsString(key),d));

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),_op);

    return og.release();
}

