

#include "cps_api_operation.h"
#include "cps_api_key.h"
#include "cps_class_map.h"
#include "cps_class_map_query.h"
#include "cps_api_events.h"



#include "python2.7/Python.h"
#include <stdio.h>
#include <vector>
#include <memory>
#include <map>


const static size_t MAX_EVENT_BUFF=20000;
const static unsigned int CUSTOM_KEY_POS = 2;

class PyRef {
    PyObject *_o;
public:
    PyRef(PyObject *o,bool inc=false) {
        _o = o;

        if (_o!=NULL && inc) Py_INCREF(_o);
    }
    bool valid() { return _o != NULL; }
    void release() { _o = NULL;}
    PyObject * get() { return _o; }
    ~PyRef() {
        if (_o!=NULL) Py_DECREF(_o);
    }
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

    PyObject *execute(const char * method, PyObject *param) {
        if (!contains(method)) return NULL;

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

            const char * name = cps_attr_id_to_name(cur[level]);

            if (!cps_class_attr_is_valid(&cur[0],cur.size()) || name==NULL) {
                PyDict_SetItem(d,PyString_FromString(buff),
                        PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                        cps_api_object_attr_len(it->attr)));
                break;
            }

            bool emb = cps_class_attr_is_embedded(&cur[0],cur.size());
            if (emb) {
                PyObject * subd = PyDict_New();
                if (subd==NULL) break;
                cps_api_object_it_t contained_it = *it;
                cps_api_object_it_inside(&contained_it);
                py_obj_dump_level(subd,cur,&contained_it);
                PyDict_SetItem(d,PyString_FromString(name),subd);
                break;
            }
            PyDict_SetItem(d,PyString_FromString(name),
                            PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                                    cps_api_object_attr_len(it->attr)));
        } while (0);
        cps_api_object_it_next(it);
    }
}

static PyObject * cps_obj_to_dict(cps_api_object_t obj) {
    cps_api_key_t *key = cps_api_object_key(obj);
    std::vector<cps_api_attr_id_t> lst;

    //get the cat and sub cat
    cps_class_ids_from_key(lst,key,CPS_OBJ_KEY_INST_POS+1,2);

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj, &it);
    PyObject *d = PyDict_New();
    py_obj_dump_level(d,lst,&it);
    return d;
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

static PyObject * py_cps_map_init(PyObject *self, PyObject *args) {
    const char * path=NULL,*prefix=NULL;
    if (! PyArg_ParseTuple( args, "ss", &path, &prefix)) return NULL;

    cps_class_objs_load(path,prefix);

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
    const char * path=NULL, *name, *desc,*type;
    PyObject *has_emb;
    if (! PyArg_ParseTuple( args, "sssO!s", &path,&name,&desc,&PyBool_Type,&has_emb, &type )) return NULL;

    std::vector<cps_api_attr_id_t> ids;
    cps_class_ids_from_string(ids,path);

    cps_class_map_node_details details;
    details.desc = desc;
    details.name = name;
    details.embedded = has_emb != Py_False;
    details.type = cps_api_object_ATTR_T_BIN;
    if (cps_class_map_init(&ids[0],ids.size(),&details)!=cps_api_ret_code_OK) {
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
        PyDict_SetItem(d,
                PyString_FromString(cps_class_ids_to_string(lst[ix].ids).c_str()),
                PyString_FromString(lst[ix].full_path.c_str()));
    }
    return d;
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
        PyRef r(cps_obj_to_dict(obj));
        PyObject *ret = PyDict_New();
        if (ret==NULL) return ret;
        PyDict_SetItemString(ret,"data",r.get());
        PyDict_SetItemString(ret,"key",PyString_FromString(cps_key_to_string(cps_api_object_key(obj)).c_str()));
        r.release();
        return ret;
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

    PyObject *dict_obj;

    if (! PyArg_ParseTuple( args, "O!O!", &PyList_Type, &param_list, &PyDict_Type,&dict_obj)) {
        Py_RETURN_FALSE;
    }

    Py_ssize_t str_keys = PyList_Size(param_list);

    if (str_keys<0) return NULL;

    std::unique_ptr<cps_api_key_t[]> keys;

    try {
        keys = std::unique_ptr<cps_api_key_t[]>(new cps_api_key_t[str_keys]);
    } catch (...) {
        Py_RETURN_FALSE;
    }

    {
        Py_ssize_t ix = 0;
        for ( ;ix < str_keys ; ++ix ) {
            PyObject *strObj = PyList_GetItem(param_list, ix);
            if (!cps_api_key_from_string(&(keys[ix]),PyString_AS_STRING(strObj))) {
                Py_RETURN_FALSE;
            }
        }
    }

    gr.keys = &(keys[0]);
    gr.key_count = str_keys;
    if (cps_api_get(&gr)!=cps_api_ret_code_OK) {
        Py_RETURN_FALSE;
    }

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(gr.list);
    for ( ; ix < mx ; ++ix) {
        cps_api_object_t obj = cps_api_object_list_get(gr.list,ix);
        cps_api_key_t *key = cps_api_object_key(obj);

        PyDict_SetItemString(dict_obj,cps_key_to_string(key).c_str(),cps_obj_to_dict(obj));
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

    PyObject *key = PyDict_GetItemString(_req,"key");
    PyObject *d = PyDict_GetItemString(_req,"data");
    if (!PyString_Check(key) || !PyDict_Check(d)) {
        return false;
    }

    cps_api_object_t obj = dict_to_cps_obj(PyString_AsString(key),d);
    cps_api_object_guard og(obj);

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

        PyDict_SetItemString(_req,"data",cps_obj_to_dict(obj));
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
    PyObject *lst = PyList_New(1);  //create with one element

    PyDict_SetItemString(p,"keys",lst);
    PyDict_SetItemString(p,"result",PyDict_New());

    //set the only element to the correct key
    PyList_SetItem(lst,0,PyString_FromString(cps_key_to_string(&param->keys[key_ix]).c_str()));

    PyObject * res = cb->execute("get",p);
    if (!PyBool_Check(res) || (Py_False == (res))) {
        return cps_api_ret_code_ERR;
    }
    PyRef ret(res);

    PyObject *d = PyDict_GetItemString(p,"result");
    if (d==NULL) {
        return cps_api_ret_code_ERR;
    }
    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(d, &pos, &key, &value)) {
        const char * k = PyString_AsString(key);
        if (!PyDict_Check(value)) return cps_api_ret_code_ERR;
        cps_api_object_guard og(dict_to_cps_obj(k,value));
        if (!og.valid()) return cps_api_ret_code_ERR;
        if (!cps_api_object_list_append(param->list,og.get())) {
            return cps_api_ret_code_ERR;
        }
        og.release();
    }
    return cps_api_ret_code_OK;
}

static cps_api_return_code_t _write_function(void * context, cps_api_transaction_params_t * param,size_t ix) {

    GILLock gil;

    cps_api_object_t obj = cps_api_object_list_get(param->change_list,ix);
    if (obj==NULL) return cps_api_ret_code_ERR;

    cps_api_object_t prev = cps_api_object_create();
    if (prev==NULL) return cps_api_ret_code_ERR;
    if (!cps_api_object_list_append(param->prev,prev)) {
        cps_api_object_delete(prev);
        return cps_api_ret_code_ERR;
    }

    cps_api_operation_types_t op = cps_api_object_type_operation(cps_api_object_key(obj));

    PyObject *p = PyDict_New();
    PyRef dict(p);

    PyObject *req = PyDict_New();
    PyObject * o = cps_obj_to_dict(obj);
    PyDict_SetItemString(req,cps_key_to_string(cps_api_object_key(obj)).c_str(),o);

    PyDict_SetItemString(p,"change",req);
    PyDict_SetItemString(p,"previous",PyDict_New());

    static std::map<int,std::string> trans = {
            {cps_api_oper_DELETE,"delete" },
            {cps_api_oper_CREATE,"create"},
            { cps_api_oper_SET, "set"},
            {cps_api_oper_ACTION,"rpc"}
    };

    PyDict_SetItemString(p,"operation",PyString_FromString(trans[op].c_str()));

    py_callbacks_t *cb = (py_callbacks_t*)context;
    PyObject *res = cb->execute("transaction",p);

    if (!PyBool_Check(res) || (Py_False==(res))) {
        return cps_api_ret_code_ERR;
    }
    PyRef ret(res);

    PyObject *key, *value;
    Py_ssize_t pos = 0;

    while (PyDict_Next(req, &pos, &key, &value)) {
        const char * k = PyString_AsString(key);

        if (!PyDict_Check(value)) return cps_api_ret_code_ERR;

        cps_api_object_guard og(dict_to_cps_obj(k,value));
        if (!og.valid()) return cps_api_ret_code_ERR;
        cps_api_object_clone(obj,og.get());
        break;
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
