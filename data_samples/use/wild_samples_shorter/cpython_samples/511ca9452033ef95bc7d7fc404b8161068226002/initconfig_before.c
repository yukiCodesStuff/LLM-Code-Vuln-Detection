#include "pycore_getopt.h"        // _PyOS_GetOpt()
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_interp.h"        // _PyInterpreterState.runtime
#include "pycore_pathconfig.h"    // _Py_path_config
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
#include "pycore_pylifecycle.h"   // _Py_PreInitializeFromConfig()
#include "pycore_pymem.h"         // _PyMem_SetDefaultAllocator()
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).";

/* Envvars that don't have equivalent command-line options are listed first */
static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"PYTHONMALLOC: set the Python memory allocators and/or install debug hooks\n"
"   on Python memory allocators. Use PYTHONMALLOC=debug to install debug\n"
"   hooks.\n"
"PYTHONCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
    config->code_debug_ranges = 1;
}


static void
config_init_defaults(PyConfig *config)
{
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

    if (config->pycache_prefix == NULL) {
        status = config_init_pycache_prefix(config);
        if (_PyStatus_EXCEPTION(status)) {
            return status;