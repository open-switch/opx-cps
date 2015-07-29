/** OPENSOURCELICENSE */
/*
 * cps_api_python_utils.cpp
 *
 *  Created on: Jul 28, 2015
 */

#include "cps_api_python.h"
#include "cps_class_map_query.h"
#include "cps_class_map.h"

#include "Python.h"
#include <vector>

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

void py_dict_set_from_attr(PyObject *d, cps_api_attr_id_t id, cps_api_object_attr_t attr) {
    char buff[100]; //enough space to write an int :)
    const char * name = cps_attr_id_to_name(id);
    if (name==NULL) {
        snprintf(buff,sizeof(buff),"%llu",(long long unsigned int)id);
        name = buff;
    }
    PyObject *by = PyByteArray_FromStringAndSize(
            (const char *)cps_api_object_attr_data_bin(attr),
                            cps_api_object_attr_len(attr));
    py_cps_util_set_item_to_dict(d,name,by);
}

static void py_obj_dump_level(PyObject * d, std::vector<cps_api_attr_id_t> &parent, cps_api_object_it_t *it) {

    std::vector<cps_api_attr_id_t> cur = parent;
    size_t level = cur.size();
    cur.push_back((cps_api_attr_id_t)0);
    cps_api_key_t key;
    memset(&key,0,sizeof(key));

    while (cps_api_object_it_valid(it)) {
        do {
            static const size_t ID_BUFF_LEN=100;
            char buff[ID_BUFF_LEN];

            cur[level] = cps_api_object_attr_id(it->attr);
            snprintf(buff,sizeof(buff)-1,"%d",(int)cur[level]);

            cps_class_map_node_details_int_t details;
            bool found = cps_class_map_detail(cur[level],details);

            const char * name = found ? details.full_path.c_str() : nullptr;

            if (!cps_class_attr_is_valid(&cur[0],cur.size())) {
                PyObject *by  = PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                        cps_api_object_attr_len(it->attr));
                py_cps_util_set_item_to_dict(d,buff,by);
                break;
            }
            if (name == nullptr) {
                name = buff;
            }

            if (details.embedded) {
                PyObject * subd = PyDict_New();
                if (subd==NULL) break;
                cps_api_object_it_t contained_it = *it;
                cps_api_object_it_inside(&contained_it);
                py_obj_dump_level(subd,cur,&contained_it);
                py_cps_util_set_item_to_dict(d,name,subd);
                break;
            }

            if ((details.attr_type & CPS_CLASS_ATTR_T_LEAF_LIST)==CPS_CLASS_ATTR_T_LEAF_LIST) {
                PyObject *o = PyDict_GetItemString(d,name);
                if (o == NULL) {
                    o = PyList_New(0);
                }
                PyList_Append(o,PyByteArray_FromStringAndSize(
                        (const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
                PyDict_SetItemString(d,name,o);
                break;
            }

            py_cps_util_set_item_to_dict(d,name,
                 PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
        } while (0);
        cps_api_object_it_next(it);
    }
}

PyObject * cps_obj_to_dict(cps_api_object_t obj) {
    if (obj==NULL) return PyDict_New();
    cps_api_key_t *key = cps_api_object_key(obj);

    std::vector<cps_api_attr_id_t> lst;
    cps_class_ids_from_key(lst,key);

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj, &it);

    PyObject *d = PyDict_New();

    if (d==NULL) return NULL;
    py_obj_dump_level(d,lst,&it);

    PyObject *o = PyDict_New();
    PyRef _o(o);

    py_cps_util_set_item_to_dict(o,"data",d);
    py_cps_util_set_item_to_dict(o,"key",PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));

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

        bool valid_name = false;

        level[depth] = cps_name_to_attr(k,valid_name);

        if (!valid_name) {
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
    cps_class_ids_from_string(level,path);
    if (level.size()>(CPS_OBJ_KEY_INST_POS+1)) level.erase(level.begin());
    if (level.size()>CUSTOM_KEY_POS) level.resize(CUSTOM_KEY_POS);
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
    return dict_to_cps_obj(PyString_AsString(key),d);
}

