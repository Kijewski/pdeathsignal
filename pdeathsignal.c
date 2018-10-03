/*
 * Copyright 2018  René Kijewski
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <Python.h>

#include <stdint.h>
#include <sys/prctl.h>

static PyObject *function_get(PyObject *self, PyObject *no_args) {
    (void) self;
    (void) no_args;

    int signal = -1;
    int outcome = prctl(
        PR_GET_PDEATHSIG,
        (unsigned long) (uintptr_t) &signal,
        0, 0, 0
    );

    if (outcome == 0) {
#     if PY_MAJOR_VERSION >= 3
        return PyLong_FromLong(signal);
#     else
        return PyInt_FromLong(signal);
#     endif
    } else {
        return PyErr_SetFromErrno(PyExc_OSError);
    }
}

static PyObject *function_set(PyObject *self, PyObject *args, PyObject *kwargs) {
    (void) self;

    static const char *keywords[] = { "signal", NULL };

    int signal = -1;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", (char**) keywords, &signal)) {
        return NULL;
    }

    int outcome = prctl(
        PR_SET_PDEATHSIG,
        signal,
        0, 0, 0
    );

    if (outcome == 0) {
        Py_RETURN_NONE;
    } else {
        return PyErr_SetFromErrno(PyExc_OSError);
    }
}

static PyMethodDef functions_def[] = {
    {
        "getpdeathsignal",
        (PyCFunction) function_get,
        METH_NOARGS,
#     if PY_VERSION_HEX >= 0x03040000
        "getpdeathsignal()\n"
        "--\n"
        "\n"
#     endif
        "Return the current value of the parent process death signal.\n"
        "\n"
        "Returns\n"
        "=======\n"
        "int\n"
        "    Current parent process death signal. 0 if none."
    },
    {
        "setpdeathsignal",
        (PyCFunction) function_set,
        METH_VARARGS | METH_KEYWORDS,
#     if PY_VERSION_HEX >= 0x03040000
        "setpdeathsignal(signal)\n"
        "--\n"
        "\n"
#     endif
        "Set the parent process death signal of the calling process.\n"
        "\n"
        "Arguments\n"
        "=========\n"
        "signal : int\n"
        "    New parent process death signal. 0 if none."
    },
    { NULL, NULL, 0, NULL }
};

static const char module_name[] = "pdeathsignal";
static const char module_doc[] = "Get and set the parent process death signal.";

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    module_name,
    module_doc,
    0,
    functions_def,
};
#endif

PyMODINIT_FUNC
#if PY_MAJOR_VERSION >= 3
PyInit_pdeathsignal(void)
#else
initpdeathsignal(void)
#endif
{
#if PY_VERSION_HEX >= 0x03050000
    return PyModuleDef_Init(&module_def);
#elif PY_MAJOR_VERSION >= 3
    return PyModule_Create(&module_def);
#else
    Py_InitModule3(module_name, functions_def, module_doc);
#endif
}
