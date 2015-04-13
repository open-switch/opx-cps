
#include "python2.7/Python.h"


#include "cps_api_operation.h"
#include "cps_api_key.h"

#include <stdio.h>
#include <vector>
#include <memory>

PyDoc_STRVAR(cps_get__doc__, "Perform a CPS get using the keys specified.");
PyDoc_STRVAR(cps_doc__, "A python interface to the CPS API");

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
    
    printf("Initializing\n");fflush(stdout);

    cps_api_get_request_guard rg(&gr);


    if (! PyArg_ParseTuple( args, "O!", &PyList_Type, &param_list)) return NULL;
    
    printf("Scanning\n");fflush(stdout);
    Py_ssize_t str_keys = PyList_Size(param_list);

    if (str_keys<0) return NULL;
    printf("found %d keys \n",(int)str_keys);fflush(stdout);

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
        return NULL;
    }

    PyObject * dict_obj = PyDict_New();

    printf("Number of lines returned is %d\n",(int)cps_api_object_list_size(gr.list));
    fflush(stdout);

    return dict_obj;
}

/* A list of all the methods defined by this module. */
/* "METH_VARGS" tells Python how to call the handler */
static PyMethodDef cps_methods[] = {
    {"cps_get",  py_cps_get, METH_VARARGS, cps_get__doc__},
    {NULL, NULL}      /* sentinel */
};

PyMODINIT_FUNC initcps(void) {
    /* There have been several InitModule functions over time */
    PyObject *m = Py_InitModule3("cps", cps_methods, cps_doc__);
    if (m==NULL) return;


}
