

#include "cps_api_operation.h"
#include "cps_api_key.h"
#include "cps_class_map.h"
#include "python2.7/Python.h"

#include <stdio.h>
#include <vector>
#include <memory>

PyDoc_STRVAR(cps_get__doc__, "Perform a CPS get using the keys specified.");
PyDoc_STRVAR(cps_doc__, "A python interface to the CPS API");

PyDoc_STRVAR(cps_cps_map_init_doc__, "Initialize the string dictionary for the CPS.");


class cps_api_key_wrapper {
protected:
    cps_api_key_t key;
public:
    cps_api_key_t *get() { return &key; }
};


static PyObject * py_cps_get(PyObject *self, PyObject *args) {
    PyObject * param_list;

    cps_api_get_params_t gr;
    if (cps_api_get_request_init (&gr)==cps_api_ret_code_ERR) return NULL;

    cps_api_get_request_guard rg(&gr);


    if (! PyArg_ParseTuple( args, "O!", &PyList_Type, &param_list)) return NULL;

    Py_ssize_t str_keys = PyList_Size(param_list);

    if (str_keys<0) return NULL;

    std::unique_ptr<cps_api_key_t[]> keys;

    try {
        keys = std::unique_ptr<cps_api_key_t[]>(new cps_api_key_t[str_keys]);
    } catch (...) {
        return NULL;
    }

    {
        Py_ssize_t ix = 0;
        for ( ;ix < str_keys ; ++ix ) {
            PyObject *strObj = PyList_GetItem(param_list, ix);
            if (!cps_api_key_from_string(&(keys[ix]),PyString_AS_STRING(strObj))) {
                return NULL;
            }
        }
    }

    gr.keys = &(keys[0]);
    gr.key_count = str_keys;
    if (cps_api_get(&gr)!=cps_api_ret_code_OK) {
        printf("Exception... bad return code\n");
        return NULL;
    }

    PyObject * dict_obj = PyDict_New();

    size_t ix = 0;
    size_t mx = cps_api_object_list_size(gr.list);
    for ( ; ix < mx ; ++ix) {
        cps_api_object_t obj = cps_api_object_list_get(gr.list,ix);
        cps_api_key_t *key = cps_api_object_key(obj);

        char buff[1024];

        PyObject * k = PyString_FromString(cps_api_key_print(key,buff,sizeof(buff)));
        if(k==NULL) {
            printf("Allocation error... python environment corrupted.");
            break;
        }
        PyDict_SetItem(dict_obj,k,PyByteArray_FromStringAndSize((const char *)cps_api_object_array(obj),
                cps_api_object_to_array_len(obj)));
    }

    return dict_obj;
}

static PyObject * py_cps_map_init(PyObject *self, PyObject *args) {
    const char * path=NULL,*prefix=NULL;
    if (! PyArg_ParseTuple( args, "ss", &path, &prefix)) return NULL;

    cps_class_objs_load(path,prefix);

    Py_RETURN_TRUE;
}

static void py_obj_dump_level(PyObject * d, std::vector<cps_api_attr_id_t> &parent, cps_api_object_it_t *it) {

    std::vector<cps_api_attr_id_t> cur = parent;
    size_t level = cur.size();
    cur.push_back((cps_api_attr_id_t)0);
    cps_api_key_t key;
    memset(&key,0,sizeof(key));

    while (cps_api_object_it_valid(it)) {
        do {
            const size_t ID_BUFF_LEN=100;
            char buff[ID_BUFF_LEN];

            cur[level] = cps_api_object_attr_id(it->attr);
            sprintf(buff,"%d",(int)cur[level]);

            const char * name = cps_class_attr_name(&key,&cur[0],cur.size());

            if (!cps_class_attr_is_valid(&key,&cur[0],cur.size()) || name==NULL) {
                PyDict_SetItem(d,PyString_FromString(buff),
                        PyByteArray_FromStringAndSize((const char *)cps_api_object_attr_data_bin(it->attr),
                        cps_api_object_attr_len(it->attr)));
                break;
            }

            bool emb = cps_class_attr_is_embedded(&key,&cur[0],cur.size());
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

static PyObject * py_cps_byte_array_to_obj(PyObject *self, PyObject *args) {
    const char * path=NULL;
    PyObject *array;
    if (! PyArg_ParseTuple( args, "sO!", &path, &PyByteArray_Type, &array)) return NULL;

    cps_api_key_t key;
    cps_api_key_from_string(&key,path);

    std::vector<cps_api_attr_id_t> lst;

    size_t ix = 1;
    size_t mx = cps_api_key_get_len(&key);
    if (mx > 3) mx = 3;
    for ( ; ix < mx ; ++ix ) {
        lst.push_back((cps_api_attr_id_t)cps_api_key_element_at(&key,ix));
    }
    cps_api_key_set_len(&key,0);

    PyObject * d = PyDict_New();
    if (d==NULL) return NULL;

    cps_api_object_t obj = cps_api_object_create();
    cps_api_object_guard og(obj);

    if (!cps_api_array_to_object(PyByteArray_AsString(array), PyByteArray_Size(array),obj)) {
        return d;
    }
    if (!cps_api_object_received(obj,PyByteArray_Size(array))) {
        return d;
    }

    cps_api_object_it_t it;
    cps_api_object_it_begin(obj, &it);
    py_obj_dump_level(d,lst,&it);
    return d;
}

static PyObject * py_cps_obj_to_array(PyObject *self, PyObject *args) {
    PyObject *d;
    if (! PyArg_ParseTuple( args, "O!", PyDict_Type, &d)) return NULL;

    return PyByteArray_FromStringAndSize(NULL,0);
}

/* A list of all the methods defined by this module. */
/* "METH_VARGS" tells Python how to call the handler */
static PyMethodDef cps_methods[] = {
    {"get",  py_cps_get, METH_VARARGS, cps_get__doc__},
    {"init",  py_cps_map_init, METH_VARARGS, cps_cps_map_init_doc__},
    {"array_to_dict",  py_cps_byte_array_to_obj, METH_VARARGS, cps_cps_map_init_doc__},
    {"dict_to_array",  py_cps_obj_to_array, METH_VARARGS, cps_cps_map_init_doc__},
    {NULL, NULL}      /* sentinel */
};


PyMODINIT_FUNC initcps(void) {
    /* There have been several InitModule functions over time */
    PyObject *m = Py_InitModule3("cps", cps_methods, cps_doc__);
    if (m==NULL) return;


}
