/*
 * C Extension module to test Python internal C APIs (Include/internal).
 */

#if !defined(Py_BUILD_CORE_BUILTIN) && !defined(Py_BUILD_CORE_MODULE)
#  error "Py_BUILD_CORE_BUILTIN or Py_BUILD_CORE_MODULE must be defined"
#endif

/* Always enable assertions */
#undef NDEBUG

#define PY_SSIZE_T_CLEAN

#include "Python.h"
#include "pycore_bitutils.h"     // _Py_bswap32()
#include "pycore_initconfig.h"   // _Py_GetConfigsAsDict()
#include "pycore_hashtable.h"    // _Py_hashtable_new()
#include "pycore_gc.h"           // PyGC_Head


static PyObject *
get_configs(PyObject *self, PyObject *Py_UNUSED(args))
{
    return _Py_GetConfigsAsDict();
}