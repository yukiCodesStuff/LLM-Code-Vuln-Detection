#include "pycore_gc.h"           // PyGC_Head


#ifdef MS_WINDOWS
#include <windows.h>

static int
_add_windows_config(PyObject *configs)
{
    HMODULE hPython3;
    wchar_t py3path[MAX_PATH];
    PyObject *dict = PyDict_New();
    PyObject *obj = NULL;
    if (!dict) {
        return -1;
    }

    hPython3 = GetModuleHandleW(PY3_DLLNAME);
    if (hPython3 && GetModuleFileNameW(hPython3, py3path, MAX_PATH)) {
        obj = PyUnicode_FromWideChar(py3path, -1);
    } else {
        obj = Py_None;
        Py_INCREF(obj);
    }
    if (obj &&
        !PyDict_SetItemString(dict, "python3_dll", obj) &&
        !PyDict_SetItemString(configs, "windows", dict)) {
        Py_DECREF(obj);
        Py_DECREF(dict);
        return 0;
    }
    Py_DECREF(obj);
    Py_DECREF(dict);
    return -1;
}
#endif


static PyObject *
get_configs(PyObject *self, PyObject *Py_UNUSED(args))
{
    PyObject *dict = _Py_GetConfigsAsDict();
#ifdef MS_WINDOWS
    if (dict) {
        if (_add_windows_config(dict) < 0) {
            Py_CLEAR(dict);
        }
    }
#endif
    return dict;
}


static PyObject*