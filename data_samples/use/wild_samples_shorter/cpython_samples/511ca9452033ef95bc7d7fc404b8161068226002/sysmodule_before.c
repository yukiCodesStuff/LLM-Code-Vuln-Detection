#include "pycore_code.h"          // _Py_QuickenedCount
#include "pycore_frame.h"         // _PyInterpreterFrame
#include "pycore_initconfig.h"    // _PyStatus_EXCEPTION()
#include "pycore_namespace.h"     // _PyNamespace_New()
#include "pycore_object.h"        // _PyObject_IS_GC()
#include "pycore_pathconfig.h"    // _PyPathConfig_ComputeSysPath0()
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
}
#endif /* USE_MALLOPT */

size_t
_PySys_GetSizeOf(PyObject *o)
{
    PyObject *res = NULL;
    SYS_DEACTIVATE_STACK_TRAMPOLINE_METHODDEF
    SYS_IS_STACK_TRAMPOLINE_ACTIVE_METHODDEF
    SYS_UNRAISABLEHOOK_METHODDEF
#ifdef Py_STATS
    SYS__STATS_ON_METHODDEF
    SYS__STATS_OFF_METHODDEF
    SYS__STATS_CLEAR_METHODDEF
    {"utf8_mode",               "-X utf8"},
    {"warn_default_encoding",   "-X warn_default_encoding"},
    {"safe_path", "-P"},
    {0}
};

static PyStructSequence_Desc flags_desc = {
    "sys.flags",        /* name */
    flags__doc__,       /* doc */
    flags_fields,       /* fields */
    17
};

static int
set_flags_from_config(PyInterpreterState *interp, PyObject *flags)
    SetFlag(preconfig->utf8_mode);
    SetFlag(config->warn_default_encoding);
    SetFlagObj(PyBool_FromLong(config->safe_path));
#undef SetFlagObj
#undef SetFlag
    return 0;
}