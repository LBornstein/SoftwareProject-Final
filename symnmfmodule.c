#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "symnmf.h" 


static double **PyList_to_DoublePtrPtr(PyObject *list, int rows, int cols) {
    /* Converts Python List of Lists to C double** */
    int i, j;
    PyObject *row, *item;
    double **mat = allocate_matrix(rows, cols);
    
    for (i = 0; i < rows; i++) {
        row = PyList_GetItem(list, i);
        for (j = 0; j < cols; j++) {
            item = PyList_GetItem(row, j);
            mat[i][j] = PyFloat_AsDouble(item);
        }
    }
    return mat;
}


static PyObject *DoublePtrPtr_to_PyList(double **mat, int rows, int cols) {
    /* Converts C double** to python list of lists */
    int i, j;
    PyObject *list, *row;
    
    list = PyList_New(rows);
    for (i = 0; i < rows; i++) {
        row = PyList_New(cols);
        for (j = 0; j < cols; j++) {
            PyList_SetItem(row, j, PyFloat_FromDouble(mat[i][j]));
        }
        PyList_SetItem(list, i, row);
    }
    return list;
}


static PyObject* wrap_sym(PyObject *self, PyObject *args) {
    /* Wrapper for sym() */
    PyObject *data_points, *py_result;
    int n, d;
    double **c_data_points, **c_result;

    if (!PyArg_ParseTuple(args, "Oii", &data_points, &n, &d)) {
        return NULL;
    }
    
    c_data_points = PyList_to_DoublePtrPtr(data_points, n, d);
    c_result = sym(c_data_points, n, d);
    py_result = DoublePtrPtr_to_PyList(c_result, n, n);

    free_matrix(c_data_points);
    free_matrix(c_result);
    return py_result;
}


static PyObject* wrap_ddg(PyObject *self, PyObject *args) {
    /* Wrapper for ddg() */
    PyObject *data_points, *py_result;
    int n, d;
    double **c_data_points, **c_result;

    if (!PyArg_ParseTuple(args, "Oii", &data_points, &n, &d)) {
        return NULL;
    }
    
    c_data_points = PyList_to_DoublePtrPtr(data_points, n, d);
    c_result = ddg(c_data_points, n, d);
    py_result = DoublePtrPtr_to_PyList(c_result, n, n);

    free_matrix(c_data_points);
    free_matrix(c_result);
    return py_result;
}

/* --- Wrapper for norm() --- */
static PyObject* wrap_norm(PyObject *self, PyObject *args) {
    PyObject *data_points, *py_result;
    int n, d;
    double **c_data_points, **c_result;

    if (!PyArg_ParseTuple(args, "Oii", &data_points, &n, &d)) {
        return NULL;
    }
    
    c_data_points = PyList_to_DoublePtrPtr(data_points, n, d);
    c_result = norm(c_data_points, n, d);
    py_result = DoublePtrPtr_to_PyList(c_result, n, n);

    free_matrix(c_data_points);
    free_matrix(c_result);
    return py_result;
}


static PyObject* wrap_symnmf(PyObject *self, PyObject *args) {
    /* Wrapper for symnmf() */
    PyObject *H_py, *W_py, *py_result;
    int n, k, max_iter;
    double eps;
    double **H_c, **W_c, **result_c;

    if (!PyArg_ParseTuple(args, "OOiidi", &H_py, &W_py, &n, &k, &eps, &max_iter)) {
        return NULL;
    }
    
    H_c = PyList_to_DoublePtrPtr(H_py, n, k);
    W_c = PyList_to_DoublePtrPtr(W_py, n, n);
    
    result_c = symnmf(H_c, W_c, n, k, eps, max_iter);
    py_result = DoublePtrPtr_to_PyList(result_c, n, k);
    
    /* note there is no need to free H here since we free it in the C code*/
    free_matrix(W_c);
    free_matrix(result_c);
    
    return py_result;
}


static PyMethodDef symnmf_methods[] = {
    /* Method Definitions Table */
    {"sym", (PyCFunction)wrap_sym, METH_VARARGS, "Calculate Similarity Matrix"},
    {"ddg", (PyCFunction)wrap_ddg, METH_VARARGS, "Calculate Diagonal Degree Matrix"},
    {"norm", (PyCFunction)wrap_norm, METH_VARARGS, "Calculate Normalized Similarity Matrix"},
    {"symnmf", (PyCFunction)wrap_symnmf, METH_VARARGS, "Execute symNMF algorithm"},
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef symnmf_module = {
    /* Module Definition */
    PyModuleDef_HEAD_INIT,
    "symnmf",
    "Python wrapper for C symNMF engine",
    -1,
    symnmf_methods
};


PyMODINIT_FUNC PyInit_symnmf(void) {
    /* Module Initialization */
    return PyModule_Create(&symnmf_module);
}