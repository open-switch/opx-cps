

#include "cps_api_operation.h"
#include "cps_api_key.h"
#include "cps_class_map.h"
#include "cps_class_map_query.h"
#include "cps_api_events.h"
#include "cps_api_object_key.h"

#include <stdlib.h>
#include "python2.7/Python.h"
#include <stdio.h>
#include <vector>
#include <memory>
#include <map>
#include <string>


const static size_t MAX_EVENT_BUFF=20000;
const static unsigned int CUSTOM_KEY_POS = 2;

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
    void release() { _o = NULL;}
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
namespace {
bool SetItemToDict(PyObject *d, const char * item, PyObject *o, bool gc=true) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyDict_Check(d)) return false;
    if (PyDict_SetItemString(d,item,o)) {
        return false;
    }
    r.release();
    return true;
}

bool SetItemToList(PyObject *l, size_t ix , PyObject *o , bool gc = true) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_SetItem(l,ix,o)) {
        return false;
    }
    r.release();
    return true;
}

bool AppendItemToList(PyObject *l, PyObject *o , bool gc = true) {
    PyRef r(o);
    if (gc==false) r.release();

    if (!PyList_Check(l)) return false;
    if (PyList_Append(l,o)) {
        return false;
    }
    r.release();
    return true;
}

}

struct py_callbacks_t {
    PyObject * _methods;

    bool contains(const char *method) {
        PyObject *o = PyDict_GetItemString(_methods,method);
        return ((o!=NULL) && (PyCallable_Check(o)));
    }

    PyObject *execute(const char * method, PyObject *param) {
        if (!contains(method)) {
            return NULL;
        }

        PyObject *o = PyDict_GetItemString(_methods,method);
        PyObject * args = Py_BuildValue("OO", _methods,param);
        PyObject *ret = PyObject_CallObject(o,args);
        Py_DECREF(args);

        return ret;
    }
};

static cps_api_object_t cps_obj_from_array(PyObject *array) {
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

static void py_dict_set_from_attr(PyObject *d, cps_api_attr_id_t id, cps_api_object_attr_t attr) {
    char buff[100]; //enough space to write an int :)
    const char * name = cps_attr_id_to_name(id);
    if (name==NULL) {
        snprintf(buff,sizeof(buff),"%llu",(long long unsigned int)id);
        name = buff;
    }
    PyObject *by = PyByteArray_FromStringAndSize(
            (const char *)cps_api_object_attr_data_bin(attr),
                            cps_api_object_attr_len(attr));
    SetItemToDict(d,buff,by);
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
                SetItemToDict(d,buff,by);
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
                SetItemToDict(d,name,subd);
                break;
            }

            if (details.type & CPS_CLASS_ATTR_T_LEAF_LIST) {
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

            SetItemToDict(d,name,
                 PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
        } while (0);
        cps_api_object_it_next(it);
    }
}

static PyObject * cps_obj_to_dict(cps_api_object_t obj) {
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

    SetItemToDict(o,"data",d);
    SetItemToDict(o,"key",PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));

    _o.release();
    return o;
}

static void add_to_object(std::vector<cps_api_attr_id_t> &level,
        cps_api_object_t obj, PyObject *d, size_t start_level ) {
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    size_t depth = level.size();
    level.resize(depth+1);
    while (PyDict_Next(d, &pos, &key, &value)) {
        const char * k = PyString_AS_STRING(key);

        level[depth] = cps_name_to_attr(k);
        if (level[depth]==((cps_api_attr_id_t)-1)) {
            level[depth] = (cps_api_attr_id_t)atoi(k);
        }

        if (PyDict_Check(value)) {
            std::vector<cps_api_attr_id_t> v = level;
            add_to_object(v,obj,value,start_level);
            continue;
        }

        if (PyByteArray_Check(value)) {
            cps_api_object_e_add(obj,&level[start_level],level.size()-start_level,cps_api_object_ATTR_T_BIN,
                    PyByteArray_AsString(value), PyByteArray_Size(value));
            continue;
        }
        if (PyString_Check(value)) {
            cps_api_object_e_add(obj,&level[start_level],level.size()-start_level,cps_api_object_ATTR_T_BIN,
                    PyString_AS_STRING(value), PyString_GET_SIZE(value));
            continue;
        }
    }
}

static cps_api_object_t dict_to_cps_obj(const char *path, PyObject *dict) {

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

static cps_api_object_t dict_to_cps_obj(PyObject *dict) {
    if (!PyDict_Check(dict)) return NULL;
    PyObject * key = PyDict_GetItemString(dict,"key");
    if (key==NULL || !PyString_Check(key)) return NULL;
    PyObject * d = PyDict_GetItemString(dict,"data");
    if (d==NULL || !PyDict_Check(d)) return NULL;
    return dict_to_cps_obj(PyString_AsString(key),d);
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
    const char * path=NULL, *name, *desc,*type,*_id;
    PyObject *has_emb;
    if (! PyArg_ParseTuple( args, "ssssO!s", &_id,&path,&name,&desc,&PyBool_Type,&has_emb, &type )) return NULL;

    long long id = strtoll(_id,NULL,0);

    std::vector<cps_api_attr_id_t> ids;
    cps_class_ids_from_string(ids,path);

    cps_class_map_node_details details;
    details.desc = desc;
    details.name = name;
    details.embedded = has_emb != Py_False;
    details.type = cps_api_object_ATTR_T_BIN;
    if (cps_class_map_init((cps_api_attr_id_t)id,&ids[0],ids.size(),&details)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyObject * py_cps_info(PyObject *self, PyObject *args) {
    const char * path=NULL;
    if (! PyArg_ParseTuple( args, "s", &path)) return NULL;

    PyObject * d = PyDict_New();
    if (d==NULL) return NULL;

    std::vector<cps_api_attr_id_t> v;
    cps_class_ids_from_string(v,path);

    cps_class_node_detail_list_t lst;
    cps_class_map_level(&v[0],v.size(),lst);

    size_t ix = 0;
    size_t mx = lst.size();
    for ( ; ix < mx ; ++ix ) {
        char buff[40]; //just enough for the attribute id
        snprintf(buff,sizeof(buff),"%lld",(long long)lst[ix].id);
        SetItemToDict(d,buff,
                PyString_FromString(lst[ix].full_path.c_str()));
    }
    return d;
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
    cps_api_attr_id_t at = cps_name_to_attr(path);
    cps_api_key_t k;
    if (!cps_api_key_from_attr_with_qual(&k,at,i->second)) {
        return PyString_FromString("");
    }
    char buff[CPS_API_KEY_STR_MAX];

    return PyString_FromString(cps_api_key_print(&k,buff,sizeof(buff)-1));
}

static PyObject * py_cps_event_connect(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t handle=NULL;
    if (cps_api_event_client_connect(&handle)!=cps_api_ret_code_OK) {
        return PyByteArray_FromStringAndSize(NULL,0);
    }
    return PyByteArray_FromStringAndSize((const char *)&handle,sizeof(handle));
}

static PyObject * py_cps_event_reg(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    const char *path;
    if (! PyArg_ParseTuple( args, "O!s",  &PyByteArray_Type, &o,&path)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return NULL;
    }

    cps_api_event_reg_t reg;
    cps_api_key_t key;

    cps_api_key_from_string(&key,path);
    reg.number_of_objects =1;
    reg.objects = &key;
    reg.priority = 0;

    if (cps_api_event_client_register(*handle,&reg)==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * py_cps_event_close(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return NULL;
    }
    cps_api_event_client_disconnect(*handle);
    Py_RETURN_TRUE;
}

static PyObject * py_cps_event_wait(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        return NULL;
    }

    cps_api_object_t obj=cps_api_object_create();
    cps_api_object_guard og(obj);
    cps_api_object_reserve(obj,MAX_EVENT_BUFF);

    if (cps_api_wait_for_event(*handle,obj)==cps_api_ret_code_OK) {
        return (cps_obj_to_dict(obj));
    }
    return PyDict_New();
}

static PyObject * py_cps_event_send(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *dict,*h;
    const char *path;
    if (! PyArg_ParseTuple( args, "O!sO!",  &PyByteArray_Type, &h,&path,&PyDict_Type, &dict)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(h);
    if (PyByteArray_Size(h)!=sizeof(*handle)) {
        return NULL;
    }

    cps_api_object_t obj = dict_to_cps_obj(path,dict);
    cps_api_object_guard og(obj);
    if (!og.valid()) {
        Py_RETURN_FALSE;
    }

    if (cps_api_event_publish(*handle,obj)==cps_api_ret_code_OK) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject * py_cps_get(PyObject *self, PyObject *args) {
    PyObject * param_list;

    cps_api_get_params_t gr;
    if (cps_api_get_request_init (&gr)==cps_api_ret_code_ERR) {
        Py_RETURN_FALSE;
    }

    cps_api_get_request_guard rg(&gr);

    PyObject *res_obj;

    if (! PyArg_ParseTuple( args, "O!O", &PyList_Type, &param_list,&res_obj)) {
        Py_RETURN_FALSE;
    }

    PyObject * lst = NULL;

    if (PyDict_Check(res_obj)) {
        PyObject *l = PyList_New(0);
        if (l==NULL) {
            Py_RETURN_FALSE;

        }
        if (PyDict_GetItemString(res_obj,"list")!=NULL)
            PyDict_DelItemString(res_obj,"list");

        if (!SetItemToDict(res_obj,"list",l)) {
            Py_RETURN_FALSE;
        }
        lst = l;
    }
    if (PyList_Check(res_obj)) {
        lst = res_obj;
    }
    if (lst==NULL) {
        Py_RETURN_FALSE;
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
                    Py_RETURN_FALSE;
                }
                if (!cps_api_key_from_string(cps_api_object_key(o),PyString_AsString(strObj))) {
                    Py_RETURN_FALSE;
                }
            }
            if (PyDict_Check(strObj)) {
                cps_api_object_t o = dict_to_cps_obj(strObj);
                if (o==NULL) {
                    Py_RETURN_FALSE;
                }
                if (!cps_api_object_list_append(gr.filters,o)) {
                    cps_api_object_delete(o);
                    Py_RETURN_FALSE;
                }
            }
        }
    }
    gr.keys = NULL;
    gr.key_count = 0;
    if (cps_api_get(&gr)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(gr.list);
    for ( ; ix < mx ; ++ix) {
        cps_api_object_t obj = cps_api_object_list_get(gr.list,ix);
        PyObject *d = cps_obj_to_dict(obj);
        PyRef r(d);
        if (d==NULL) {
            Py_RETURN_FALSE;
        }
        if (PyList_Append(lst,d)) {
            Py_RETURN_FALSE;
        }
        r.release();
    }

    Py_RETURN_TRUE;
}

static bool py_add_object_to_trans( cps_api_transaction_params_t *tr, PyObject *dict) {
    PyObject *_req = PyDict_GetItemString(dict,"change");
    PyObject *_op = PyDict_GetItemString(dict,"operation");
    if (_op==NULL || _req==NULL) {
        return false;
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

static PyObject * py_cps_trans(PyObject *self, PyObject *args) {
    PyObject *list;
    if (! PyArg_ParseTuple( args, "O!",  &PyList_Type, &list)) return NULL;

    cps_api_transaction_params_t tr;
    if (cps_api_transaction_init(&tr)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    cps_api_transaction_guard pg(&tr);

    size_t ix = 0;
    size_t mx = PyList_Size(list);
    for ( ; ix < mx ; ++ix ) {
        PyObject *dict = PyList_GetItem(list,ix);
        if (!py_add_object_to_trans(&tr,dict)) {
            Py_RETURN_FALSE;
        }
    }

    if (cps_api_commit(&tr)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }

    ix = 0;
    mx = PyList_Size(list);
    for ( ; ix < mx ; ++ix ) {
        PyObject *dict = PyList_GetItem(list,ix);
        if (dict==NULL || !PyDict_Check(dict)) {
            Py_RETURN_FALSE;
        }
        PyObject *_req = PyDict_GetItemString(dict,"change");
        if (_req==NULL || !PyDict_Check(_req)) {
            Py_RETURN_FALSE;
        }

        cps_api_object_t obj = cps_api_object_list_get(tr.change_list,ix);
        if (obj==NULL) {
            Py_RETURN_FALSE;
        }
        PyDict_SetItemString(_req,"key",
                PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));

        if (PyDict_GetItemString(dict,"change")!=NULL)
            PyDict_DelItemString(dict,"change");

        SetItemToDict(dict,"change",cps_obj_to_dict(obj));
    }

    cps_api_transaction_close(&tr);

    Py_RETURN_TRUE;
}

static PyObject * py_cps_obj_init(PyObject *self, PyObject *args) {
    cps_api_operation_handle_t handle;
    if (cps_api_operation_subsystem_init(&handle,1)!=cps_api_ret_code_OK) {
        return PyByteArray_FromStringAndSize(NULL,0);
    }
    return PyByteArray_FromStringAndSize((const char *)&handle,sizeof(handle));
}

static cps_api_return_code_t _read_function (void * context, cps_api_get_params_t * param,
        size_t key_ix) {
    py_callbacks_t *cb = (py_callbacks_t*)context;

    GILLock gil;

    PyObject *p = PyDict_New();
    PyRef dict(p);

    SetItemToDict(p,"filter",cps_obj_to_dict(cps_api_object_list_get(
            param->filters,key_ix)));
    SetItemToDict(p,"list",PyList_New(0));

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

    SetItemToDict(p,"change",cps_obj_to_dict(obj));
    SetItemToDict(p,"previous",cps_obj_to_dict(prev));

    static const std::map<int,std::string> trans = {
            {cps_api_oper_DELETE,"delete" },
            {cps_api_oper_CREATE,"create"},
            {cps_api_oper_SET, "set"},
            {cps_api_oper_ACTION,"rpc"}
    };

    SetItemToDict(p,"operation",PyString_FromString(trans.at(op).c_str()));

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


static PyObject * py_cps_obj_reg(PyObject *self, PyObject *args) {
    cps_api_operation_handle_t *handle=NULL;
    PyObject *h,*o;
    const char *path;
    if (! PyArg_ParseTuple( args, "O!sO!",  &PyByteArray_Type, &h,&path,&PyDict_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(h);
    if (PyByteArray_Size(h)!=sizeof(*handle)) {
        Py_RETURN_FALSE;
    }
    std::unique_ptr<py_callbacks_t> p (new py_callbacks_t);
    if (p.get()==NULL) {
        Py_RETURN_FALSE;
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
        return NULL;
    }

    if (cps_api_register(&f)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }
    p.release();
    Py_INCREF(o);

    Py_RETURN_TRUE;
}

static PyObject * py_cps_obj_close(PyObject *self, PyObject *args) {
    cps_api_event_service_handle_t *handle=NULL;
    PyObject *o;
    if (! PyArg_ParseTuple( args, "O!s",  &PyByteArray_Type, &o)) return NULL;

    handle = (cps_api_event_service_handle_t*)PyByteArray_AsString(o);
    if (PyByteArray_Size(o)!=sizeof(*handle)) {
        Py_RETURN_FALSE;
    }

    Py_RETURN_TRUE;
}


PyDoc_STRVAR(cps_get__doc__, "Perform a CPS get using an list of keys. Currently the keys should be in x.y.z format.");
PyDoc_STRVAR(cps_trans__doc__, "Perform a CPS transaction operation the dictionary provided. "
        " The dictionary needs to contain a dictionary of settings.");
PyDoc_STRVAR(cps_doc__, "A python interface to the CPS API");

PyDoc_STRVAR(cps_cps_generic_doc__, "A CPS mapping function.");

/* A list of all the methods defined by this module. */
/* "METH_VARGS" tells Python how to call the handler */
static PyMethodDef cps_methods[] = {
    {"get",  py_cps_get, METH_VARARGS, cps_get__doc__},
    {"transaction",  py_cps_trans, METH_VARARGS, cps_trans__doc__},

    {"init",  py_cps_map_init, METH_VARARGS, cps_cps_generic_doc__},
    {"convarray",  py_cps_byte_array_to_obj, METH_VARARGS, cps_cps_generic_doc__},
    {"arraykey",  py_cps_byte_array_key, METH_VARARGS, cps_cps_generic_doc__},
    {"convdict",  py_cps_obj_to_array, METH_VARARGS, cps_cps_generic_doc__},
    {"info",  py_cps_info, METH_VARARGS, cps_cps_generic_doc__},
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

    {NULL, NULL}      /* sentinel */
};


PyMODINIT_FUNC initcps(void) {
    /* There have been several InitModule functions over time */
    PyObject *m = Py_InitModule3("cps", cps_methods, cps_doc__);
    if (m==NULL) return;

    if (! PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
    }
    cps_api_event_service_init();

}
