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

#include <errno.h>
#include <signal.h>
#include <sched.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <unistd.h>


PyDoc_STRVAR(module_doc, "Get and set the parent process death signal.");
static const char module_name[] = "pdeathsignal";


#if PY_VERSION_HEX >= 0x03040000
#   define DocVar(name, signature, desc) PyDoc_STRVAR(name, signature "\n--\n\n" desc)
#else
#   define DocVar(name, signature, desc) PyDoc_STRVAR(name, desc)
#endif


static const char *setpdeathsignal_keywords[] = { "signal", NULL };
DocVar(
    getpdeathsignal_doc,
    "getpdeathsignal()",
    "Return the current value of the parent process death signal.\n"
    "\n"
    "Returns\n"
    "=======\n"
    "int\n"
    "    Current parent process death signal. 0 if none."
);
static PyObject *getpdeathsignal_impl(PyObject *self, PyObject *no_args);


DocVar(
    setpdeathsignal_doc,
    "setpdeathsignal(signal=0)",
    "Set the parent process death signal of the calling process.\n"
    "\n"
    "Arguments\n"
    "=========\n"
    "signal : int\n"
    "    New parent process death signal. 0 if none."
);
static PyObject *setpdeathsignal_impl(PyObject *self, PyObject *args, PyObject *kwargs);


static const char *cloneandexecve_keywords[] = {
    "path", "args", "env", "signal", "sibling", "search_path",
    "setsid", "doublefork", "sigign",
    NULL
};
DocVar(
    cloneandexecve_doc,
    "cloneandexecve(path, args=None, env=None, "
#if PY_VERSION_HEX >= 0x03030000
    "*, "
#endif
    "signal=0, sibling=False, search_path=False, "
    "setsid=False, doublefork=False, sigign=[])",
    ""
);
static PyObject *cloneandexecve_impl(PyObject *self, PyObject *args, PyObject *kwargs);


static PyMethodDef functions_def[] = {
    { "getpdeathsignal", (PyCFunction) getpdeathsignal_impl, METH_NOARGS, getpdeathsignal_doc },
    { "setpdeathsignal", (PyCFunction) setpdeathsignal_impl, METH_VARARGS | METH_KEYWORDS, setpdeathsignal_doc },
    { "cloneandexecve", (PyCFunction) cloneandexecve_impl, METH_VARARGS | METH_KEYWORDS, cloneandexecve_doc },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef module_def = {
    PyModuleDef_HEAD_INIT,
    module_name,
    module_doc,
    0,
    functions_def,
};
#endif

#if PY_MAJOR_VERSION >= 3
#   define pymalloc(N) PyMem_RawMalloc((N))
#   define pyfree(P) PyMem_RawFree((P))
#else
#   define pymalloc(N) PyMem_Malloc((N))
#   define pyfree(P) PyMem_Free((P))
#endif


static PyObject *int_or_errno(bool success, long value) {
    if (success) {
#     if PY_MAJOR_VERSION >= 3
        return PyLong_FromLong(value);
#     else
        return PyInt_FromLong(value);
#     endif
    } else {
        return PyErr_SetFromErrno(PyExc_OSError);
    }
}


static PyObject *as_bytes(PyObject *value) {
    if (PyBytes_Check(value)) {
        Py_INCREF(value);
        return value;
    } else if (PyUnicode_Check(value)) {
        return PyUnicode_AsEncodedString(value, "UTF-8", "strict");
    } else {
        return PyErr_Format(PyExc_TypeError, "Expected bytes or unicode, got %s.", value->ob_type->tp_name);
    }
}


static PyObject *object_as_list_of_bytes(PyObject *obj) {
    PyObject *list = PySequence_List(obj);
    if (!list) {
        return NULL;
    }

    Py_ssize_t length = PyList_GET_SIZE(list);
    for (Py_ssize_t index = 0; index < length; ++index) {
        PyObject *elem = PyList_GET_ITEM(list, index);
        PyObject *bytes = as_bytes(elem);
        if (!bytes) {
            return NULL;
        }
        PyList_SET_ITEM(list, index, bytes);
        Py_DECREF(elem);
    }

    return list;
}


static char **bytes_list_to_cstring_array(PyObject *list) {
    if (!list) {
        return NULL;
    }

    Py_ssize_t length = PyList_GET_SIZE(list);
    char **result = pymalloc(sizeof(char*) * (length + 1));
    if (!result) {
        PyErr_NoMemory();
        return NULL;
    }

    result[length] = NULL;
    for (Py_ssize_t index = 0; index < length; ++index) {
        PyObject *elem = PyList_GET_ITEM(list, index);
        result[index] = PyBytes_AS_STRING(elem);
    }

    return result;
}


static int path_converter(PyObject *input, PyObject **output) {
#if PY_VERSION_HEX >= 0x03010000
    return PyUnicode_FSConverter(input, output);
#else
    PyObject *result = as_bytes(input);
    if (result) {
        *output = result;
        return true;
    }
    return false;
#endif
}


static int bytes_list_converter(PyObject *obj, PyObject **list) {
    if (!obj || (obj == Py_None)) {
        *list = NULL;
        return true;
    }

    PyObject *temp = object_as_list_of_bytes(obj);
    *list = temp;
    return temp != NULL;
}


static int bool_false_converter(PyObject *obj, bool *result) {
    if (!obj || (obj == Py_None)) {
        *result = false;
    } else {
        int value = PyObject_IsTrue(obj);
        if (value < 0) {
            return false;
        }
        *result = value != 0;
    }
    return true;
}


static bool pylong_to_signum(PyObject *obj, int *result) {
    long value;
#if PY_MAJOR_VERSION >= 3
    value = PyLong_AsLong(obj);
#else
    value = PyInt_AsLong(obj);
#endif
    if ((value < 0) || (value > _NSIG)) {
        if (!((value == -1) || PyErr_Occurred())) {
            PyErr_Format(
                PyExc_OverflowError, "Signal number out of range: not (0 <= %zd <= %d)",
                value, _NSIG
            );
        }
        return false;
    }
    *result = (int) value;
    return true;
}


static int signal_x_convert(PyObject *obj, int *result, int x) {
    if (!obj || (obj == Py_None)) {
        *result = x;
        return true;
    } else {
        return pylong_to_signum(obj, result);
    }
}

static int signal_0_convert(PyObject *obj, int *result) {
    return signal_x_convert(obj, result, 0);
}

static int signal_m1_convert(PyObject *obj, int *result) {
    return signal_x_convert(obj, result, -1);
}


static int sigign_converter(PyObject *obj, uint64_t *result) {
    if (!obj || (obj == Py_None)) {
        *result = 0;
        return true;
    }

    int truthy = PyObject_IsTrue(obj);
    if (truthy < 0) {
        return false;
    } else if (!truthy) {
        *result = 0;
        return true;
    }

    unsigned long long ull_value;
#if PY_MAJOR_VERSION >= 3
    ull_value = PyLong_AsUnsignedLongLong(obj);
#else
    ull_value = PyInt_AsUnsignedLongLongMask(obj);
#endif
    if ((ull_value == (unsigned long long) -1) && PyErr_Occurred()) {
        if (!PyErr_ExceptionMatches(PyExc_TypeError)) {
            return false;
        }
        PyErr_Clear();
    } else if ((ull_value & ((uint64_t) -1)) == ull_value) {
        *result = ull_value & ((uint64_t) -1);
        return true;
    } else {
        // impossible unless sizeof(long long) != 64 bits
        PyErr_SetNone(PyExc_OverflowError);
        return false;
    }

    uint64_t accu = 0;
    bool success = false;
    PyObject *iterator = PyObject_GetIter(obj);
    if (!iterator) {
        return false;
    }
    PyObject *(*iternext)(PyObject *) = *Py_TYPE(iterator)->tp_iternext;
    for (;;) {
        PyObject *elem = iternext(iterator);
        if (!elem) {
            if (PyErr_Occurred()) {
                if (!PyErr_ExceptionMatches(PyExc_StopIteration)) {
                    success = false;
                    break;
                }
                PyErr_Clear();
            }
            success = true;
            break;
        }

        int signum;
        success = pylong_to_signum(elem, &signum);
        Py_DECREF(elem);
        if (!success) {
            break;
        }
        if (signum > 0) {
            accu |= 1 << (signum - 1);
        }
    }

    Py_DECREF(iterator);
    if (success) {
        *result = accu;
    }
    return success;
}


static PyObject *getpdeathsignal_impl(PyObject *self, PyObject *no_args) {
    (void) self;
    (void) no_args;

    int signal = -1;
    int outcome = prctl(
        PR_GET_PDEATHSIG,
        (unsigned long) (uintptr_t) &signal,
        0, 0, 0
    );
    return int_or_errno(outcome == 0, signal);
}


static PyObject *setpdeathsignal_impl(PyObject *self, PyObject *args, PyObject *kwargs) {
    (void) self;

    int signal = 0;
    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs, "|O&:setpdeathsignal",
        (char**) setpdeathsignal_keywords,
        signal_0_convert, &signal
    )) {
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


enum {
    ETD_SEARCH_PATH,
    ETD_SETSID,
    ETD_DOUBLEFORK,
};

typedef struct {
    char *path;
    char **argv;
    char **envp;
    unsigned flags;
    int parent_signal;
    uint64_t sigign;
    int childpid;
    char *fun;
    int error;
} ExecTrampolineData;


static int exec_trampoline(void *arg) {
    ExecTrampolineData *data = (ExecTrampolineData*) arg;
    char *fun = NULL;

    if (data->flags & (1 << ETD_SETSID)) {
        data->flags &= ~(1 << ETD_SETSID);
        pid_t outcome = setsid();
        if (outcome == (pid_t) -1) {
            fun = "setsid";
            goto fail;
        }
    }

    int parent_signal = data->parent_signal;
    if (parent_signal >= 0) {
        data->parent_signal = 0;
        int outcome = prctl(PR_SET_PDEATHSIG, parent_signal, 0, 0, 0);
        if (outcome != 0) {
            fun = "PR_SET_PDEATHSIG";
            goto fail;
        }
    }

    if (data->flags & (1 << ETD_DOUBLEFORK)) {
        data->flags &= ~(1 << ETD_DOUBLEFORK);
        char child_stack[1 << 10];
        int childprocess = clone(exec_trampoline, child_stack + sizeof(child_stack), CLONE_VFORK | CLONE_VM, data);
        if (childprocess < 0) {
            fun = "clone(doublefork)";
        }
        goto fail;
    }

    uint64_t sigign = data->sigign;
    if (sigign) {
        data->sigign = 0;
        int signum = 0;
        do {
            bool has = ((sigign & 1) == 1);
            sigign >>= 1;
            ++signum;

            if (!has || (signum == SIGKILL) || (signum == SIGSTOP)) {
                continue;
            }

            sighandler_t outcome = signal(signum, SIG_IGN);
            if (outcome == SIG_ERR) {
                fun = "signal";
                goto fail;
            }
        } while (sigign);
    }

    data->childpid = getpid();

    if (data->envp) {
        if (data->flags & (1 << ETD_SEARCH_PATH)) {
            execvpe(data->path,  data->argv, data->envp);
            fun = "execvpe";
        } else {
            execve(data->path, data->argv, data->envp);
            fun = "execve";
        }
    } else {
        if (data->flags & (1 << ETD_SEARCH_PATH)) {
            execvp(data->path, data->argv);
            fun = "execvp";
        } else {
            execv(data->path, data->argv);
            fun = "execv";
        }
    }

  fail:
    data->fun = fun;
    data->error = errno;
    raise(SIGKILL);
    return -1;
}


static PyObject *cloneandexecve_impl(PyObject *self, PyObject *args, PyObject *kwargs) {
    (void) self;

    PyObject *result = NULL;
    PyObject *exec_path = NULL;
    PyObject *exec_args = NULL;
    PyObject *exec_env = NULL;
    int exec_signal = -1;
    bool exec_sibling = false;
    bool exec_search_path = false;
    bool exec_setsid = false;
    bool exec_doublefork = false;
    uint64_t exec_sigign = 0;
    char **argv_list = NULL;
    char **envp_list = NULL;

    if (!PyArg_ParseTupleAndKeywords(
        args, kwargs,
            "O&"
        "|" "O&" "O&"
#if PY_VERSION_HEX >= 0x03030000
        "$"
#endif
        "O&" "O&" "O&" "O&" "O&" "O&"
        ":" "cloneandexecve",
        (char**) cloneandexecve_keywords,
        path_converter, &exec_path,
        // |
        bytes_list_converter, &exec_args,
        bytes_list_converter, &exec_env,
        // $
        signal_m1_convert, &exec_signal,
        bool_false_converter, &exec_sibling,
        bool_false_converter, &exec_search_path,
        bool_false_converter, &exec_setsid,
        bool_false_converter, &exec_doublefork,
        sigign_converter, &exec_sigign
    )) {
        return NULL;
    }

    argv_list = bytes_list_to_cstring_array(exec_args);
    if (!argv_list && PyErr_Occurred()) {
        goto end;
    }

    envp_list = bytes_list_to_cstring_array(exec_env);
    if (!envp_list && PyErr_Occurred()) {
        goto end;
    }

    const char *static_argv_list[] = { PyBytes_AS_STRING(exec_path), NULL };
    char child_stack[1 << 11];
    ExecTrampolineData trampoline_data = {
        (char*) static_argv_list[0],
        (argv_list ? (char**) argv_list : (char**) static_argv_list),
        envp_list,
        (
            (exec_search_path ? (1 << ETD_SEARCH_PATH) : 0) |
            (exec_setsid ? (1 << ETD_SETSID) : 0) |
            (exec_doublefork ? (1 << ETD_DOUBLEFORK) : 0) |
            0
        ),
        exec_signal,
        exec_sigign,
        -1, NULL, -1
    };
    int flags = CLONE_VFORK | CLONE_VM;
    if (exec_sibling) {
        flags |= CLONE_PARENT;
    }
    int childprocess = clone(exec_trampoline, child_stack + sizeof(child_stack), flags, &trampoline_data);
    if (childprocess < 0) {
        int code = errno;
        PyErr_Format(PyExc_Exception, "clone failed with errno=%d", code);
        goto end;
    }

    pid_t waitpid_result = waitpid(childprocess, NULL, WNOHANG) > 0;
    if ((trampoline_data.childpid >= 0) && (trampoline_data.childpid != childprocess)) {
        waitpid_result = waitpid(childprocess, NULL, WNOHANG) > 0;
    }

    if (trampoline_data.fun) {
        PyErr_Format(
            PyExc_OSError, "clone successful, but %s failed with errno=%d",
            trampoline_data.fun, trampoline_data.error
        );
        goto end;
    }

    if (waitpid_result == 0) {
#if PY_MAJOR_VERSION >= 3
        result = PyLong_FromLong(trampoline_data.childpid);
#else
        result = PyInt_FromLong(trampoline_data.childpid);
#endif
    } else {
        Py_INCREF(Py_None);
        result = Py_None;
    }

  end:
    pyfree(argv_list);
    pyfree(envp_list);
    Py_XDECREF(exec_path);
    Py_XDECREF(exec_args);
    Py_XDECREF(exec_env);
    return result;
}


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
