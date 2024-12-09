- exit(sts): raise SystemExit
Data members:
- stdin, stdout, stderr: standard file objects
- modules: the table of modules (dictionary)
- path: module search path (list of strings)
- argv: script arguments (list of strings)
- ps1, ps2: optional primary and secondary prompts (strings)
*/

#include "Python.h"
#include "pycore_call.h"          // _PyObject_CallNoArgs()
#include "pycore_ceval.h"         // _PyEval_SetAsyncGenFinalizer()
#include "pycore_code.h"          // _Py_QuickenedCount
#include "pycore_frame.h"         // _PyInterpreterFrame
#include "pycore_initconfig.h"    // _PyStatus_EXCEPTION()
#include "pycore_long.h"          // _PY_LONG_MAX_STR_DIGITS_THRESHOLD
#include "pycore_namespace.h"     // _PyNamespace_New()
#include "pycore_object.h"        // _PyObject_IS_GC()
#include "pycore_pathconfig.h"    // _PyPathConfig_ComputeSysPath0()
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
#include "pycore_pylifecycle.h"   // _PyErr_WriteUnraisableDefaultHook()
#include "pycore_pymath.h"        // _PY_SHORT_FLOAT_REPR
#include "pycore_pymem.h"         // _PyMem_SetDefaultAllocator()
#include "pycore_pystate.h"       // _PyThreadState_GET()
#include "pycore_structseq.h"     // _PyStructSequence_InitBuiltinWithFlags()
#include "pycore_tuple.h"         // _PyTuple_FromArray()

#include "frameobject.h"          // PyFrame_FastToLocalsWithError()
#include "pydtrace.h"
#include "osdefs.h"               // DELIM
#include "stdlib_module_names.h"  // _Py_stdlib_module_names
#include <locale.h>

#ifdef MS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif /* MS_WINDOWS */

#ifdef MS_COREDLL
extern void *PyWin_DLLhModule;
/* A string loaded from the DLL at startup: */
extern const char *PyWin_DLLVersionString;
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/*[clinic input]
module sys
[clinic start generated code]*/
/*[clinic end generated code: output=da39a3ee5e6b4b0d input=3726b388feee8cea]*/

#include "clinic/sysmodule.c.h"

PyObject *
_PySys_GetAttr(PyThreadState *tstate, PyObject *name)
{
    PyObject *sd = tstate->interp->sysdict;
    if (sd == NULL) {
        return NULL;
    }
    PyObject *exc_type, *exc_value, *exc_tb;
    _PyErr_Fetch(tstate, &exc_type, &exc_value, &exc_tb);
    /* XXX Suppress a new exception if it was raised and restore
     * the old one. */
    PyObject *value = _PyDict_GetItemWithError(sd, name);
    _PyErr_Restore(tstate, exc_type, exc_value, exc_tb);
    return value;
}
{
    int flag;
    mallopt(M_DEBUG, flag);
    Py_RETURN_NONE;
}
#endif /* USE_MALLOPT */


/*[clinic input]
sys.get_int_max_str_digits

Set the maximum string digits limit for non-binary int<->str conversions.
[clinic start generated code]*/

static PyObject *
sys_get_int_max_str_digits_impl(PyObject *module)
/*[clinic end generated code: output=0042f5e8ae0e8631 input=8dab13e2023e60d5]*/
{
    PyInterpreterState *interp = _PyInterpreterState_GET();
    return PyLong_FromSsize_t(interp->int_max_str_digits);
}
static PyMethodDef sys_methods[] = {
    /* Might as well keep this in alphabetic order */
    SYS_ADDAUDITHOOK_METHODDEF
    {"audit", _PyCFunction_CAST(sys_audit), METH_FASTCALL, audit_doc },
    {"breakpointhook", _PyCFunction_CAST(sys_breakpointhook),
     METH_FASTCALL | METH_KEYWORDS, breakpointhook_doc},
    SYS__CLEAR_TYPE_CACHE_METHODDEF
    SYS__CURRENT_FRAMES_METHODDEF
    SYS__CURRENT_EXCEPTIONS_METHODDEF
    SYS_DISPLAYHOOK_METHODDEF
    SYS_EXCEPTION_METHODDEF
    SYS_EXC_INFO_METHODDEF
    SYS_EXCEPTHOOK_METHODDEF
    SYS_EXIT_METHODDEF
    SYS_GETDEFAULTENCODING_METHODDEF
    SYS_GETDLOPENFLAGS_METHODDEF
    SYS_GETALLOCATEDBLOCKS_METHODDEF
#ifdef Py_STATS
    {"getdxp", _Py_GetDXProfile, METH_VARARGS},
#endif
    SYS_GETFILESYSTEMENCODING_METHODDEF
    SYS_GETFILESYSTEMENCODEERRORS_METHODDEF
    SYS__GETQUICKENEDCOUNT_METHODDEF
#ifdef Py_TRACE_REFS
    {"getobjects", _Py_GetObjects, METH_VARARGS},
#endif
    SYS_GETTOTALREFCOUNT_METHODDEF
    SYS_GETREFCOUNT_METHODDEF
    SYS_GETRECURSIONLIMIT_METHODDEF
    {"getsizeof", _PyCFunction_CAST(sys_getsizeof),
     METH_VARARGS | METH_KEYWORDS, getsizeof_doc},
    SYS__GETFRAME_METHODDEF
    SYS_GETWINDOWSVERSION_METHODDEF
    SYS__ENABLELEGACYWINDOWSFSENCODING_METHODDEF
    SYS_INTERN_METHODDEF
    SYS_IS_FINALIZING_METHODDEF
    SYS_MDEBUG_METHODDEF
    SYS_SETSWITCHINTERVAL_METHODDEF
    SYS_GETSWITCHINTERVAL_METHODDEF
    SYS_SETDLOPENFLAGS_METHODDEF
    {"setprofile", sys_setprofile, METH_O, setprofile_doc},
    SYS__SETPROFILEALLTHREADS_METHODDEF
    SYS_GETPROFILE_METHODDEF
    SYS_SETRECURSIONLIMIT_METHODDEF
    {"settrace", sys_settrace, METH_O, settrace_doc},
    SYS__SETTRACEALLTHREADS_METHODDEF
    SYS_GETTRACE_METHODDEF
    SYS_CALL_TRACING_METHODDEF
    SYS__DEBUGMALLOCSTATS_METHODDEF
    SYS_SET_COROUTINE_ORIGIN_TRACKING_DEPTH_METHODDEF
    SYS_GET_COROUTINE_ORIGIN_TRACKING_DEPTH_METHODDEF
    {"set_asyncgen_hooks", _PyCFunction_CAST(sys_set_asyncgen_hooks),
     METH_VARARGS | METH_KEYWORDS, set_asyncgen_hooks_doc},
    SYS_GET_ASYNCGEN_HOOKS_METHODDEF
    SYS_GETANDROIDAPILEVEL_METHODDEF
    SYS_ACTIVATE_STACK_TRAMPOLINE_METHODDEF
    SYS_DEACTIVATE_STACK_TRAMPOLINE_METHODDEF
    SYS_IS_STACK_TRAMPOLINE_ACTIVE_METHODDEF
    SYS_UNRAISABLEHOOK_METHODDEF
    SYS_GET_INT_MAX_STR_DIGITS_METHODDEF
    SYS_SET_INT_MAX_STR_DIGITS_METHODDEF
#ifdef Py_STATS
    SYS__STATS_ON_METHODDEF
    SYS__STATS_OFF_METHODDEF
    SYS__STATS_CLEAR_METHODDEF
    SYS__STATS_DUMP_METHODDEF
#endif
    {NULL, NULL}  // sentinel
};


static PyObject *
list_builtin_module_names(void)
{
    PyObject *list = PyList_New(0);
    if (list == NULL) {
        return NULL;
    }
    for (Py_ssize_t i = 0; PyImport_Inittab[i].name != NULL; i++) {
        PyObject *name = PyUnicode_FromString(PyImport_Inittab[i].name);
        if (name == NULL) {
            goto error;
        }
        if (PyList_Append(list, name) < 0) {
            Py_DECREF(name);
            goto error;
        }
        Py_DECREF(name);
    }
    if (PyList_Sort(list) != 0) {
        goto error;
    }
    PyObject *tuple = PyList_AsTuple(list);
    Py_DECREF(list);
    return tuple;

error:
    Py_DECREF(list);
    return NULL;
}
static PyStructSequence_Field flags_fields[] = {
    {"debug",                   "-d"},
    {"inspect",                 "-i"},
    {"interactive",             "-i"},
    {"optimize",                "-O or -OO"},
    {"dont_write_bytecode",     "-B"},
    {"no_user_site",            "-s"},
    {"no_site",                 "-S"},
    {"ignore_environment",      "-E"},
    {"verbose",                 "-v"},
    {"bytes_warning",           "-b"},
    {"quiet",                   "-q"},
    {"hash_randomization",      "-R"},
    {"isolated",                "-I"},
    {"dev_mode",                "-X dev"},
    {"utf8_mode",               "-X utf8"},
    {"warn_default_encoding",   "-X warn_default_encoding"},
    {"safe_path", "-P"},
    {"int_max_str_digits",      "-X int_max_str_digits"},
    {0}
};

static PyStructSequence_Desc flags_desc = {
    "sys.flags",        /* name */
    flags__doc__,       /* doc */
    flags_fields,       /* fields */
    18
};

static int
set_flags_from_config(PyInterpreterState *interp, PyObject *flags)
{
    const PyPreConfig *preconfig = &interp->runtime->preconfig;
    const PyConfig *config = _PyInterpreterState_GetConfig(interp);

    // _PySys_UpdateConfig() modifies sys.flags in-place:
    // Py_XDECREF() is needed in this case.
    Py_ssize_t pos = 0;
#define SetFlagObj(expr) \
    do { \
        PyObject *value = (expr); \
        if (value == NULL) { \
            return -1; \
        } \
        Py_XDECREF(PyStructSequence_GET_ITEM(flags, pos)); \
        PyStructSequence_SET_ITEM(flags, pos, value); \
        pos++; \
    } while (0)
#define SetFlag(expr) SetFlagObj(PyLong_FromLong(expr))

    SetFlag(config->parser_debug);
    SetFlag(config->inspect);
    SetFlag(config->interactive);
    SetFlag(config->optimization_level);
    SetFlag(!config->write_bytecode);
    SetFlag(!config->user_site_directory);
    SetFlag(!config->site_import);
    SetFlag(!config->use_environment);
    SetFlag(config->verbose);
    SetFlag(config->bytes_warning);
    SetFlag(config->quiet);
    SetFlag(config->use_hash_seed == 0 || config->hash_seed != 0);
    SetFlag(config->isolated);
    SetFlagObj(PyBool_FromLong(config->dev_mode));
    SetFlag(preconfig->utf8_mode);
    SetFlag(config->warn_default_encoding);
    SetFlagObj(PyBool_FromLong(config->safe_path));
    SetFlag(_Py_global_config_int_max_str_digits);
#undef SetFlagObj
#undef SetFlag
    return 0;
}
{
    const PyPreConfig *preconfig = &interp->runtime->preconfig;
    const PyConfig *config = _PyInterpreterState_GetConfig(interp);

    // _PySys_UpdateConfig() modifies sys.flags in-place:
    // Py_XDECREF() is needed in this case.
    Py_ssize_t pos = 0;
#define SetFlagObj(expr) \
    do { \
        PyObject *value = (expr); \
        if (value == NULL) { \
            return -1; \
        } \
        Py_XDECREF(PyStructSequence_GET_ITEM(flags, pos)); \
        PyStructSequence_SET_ITEM(flags, pos, value); \
        pos++; \
    } while (0)
#define SetFlag(expr) SetFlagObj(PyLong_FromLong(expr))

    SetFlag(config->parser_debug);
    SetFlag(config->inspect);
    SetFlag(config->interactive);
    SetFlag(config->optimization_level);
    SetFlag(!config->write_bytecode);
    SetFlag(!config->user_site_directory);
    SetFlag(!config->site_import);
    SetFlag(!config->use_environment);
    SetFlag(config->verbose);
    SetFlag(config->bytes_warning);
    SetFlag(config->quiet);
    SetFlag(config->use_hash_seed == 0 || config->hash_seed != 0);
    SetFlag(config->isolated);
    SetFlagObj(PyBool_FromLong(config->dev_mode));
    SetFlag(preconfig->utf8_mode);
    SetFlag(config->warn_default_encoding);
    SetFlagObj(PyBool_FromLong(config->safe_path));
    SetFlag(_Py_global_config_int_max_str_digits);
#undef SetFlagObj
#undef SetFlag
    return 0;
}


static PyObject*
make_flags(PyInterpreterState *interp)
{
    PyObject *flags = PyStructSequence_New(&FlagsType);
    if (flags == NULL) {
        return NULL;
    }