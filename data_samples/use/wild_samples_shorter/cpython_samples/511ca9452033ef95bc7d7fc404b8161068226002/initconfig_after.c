#include "pycore_getopt.h"        // _PyOS_GetOpt()
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_interp.h"        // _PyInterpreterState.runtime
#include "pycore_long.h"          // _PY_LONG_MAX_STR_DIGITS_THRESHOLD
#include "pycore_pathconfig.h"    // _Py_path_config
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
#include "pycore_pylifecycle.h"   // _Py_PreInitializeFromConfig()
#include "pycore_pymem.h"         // _PyMem_SetDefaultAllocator()
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).\n\
\n\
-X int_max_str_digits=number: limit the size of int<->str conversions.\n\
    This helps avoid denial of service attacks when parsing untrusted data.\n\
    The default is sys.int_info.default_max_str_digits.  0 disables.";

/* Envvars that don't have equivalent command-line options are listed first */
static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"PYTHONINTMAXSTRDIGITS: limits the maximum digit characters in an int value\n"
"   when converting from a string and when converting an int back to a str.\n"
"   A value of 0 disables the limit.  Conversions to or from bases 2, 4, 8,\n"
"   16, and 32 are never limited.\n"
"PYTHONMALLOC: set the Python memory allocators and/or install debug hooks\n"
"   on Python memory allocators. Use PYTHONMALLOC=debug to install debug\n"
"   hooks.\n"
"PYTHONCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
    config->code_debug_ranges = 1;
}

/* Excluded from public struct PyConfig for backporting reasons. */
/* default to unconfigured, _PyLong_InitTypes() does the rest */
int _Py_global_config_int_max_str_digits = -1;


static void
config_init_defaults(PyConfig *config)
{
    return _PyStatus_OK();
}

static PyStatus
config_init_int_max_str_digits(PyConfig *config)
{
    int maxdigits;
    int valid = 0;

    const char *env = config_get_env(config, "PYTHONINTMAXSTRDIGITS");
    if (env) {
        if (!_Py_str_to_int(env, &maxdigits)) {
            valid = ((maxdigits == 0) || (maxdigits >= _PY_LONG_MAX_STR_DIGITS_THRESHOLD));
        }
        if (!valid) {
#define STRINGIFY(VAL) _STRINGIFY(VAL)
#define _STRINGIFY(VAL) #VAL
            return _PyStatus_ERR(
                    "PYTHONINTMAXSTRDIGITS: invalid limit; must be >= "
                    STRINGIFY(_PY_LONG_MAX_STR_DIGITS_THRESHOLD)
                    " or 0 for unlimited.");
        }
        _Py_global_config_int_max_str_digits = maxdigits;
    }

    const wchar_t *xoption = config_get_xoption(config, L"int_max_str_digits");
    if (xoption) {
        const wchar_t *sep = wcschr(xoption, L'=');
        if (sep) {
            if (!config_wstr_to_int(sep + 1, &maxdigits)) {
                valid = ((maxdigits == 0) || (maxdigits >= _PY_LONG_MAX_STR_DIGITS_THRESHOLD));
            }
        }
        if (!valid) {
            return _PyStatus_ERR(
                    "-X int_max_str_digits: invalid limit; must be >= "
                    STRINGIFY(_PY_LONG_MAX_STR_DIGITS_THRESHOLD)
                    " or 0 for unlimited.");
#undef _STRINGIFY
#undef STRINGIFY
        }
        _Py_global_config_int_max_str_digits = maxdigits;
    }
    return _PyStatus_OK();
}

static PyStatus
config_init_pycache_prefix(PyConfig *config)
{
            return status;
        }
    }

    if (config->perf_profiling < 0) {
        status = config_init_perf_profiling(config);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    if (_Py_global_config_int_max_str_digits < 0) {
        status = config_init_int_max_str_digits(config);
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    if (config->pycache_prefix == NULL) {
        status = config_init_pycache_prefix(config);
        if (_PyStatus_EXCEPTION(status)) {
            return status;