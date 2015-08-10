/** OPENSOURCELICENSE */
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
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyDict_Check(d)) return false;
    if (PyDict_SetItemString(d,item,o)) {
        return false;
    }
    r.release();
    return true;
}

bool py_cps_util_set_item_to_list(PyObject *l, size_t ix , PyObject *o , bool gc) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_SetItem(l,ix,o)) {
        return false;
    }
    r.release();
    return true;
}

bool py_cps_util_append_item_to_list(PyObject *l, PyObject *o , bool gc ) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_Append(l,o)) {
        return false;
    }
    r.release();
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
                    ent = par_ent;
                }
            }

            if (ent==nullptr) {
                PyObject *by  = PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                        cps_api_object_attr_len(it->attr));
                py_cps_util_set_item_to_dict(d,name.c_str(),by);
                break;
            }

            if (ent->embedded) {
                PyRef r (PyDict_New());
                if (r.get()==nullptr) break;
                cps_api_object_it_t contained_it = *it;
                cps_api_object_it_inside(&contained_it);

                std::vector<cps_api_attr_id_t> cpy = ids;
                cpy.push_back(id);
                py_obj_dump_level(r.get(),&contained_it,cpy);
                py_cps_util_set_item_to_dict(d,name.c_str(),r.get());
                r.release();
                break;
            }

            if ((ent->attr_type & CPS_CLASS_ATTR_T_LEAF_LIST)==CPS_CLASS_ATTR_T_LEAF_LIST) {
                PyObject *o = PyDict_GetItemString(d,name.c_str());
                if (o == NULL) {
                    o = PyList_New(0);
                }
                PyList_Append(o,PyByteArray_FromStringAndSize(
                        (const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
                PyDict_SetItemString(d,name.c_str(),o);
                break;
            }

            py_cps_util_set_item_to_dict(d,name.c_str(),
                 PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
        } while (0);
        cps_api_object_it_next(it);
    }
}

static std::map<std::string,cps_api_operation_types_t> _op_types = {
        { "delete",cps_api_oper_DELETE},
        { "create",cps_api_oper_CREATE},
        { "set",cps_api_oper_SET},
        { "action",cps_api_oper_ACTION}
};

PyObject * cps_obj_to_dict(cps_api_object_t obj) {
    if (obj==NULL) return PyDict_New();

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj, &it);

    PyObject *d = PyDict_New();

    if (d==NULL) return NULL;
    std::vector<cps_api_attr_id_t> ids;
    py_obj_dump_level(d,&it,ids);

    PyObject *o = PyDict_New();
    PyRef _o(o);

    py_cps_util_set_item_to_dict(o,"data",d);
    py_cps_util_set_item_to_dict(o,"key",PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));

    cps_api_operation_types_t type = cps_api_object_type_operation(cps_api_object_key(obj));

    auto a_it = std::find_if(_op_types.begin(),_op_types.end(),[&type]
        (const std::map<std::string,cps_api_operation_types_t>::value_type &val) {
            return val.second ==type;
        });
    if (a_it!=_op_types.end()) {
        py_cps_util_set_item_to_dict(o,"operation",PyString_FromString(a_it->first.c_str()));
    }

    _o.release();
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
        auto it = _op_types.find(PyString_AsString(op));
        if (it==_op_types.end()) {
            py_set_error_string("Invalid operation type specified on converted object.");
            return nullptr;
        }
        _op = it->second;
    }

    cps_api_object_guard og(dict_to_cps_obj(PyString_AsString(key),d));

    cps_api_object_set_type_operation(cps_api_object_key(og.get()),_op);

    return og.release();
}

