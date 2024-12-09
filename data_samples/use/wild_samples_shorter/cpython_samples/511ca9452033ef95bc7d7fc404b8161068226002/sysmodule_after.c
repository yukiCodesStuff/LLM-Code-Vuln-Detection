#include "pycore_code.h"          // _Py_QuickenedCount
#include "pycore_frame.h"         // _PyInterpreterFrame
#include "pycore_initconfig.h"    // _PyStatus_EXCEPTION()
#include "pycore_long.h"          // _PY_LONG_MAX_STR_DIGITS_THRESHOLD
#include "pycore_namespace.h"     // _PyNamespace_New()
#include "pycore_object.h"        // _PyObject_IS_GC()
#include "pycore_pathconfig.h"    // _PyPathConfig_ComputeSysPath0()
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
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

/*[clinic input]
sys.set_int_max_str_digits

    maxdigits: int

Set the maximum string digits limit for non-binary int<->str conversions.
[clinic start generated code]*/

static PyObject *
sys_set_int_max_str_digits_impl(PyObject *module, int maxdigits)
/*[clinic end generated code: output=734d4c2511f2a56d input=d7e3f325db6910c5]*/
{
    PyThreadState *tstate = _PyThreadState_GET();
    if ((!maxdigits) || (maxdigits >= _PY_LONG_MAX_STR_DIGITS_THRESHOLD)) {
        tstate->interp->int_max_str_digits = maxdigits;
        Py_RETURN_NONE;
    } else {
        PyErr_Format(
            PyExc_ValueError, "maxdigits must be 0 or larger than %d",
            _PY_LONG_MAX_STR_DIGITS_THRESHOLD);
        return NULL;
    }
}

size_t
_PySys_GetSizeOf(PyObject *o)
{
    PyObject *res = NULL;
    SYS_DEACTIVATE_STACK_TRAMPOLINE_METHODDEF
    SYS_IS_STACK_TRAMPOLINE_ACTIVE_METHODDEF
    SYS_UNRAISABLEHOOK_METHODDEF
    SYS_GET_INT_MAX_STR_DIGITS_METHODDEF
    SYS_SET_INT_MAX_STR_DIGITS_METHODDEF
#ifdef Py_STATS
    SYS__STATS_ON_METHODDEF
    SYS__STATS_OFF_METHODDEF
    SYS__STATS_CLEAR_METHODDEF
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
    SetFlag(preconfig->utf8_mode);
    SetFlag(config->warn_default_encoding);
    SetFlagObj(PyBool_FromLong(config->safe_path));
    SetFlag(_Py_global_config_int_max_str_digits);
#undef SetFlagObj
#undef SetFlag
    return 0;
}