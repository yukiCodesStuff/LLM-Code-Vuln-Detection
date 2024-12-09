#include "Python.h"
#include "pycore_fileutils.h"     // _Py_HasFileSystemDefaultEncodeErrors
#include "pycore_getopt.h"        // _PyOS_GetOpt()
#include "pycore_initconfig.h"    // _PyStatus_OK()
#include "pycore_interp.h"        // _PyInterpreterState.runtime
#include "pycore_pathconfig.h"    // _Py_path_config
#include "pycore_pyerrors.h"      // _PyErr_Fetch()
#include "pycore_pylifecycle.h"   // _Py_PreInitializeFromConfig()
#include "pycore_pymem.h"         // _PyMem_SetDefaultAllocator()
#include "pycore_pystate.h"       // _PyThreadState_GET()

#include "osdefs.h"               // DELIM

#include <locale.h>               // setlocale()
#include <stdlib.h>               // getenv()
#if defined(MS_WINDOWS) || defined(__CYGWIN__)
#  ifdef HAVE_IO_H
#    include <io.h>
#  endif
#  ifdef HAVE_FCNTL_H
#    include <fcntl.h>            // O_BINARY
#  endif
#endif

/* --- Command line options --------------------------------------- */

/* Short usage message (with %s for argv0) */
static const char usage_line[] =
"usage: %ls [option] ... [-c cmd | -m mod | file | -] [arg] ...\n";

/* Long help message */
/* Lines sorted by option name; keep in sync with usage_envvars* below */
static const char usage_help[] = "\
Options (and corresponding environment variables):\n\
-b     : issue warnings about str(bytes_instance), str(bytearray_instance)\n\
         and comparing bytes/bytearray with str. (-bb: issue errors)\n\
-B     : don't write .pyc files on import; also PYTHONDONTWRITEBYTECODE=x\n\
-c cmd : program passed in as string (terminates option list)\n\
-d     : turn on parser debugging output (for experts only, only works on\n\
         debug builds); also PYTHONDEBUG=x\n\
-E     : ignore PYTHON* environment variables (such as PYTHONPATH)\n\
-h     : print this help message and exit (also -? or --help)\n\
-i     : inspect interactively after running script; forces a prompt even\n\
         if stdin does not appear to be a terminal; also PYTHONINSPECT=x\n\
-I     : isolate Python from the user's environment (implies -E and -s)\n\
-m mod : run library module as a script (terminates option list)\n\
-O     : remove assert and __debug__-dependent statements; add .opt-1 before\n\
         .pyc extension; also PYTHONOPTIMIZE=x\n\
-OO    : do -O changes and also discard docstrings; add .opt-2 before\n\
         .pyc extension\n\
-P     : don't prepend a potentially unsafe path to sys.path\n\
-q     : don't print version and copyright messages on interactive startup\n\
-s     : don't add user site directory to sys.path; also PYTHONNOUSERSITE\n\
-S     : don't imply 'import site' on initialization\n\
-u     : force the stdout and stderr streams to be unbuffered;\n\
         this option has no effect on stdin; also PYTHONUNBUFFERED=x\n\
-v     : verbose (trace import statements); also PYTHONVERBOSE=x\n\
         can be supplied multiple times to increase verbosity\n\
-V     : print the Python version number and exit (also --version)\n\
         when given twice, print more information about the build\n\
-W arg : warning control; arg is action:message:category:module:lineno\n\
         also PYTHONWARNINGS=arg\n\
-x     : skip first line of source, allowing use of non-Unix forms of #!cmd\n\
-X opt : set implementation-specific option\n\
--check-hash-based-pycs always|default|never:\n\
         control how Python invalidates hash-based .pyc files\n\
--help-env      : print help about Python environment variables and exit\n\
--help-xoptions : print help about implementation-specific -X options and exit\n\
--help-all      : print complete help information and exit\n\
Arguments:\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
";

static const char usage_xoptions[] = "\
The following implementation-specific options are available:\n\
\n\
-X faulthandler: enable faulthandler\n\
\n\
-X showrefcount: output the total reference count and number of used\n\
    memory blocks when the program finishes or after each statement in the\n\
    interactive interpreter. This only works on debug builds\n\
\n\
-X tracemalloc: start tracing Python memory allocations using the\n\
    tracemalloc module. By default, only the most recent frame is stored in a\n\
    traceback of a trace. Use -X tracemalloc=NFRAME to start tracing with a\n\
    traceback limit of NFRAME frames\n\
\n\
-X importtime: show how long each import takes. It shows module name,\n\
    cumulative time (including nested imports) and self time (excluding\n\
    nested imports). Note that its output may be broken in multi-threaded\n\
    application. Typical usage is python3 -X importtime -c 'import asyncio'\n\
\n\
-X dev: enable CPython's \"development mode\", introducing additional runtime\n\
    checks which are too expensive to be enabled by default. Effect of the\n\
    developer mode:\n\
       * Add default warning filter, as -W default\n\
       * Install debug hooks on memory allocators: see the PyMem_SetupDebugHooks()\n\
         C function\n\
       * Enable the faulthandler module to dump the Python traceback on a crash\n\
       * Enable asyncio debug mode\n\
       * Set the dev_mode attribute of sys.flags to True\n\
       * io.IOBase destructor logs close() exceptions\n\
\n\
-X utf8: enable UTF-8 mode for operating system interfaces, overriding the default\n\
    locale-aware mode. -X utf8=0 explicitly disables UTF-8 mode (even when it would\n\
    otherwise activate automatically)\n\
\n\
-X pycache_prefix=PATH: enable writing .pyc files to a parallel tree rooted at the\n\
    given directory instead of to the code tree\n\
\n\
-X warn_default_encoding: enable opt-in EncodingWarning for 'encoding=None'\n\
\n\
-X no_debug_ranges: disable the inclusion of the tables mapping extra location \n\
   information (end line, start column offset and end column offset) to every \n\
   instruction in code objects. This is useful when smaller code objects and pyc \n\
   files are desired as well as suppressing the extra visual location indicators \n\
   when the interpreter displays tracebacks.\n\
\n\
-X perf: activate support for the Linux \"perf\" profiler by activating the \"perf\"\n\
    trampoline. When this option is activated, the Linux \"perf\" profiler will be \n\
    able to report Python calls. This option is only available on some platforms and will \n\
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).";

/* Envvars that don't have equivalent command-line options are listed first */
static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"PYTHONSTARTUP: file executed on interactive startup (no default)\n"
"PYTHONPATH   : '%lc'-separated list of directories prefixed to the\n"
"               default module search path.  The result is sys.path.\n"
"PYTHONSAFEPATH: don't prepend a potentially unsafe path to sys.path.\n"
"PYTHONHOME   : alternate <prefix> directory (or <prefix>%lc<exec_prefix>).\n"
"               The default module search path uses %s.\n"
"PYTHONPLATLIBDIR : override sys.platlibdir.\n"
"PYTHONCASEOK : ignore case in 'import' statements (Windows).\n"
"PYTHONUTF8: if set to 1, enable the UTF-8 mode.\n"
"PYTHONIOENCODING: Encoding[:errors] used for stdin/stdout/stderr.\n"
"PYTHONFAULTHANDLER: dump the Python traceback on fatal errors.\n"
"PYTHONHASHSEED: if this variable is set to 'random', a random value is used\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"PYTHONMALLOC: set the Python memory allocators and/or install debug hooks\n"
"   on Python memory allocators. Use PYTHONMALLOC=debug to install debug\n"
"   hooks.\n"
"PYTHONCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
"   coercion behavior. Use PYTHONCOERCECLOCALE=warn to request display of\n"
"   locale coercion and locale compatibility warnings on stderr.\n"
"PYTHONBREAKPOINT: if this variable is set to 0, it disables the default\n"
"   debugger. It can be set to the callable of your debugger of choice.\n"
"PYTHONDEVMODE: enable the development mode.\n"
"PYTHONPYCACHEPREFIX: root directory for bytecode cache (pyc) files.\n"
"PYTHONWARNDEFAULTENCODING: enable opt-in EncodingWarning for 'encoding=None'.\n"
"PYTHONNODEBUGRANGES: If this variable is set, it disables the inclusion of the \n"
"   tables mapping extra location information (end line, start column offset \n"
"   and end column offset) to every instruction in code objects. This is useful \n"
"   when smaller code objects and pyc files are desired as well as suppressing the \n"
"   extra visual location indicators when the interpreter displays tracebacks.\n"
"These variables have equivalent command-line parameters (see --help for details):\n"
"PYTHONDEBUG             : enable parser debug mode (-d)\n"
"PYTHONDONTWRITEBYTECODE : don't write .pyc files (-B)\n"
"PYTHONINSPECT           : inspect interactively after running script (-i)\n"
"PYTHONNOUSERSITE        : disable user site directory (-s)\n"
"PYTHONOPTIMIZE          : enable level 1 optimizations (-O)\n"
"PYTHONUNBUFFERED        : disable stdout/stderr buffering (-u)\n"
"PYTHONVERBOSE           : trace import statements (-v)\n"
"PYTHONWARNINGS=arg      : warning control (-W arg)\n";

#if defined(MS_WINDOWS)
#  define PYTHONHOMEHELP "<prefix>\\python{major}{minor}"
#else
#  define PYTHONHOMEHELP "<prefix>/lib/pythonX.X"
#endif


/* --- Global configuration variables ----------------------------- */

/* UTF-8 mode (PEP 540): if equals to 1, use the UTF-8 encoding, and change
   stdin and stdout error handler to "surrogateescape". */
int Py_UTF8Mode = 0;
int Py_DebugFlag = 0; /* Needed by parser.c */
int Py_VerboseFlag = 0; /* Needed by import.c */
int Py_QuietFlag = 0; /* Needed by sysmodule.c */
int Py_InteractiveFlag = 0; /* Previously, was used by Py_FdIsInteractive() */
int Py_InspectFlag = 0; /* Needed to determine whether to exit at SystemExit */
int Py_OptimizeFlag = 0; /* Needed by compile.c */
int Py_NoSiteFlag = 0; /* Suppress 'import site' */
int Py_BytesWarningFlag = 0; /* Warn on str(bytes) and str(buffer) */
int Py_FrozenFlag = 0; /* Needed by getpath.c */
int Py_IgnoreEnvironmentFlag = 0; /* e.g. PYTHONPATH, PYTHONHOME */
int Py_DontWriteBytecodeFlag = 0; /* Suppress writing bytecode files (*.pyc) */
int Py_NoUserSiteDirectory = 0; /* for -s and site.py */
int Py_UnbufferedStdioFlag = 0; /* Unbuffered binary std{in,out,err} */
int Py_HashRandomizationFlag = 0; /* for -R and PYTHONHASHSEED */
int Py_IsolatedFlag = 0; /* for -I, isolate from user's env */
#ifdef MS_WINDOWS
int Py_LegacyWindowsFSEncodingFlag = 0; /* Uses mbcs instead of utf-8 */
int Py_LegacyWindowsStdioFlag = 0; /* Uses FileIO instead of WindowsConsoleIO */
#endif


static PyObject *
_Py_GetGlobalVariablesAsDict(void)
{
_Py_COMP_DIAG_PUSH
_Py_COMP_DIAG_IGNORE_DEPR_DECLS
    PyObject *dict, *obj;

    dict = PyDict_New();
    if (dict == NULL) {
        return NULL;
    }

#define SET_ITEM(KEY, EXPR) \
        do { \
            obj = (EXPR); \
            if (obj == NULL) { \
                return NULL; \
            } \
            int res = PyDict_SetItemString(dict, (KEY), obj); \
            Py_DECREF(obj); \
            if (res < 0) { \
                goto fail; \
            } \
        } while (0)
#define SET_ITEM_INT(VAR) \
    SET_ITEM(#VAR, PyLong_FromLong(VAR))
#define FROM_STRING(STR) \
    ((STR != NULL) ? \
        PyUnicode_FromString(STR) \
        : (Py_INCREF(Py_None), Py_None))
#define SET_ITEM_STR(VAR) \
    SET_ITEM(#VAR, FROM_STRING(VAR))

    SET_ITEM_STR(Py_FileSystemDefaultEncoding);
    SET_ITEM_INT(Py_HasFileSystemDefaultEncoding);
    SET_ITEM_STR(Py_FileSystemDefaultEncodeErrors);
    SET_ITEM_INT(_Py_HasFileSystemDefaultEncodeErrors);

    SET_ITEM_INT(Py_UTF8Mode);
    SET_ITEM_INT(Py_DebugFlag);
    SET_ITEM_INT(Py_VerboseFlag);
    SET_ITEM_INT(Py_QuietFlag);
    SET_ITEM_INT(Py_InteractiveFlag);
    SET_ITEM_INT(Py_InspectFlag);

    SET_ITEM_INT(Py_OptimizeFlag);
    SET_ITEM_INT(Py_NoSiteFlag);
    SET_ITEM_INT(Py_BytesWarningFlag);
    SET_ITEM_INT(Py_FrozenFlag);
    SET_ITEM_INT(Py_IgnoreEnvironmentFlag);
    SET_ITEM_INT(Py_DontWriteBytecodeFlag);
    SET_ITEM_INT(Py_NoUserSiteDirectory);
    SET_ITEM_INT(Py_UnbufferedStdioFlag);
    SET_ITEM_INT(Py_HashRandomizationFlag);
    SET_ITEM_INT(Py_IsolatedFlag);

#ifdef MS_WINDOWS
    SET_ITEM_INT(Py_LegacyWindowsFSEncodingFlag);
    SET_ITEM_INT(Py_LegacyWindowsStdioFlag);
#endif

    return dict;

fail:
    Py_DECREF(dict);
    return NULL;

#undef FROM_STRING
#undef SET_ITEM
#undef SET_ITEM_INT
#undef SET_ITEM_STR
_Py_COMP_DIAG_POP
}
Options (and corresponding environment variables):\n\
-b     : issue warnings about str(bytes_instance), str(bytearray_instance)\n\
         and comparing bytes/bytearray with str. (-bb: issue errors)\n\
-B     : don't write .pyc files on import; also PYTHONDONTWRITEBYTECODE=x\n\
-c cmd : program passed in as string (terminates option list)\n\
-d     : turn on parser debugging output (for experts only, only works on\n\
         debug builds); also PYTHONDEBUG=x\n\
-E     : ignore PYTHON* environment variables (such as PYTHONPATH)\n\
-h     : print this help message and exit (also -? or --help)\n\
-i     : inspect interactively after running script; forces a prompt even\n\
         if stdin does not appear to be a terminal; also PYTHONINSPECT=x\n\
-I     : isolate Python from the user's environment (implies -E and -s)\n\
-m mod : run library module as a script (terminates option list)\n\
-O     : remove assert and __debug__-dependent statements; add .opt-1 before\n\
         .pyc extension; also PYTHONOPTIMIZE=x\n\
-OO    : do -O changes and also discard docstrings; add .opt-2 before\n\
         .pyc extension\n\
-P     : don't prepend a potentially unsafe path to sys.path\n\
-q     : don't print version and copyright messages on interactive startup\n\
-s     : don't add user site directory to sys.path; also PYTHONNOUSERSITE\n\
-S     : don't imply 'import site' on initialization\n\
-u     : force the stdout and stderr streams to be unbuffered;\n\
         this option has no effect on stdin; also PYTHONUNBUFFERED=x\n\
-v     : verbose (trace import statements); also PYTHONVERBOSE=x\n\
         can be supplied multiple times to increase verbosity\n\
-V     : print the Python version number and exit (also --version)\n\
         when given twice, print more information about the build\n\
-W arg : warning control; arg is action:message:category:module:lineno\n\
         also PYTHONWARNINGS=arg\n\
-x     : skip first line of source, allowing use of non-Unix forms of #!cmd\n\
-X opt : set implementation-specific option\n\
--check-hash-based-pycs always|default|never:\n\
         control how Python invalidates hash-based .pyc files\n\
--help-env      : print help about Python environment variables and exit\n\
--help-xoptions : print help about implementation-specific -X options and exit\n\
--help-all      : print complete help information and exit\n\
Arguments:\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
";

static const char usage_xoptions[] = "\
The following implementation-specific options are available:\n\
\n\
-X faulthandler: enable faulthandler\n\
\n\
-X showrefcount: output the total reference count and number of used\n\
    memory blocks when the program finishes or after each statement in the\n\
    interactive interpreter. This only works on debug builds\n\
\n\
-X tracemalloc: start tracing Python memory allocations using the\n\
    tracemalloc module. By default, only the most recent frame is stored in a\n\
    traceback of a trace. Use -X tracemalloc=NFRAME to start tracing with a\n\
    traceback limit of NFRAME frames\n\
\n\
-X importtime: show how long each import takes. It shows module name,\n\
    cumulative time (including nested imports) and self time (excluding\n\
    nested imports). Note that its output may be broken in multi-threaded\n\
    application. Typical usage is python3 -X importtime -c 'import asyncio'\n\
\n\
-X dev: enable CPython's \"development mode\", introducing additional runtime\n\
    checks which are too expensive to be enabled by default. Effect of the\n\
    developer mode:\n\
       * Add default warning filter, as -W default\n\
       * Install debug hooks on memory allocators: see the PyMem_SetupDebugHooks()\n\
         C function\n\
       * Enable the faulthandler module to dump the Python traceback on a crash\n\
       * Enable asyncio debug mode\n\
       * Set the dev_mode attribute of sys.flags to True\n\
       * io.IOBase destructor logs close() exceptions\n\
\n\
-X utf8: enable UTF-8 mode for operating system interfaces, overriding the default\n\
    locale-aware mode. -X utf8=0 explicitly disables UTF-8 mode (even when it would\n\
    otherwise activate automatically)\n\
\n\
-X pycache_prefix=PATH: enable writing .pyc files to a parallel tree rooted at the\n\
    given directory instead of to the code tree\n\
\n\
-X warn_default_encoding: enable opt-in EncodingWarning for 'encoding=None'\n\
\n\
-X no_debug_ranges: disable the inclusion of the tables mapping extra location \n\
   information (end line, start column offset and end column offset) to every \n\
   instruction in code objects. This is useful when smaller code objects and pyc \n\
   files are desired as well as suppressing the extra visual location indicators \n\
   when the interpreter displays tracebacks.\n\
\n\
-X perf: activate support for the Linux \"perf\" profiler by activating the \"perf\"\n\
    trampoline. When this option is activated, the Linux \"perf\" profiler will be \n\
    able to report Python calls. This option is only available on some platforms and will \n\
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).";

/* Envvars that don't have equivalent command-line options are listed first */
static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"PYTHONSTARTUP: file executed on interactive startup (no default)\n"
"PYTHONPATH   : '%lc'-separated list of directories prefixed to the\n"
"               default module search path.  The result is sys.path.\n"
"PYTHONSAFEPATH: don't prepend a potentially unsafe path to sys.path.\n"
"PYTHONHOME   : alternate <prefix> directory (or <prefix>%lc<exec_prefix>).\n"
"               The default module search path uses %s.\n"
"PYTHONPLATLIBDIR : override sys.platlibdir.\n"
"PYTHONCASEOK : ignore case in 'import' statements (Windows).\n"
"PYTHONUTF8: if set to 1, enable the UTF-8 mode.\n"
"PYTHONIOENCODING: Encoding[:errors] used for stdin/stdout/stderr.\n"
"PYTHONFAULTHANDLER: dump the Python traceback on fatal errors.\n"
"PYTHONHASHSEED: if this variable is set to 'random', a random value is used\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"PYTHONMALLOC: set the Python memory allocators and/or install debug hooks\n"
"   on Python memory allocators. Use PYTHONMALLOC=debug to install debug\n"
"   hooks.\n"
"PYTHONCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
"   coercion behavior. Use PYTHONCOERCECLOCALE=warn to request display of\n"
"   locale coercion and locale compatibility warnings on stderr.\n"
"PYTHONBREAKPOINT: if this variable is set to 0, it disables the default\n"
"   debugger. It can be set to the callable of your debugger of choice.\n"
"PYTHONDEVMODE: enable the development mode.\n"
"PYTHONPYCACHEPREFIX: root directory for bytecode cache (pyc) files.\n"
"PYTHONWARNDEFAULTENCODING: enable opt-in EncodingWarning for 'encoding=None'.\n"
"PYTHONNODEBUGRANGES: If this variable is set, it disables the inclusion of the \n"
"   tables mapping extra location information (end line, start column offset \n"
"   and end column offset) to every instruction in code objects. This is useful \n"
"   when smaller code objects and pyc files are desired as well as suppressing the \n"
"   extra visual location indicators when the interpreter displays tracebacks.\n"
"These variables have equivalent command-line parameters (see --help for details):\n"
"PYTHONDEBUG             : enable parser debug mode (-d)\n"
"PYTHONDONTWRITEBYTECODE : don't write .pyc files (-B)\n"
"PYTHONINSPECT           : inspect interactively after running script (-i)\n"
"PYTHONNOUSERSITE        : disable user site directory (-s)\n"
"PYTHONOPTIMIZE          : enable level 1 optimizations (-O)\n"
"PYTHONUNBUFFERED        : disable stdout/stderr buffering (-u)\n"
"PYTHONVERBOSE           : trace import statements (-v)\n"
"PYTHONWARNINGS=arg      : warning control (-W arg)\n";

#if defined(MS_WINDOWS)
#  define PYTHONHOMEHELP "<prefix>\\python{major}{minor}"
#else
#  define PYTHONHOMEHELP "<prefix>/lib/pythonX.X"
#endif


/* --- Global configuration variables ----------------------------- */

/* UTF-8 mode (PEP 540): if equals to 1, use the UTF-8 encoding, and change
   stdin and stdout error handler to "surrogateescape". */
int Py_UTF8Mode = 0;
int Py_DebugFlag = 0; /* Needed by parser.c */
int Py_VerboseFlag = 0; /* Needed by import.c */
int Py_QuietFlag = 0; /* Needed by sysmodule.c */
int Py_InteractiveFlag = 0; /* Previously, was used by Py_FdIsInteractive() */
int Py_InspectFlag = 0; /* Needed to determine whether to exit at SystemExit */
int Py_OptimizeFlag = 0; /* Needed by compile.c */
int Py_NoSiteFlag = 0; /* Suppress 'import site' */
int Py_BytesWarningFlag = 0; /* Warn on str(bytes) and str(buffer) */
int Py_FrozenFlag = 0; /* Needed by getpath.c */
int Py_IgnoreEnvironmentFlag = 0; /* e.g. PYTHONPATH, PYTHONHOME */
int Py_DontWriteBytecodeFlag = 0; /* Suppress writing bytecode files (*.pyc) */
int Py_NoUserSiteDirectory = 0; /* for -s and site.py */
int Py_UnbufferedStdioFlag = 0; /* Unbuffered binary std{in,out,err} */
int Py_HashRandomizationFlag = 0; /* for -R and PYTHONHASHSEED */
int Py_IsolatedFlag = 0; /* for -I, isolate from user's env */
#ifdef MS_WINDOWS
int Py_LegacyWindowsFSEncodingFlag = 0; /* Uses mbcs instead of utf-8 */
int Py_LegacyWindowsStdioFlag = 0; /* Uses FileIO instead of WindowsConsoleIO */
#endif


static PyObject *
_Py_GetGlobalVariablesAsDict(void)
{
_Py_COMP_DIAG_PUSH
_Py_COMP_DIAG_IGNORE_DEPR_DECLS
    PyObject *dict, *obj;

    dict = PyDict_New();
    if (dict == NULL) {
        return NULL;
    }

#define SET_ITEM(KEY, EXPR) \
        do { \
            obj = (EXPR); \
            if (obj == NULL) { \
                return NULL; \
            } \
            int res = PyDict_SetItemString(dict, (KEY), obj); \
            Py_DECREF(obj); \
            if (res < 0) { \
                goto fail; \
            } \
        } while (0)
#define SET_ITEM_INT(VAR) \
    SET_ITEM(#VAR, PyLong_FromLong(VAR))
#define FROM_STRING(STR) \
    ((STR != NULL) ? \
        PyUnicode_FromString(STR) \
        : (Py_INCREF(Py_None), Py_None))
#define SET_ITEM_STR(VAR) \
    SET_ITEM(#VAR, FROM_STRING(VAR))

    SET_ITEM_STR(Py_FileSystemDefaultEncoding);
    SET_ITEM_INT(Py_HasFileSystemDefaultEncoding);
    SET_ITEM_STR(Py_FileSystemDefaultEncodeErrors);
    SET_ITEM_INT(_Py_HasFileSystemDefaultEncodeErrors);

    SET_ITEM_INT(Py_UTF8Mode);
    SET_ITEM_INT(Py_DebugFlag);
    SET_ITEM_INT(Py_VerboseFlag);
    SET_ITEM_INT(Py_QuietFlag);
    SET_ITEM_INT(Py_InteractiveFlag);
    SET_ITEM_INT(Py_InspectFlag);

    SET_ITEM_INT(Py_OptimizeFlag);
    SET_ITEM_INT(Py_NoSiteFlag);
    SET_ITEM_INT(Py_BytesWarningFlag);
    SET_ITEM_INT(Py_FrozenFlag);
    SET_ITEM_INT(Py_IgnoreEnvironmentFlag);
    SET_ITEM_INT(Py_DontWriteBytecodeFlag);
    SET_ITEM_INT(Py_NoUserSiteDirectory);
    SET_ITEM_INT(Py_UnbufferedStdioFlag);
    SET_ITEM_INT(Py_HashRandomizationFlag);
    SET_ITEM_INT(Py_IsolatedFlag);

#ifdef MS_WINDOWS
    SET_ITEM_INT(Py_LegacyWindowsFSEncodingFlag);
    SET_ITEM_INT(Py_LegacyWindowsStdioFlag);
#endif

    return dict;

fail:
    Py_DECREF(dict);
    return NULL;

#undef FROM_STRING
#undef SET_ITEM
#undef SET_ITEM_INT
#undef SET_ITEM_STR
_Py_COMP_DIAG_POP
}
Options (and corresponding environment variables):\n\
-b     : issue warnings about str(bytes_instance), str(bytearray_instance)\n\
         and comparing bytes/bytearray with str. (-bb: issue errors)\n\
-B     : don't write .pyc files on import; also PYTHONDONTWRITEBYTECODE=x\n\
-c cmd : program passed in as string (terminates option list)\n\
-d     : turn on parser debugging output (for experts only, only works on\n\
         debug builds); also PYTHONDEBUG=x\n\
-E     : ignore PYTHON* environment variables (such as PYTHONPATH)\n\
-h     : print this help message and exit (also -? or --help)\n\
-i     : inspect interactively after running script; forces a prompt even\n\
         if stdin does not appear to be a terminal; also PYTHONINSPECT=x\n\
-I     : isolate Python from the user's environment (implies -E and -s)\n\
-m mod : run library module as a script (terminates option list)\n\
-O     : remove assert and __debug__-dependent statements; add .opt-1 before\n\
         .pyc extension; also PYTHONOPTIMIZE=x\n\
-OO    : do -O changes and also discard docstrings; add .opt-2 before\n\
         .pyc extension\n\
-P     : don't prepend a potentially unsafe path to sys.path\n\
-q     : don't print version and copyright messages on interactive startup\n\
-s     : don't add user site directory to sys.path; also PYTHONNOUSERSITE\n\
-S     : don't imply 'import site' on initialization\n\
-u     : force the stdout and stderr streams to be unbuffered;\n\
         this option has no effect on stdin; also PYTHONUNBUFFERED=x\n\
-v     : verbose (trace import statements); also PYTHONVERBOSE=x\n\
         can be supplied multiple times to increase verbosity\n\
-V     : print the Python version number and exit (also --version)\n\
         when given twice, print more information about the build\n\
-W arg : warning control; arg is action:message:category:module:lineno\n\
         also PYTHONWARNINGS=arg\n\
-x     : skip first line of source, allowing use of non-Unix forms of #!cmd\n\
-X opt : set implementation-specific option\n\
--check-hash-based-pycs always|default|never:\n\
         control how Python invalidates hash-based .pyc files\n\
--help-env      : print help about Python environment variables and exit\n\
--help-xoptions : print help about implementation-specific -X options and exit\n\
--help-all      : print complete help information and exit\n\
Arguments:\n\
file   : program read from script file\n\
-      : program read from stdin (default; interactive mode if a tty)\n\
arg ...: arguments passed to program in sys.argv[1:]\n\
";

static const char usage_xoptions[] = "\
The following implementation-specific options are available:\n\
\n\
-X faulthandler: enable faulthandler\n\
\n\
-X showrefcount: output the total reference count and number of used\n\
    memory blocks when the program finishes or after each statement in the\n\
    interactive interpreter. This only works on debug builds\n\
\n\
-X tracemalloc: start tracing Python memory allocations using the\n\
    tracemalloc module. By default, only the most recent frame is stored in a\n\
    traceback of a trace. Use -X tracemalloc=NFRAME to start tracing with a\n\
    traceback limit of NFRAME frames\n\
\n\
-X importtime: show how long each import takes. It shows module name,\n\
    cumulative time (including nested imports) and self time (excluding\n\
    nested imports). Note that its output may be broken in multi-threaded\n\
    application. Typical usage is python3 -X importtime -c 'import asyncio'\n\
\n\
-X dev: enable CPython's \"development mode\", introducing additional runtime\n\
    checks which are too expensive to be enabled by default. Effect of the\n\
    developer mode:\n\
       * Add default warning filter, as -W default\n\
       * Install debug hooks on memory allocators: see the PyMem_SetupDebugHooks()\n\
         C function\n\
       * Enable the faulthandler module to dump the Python traceback on a crash\n\
       * Enable asyncio debug mode\n\
       * Set the dev_mode attribute of sys.flags to True\n\
       * io.IOBase destructor logs close() exceptions\n\
\n\
-X utf8: enable UTF-8 mode for operating system interfaces, overriding the default\n\
    locale-aware mode. -X utf8=0 explicitly disables UTF-8 mode (even when it would\n\
    otherwise activate automatically)\n\
\n\
-X pycache_prefix=PATH: enable writing .pyc files to a parallel tree rooted at the\n\
    given directory instead of to the code tree\n\
\n\
-X warn_default_encoding: enable opt-in EncodingWarning for 'encoding=None'\n\
\n\
-X no_debug_ranges: disable the inclusion of the tables mapping extra location \n\
   information (end line, start column offset and end column offset) to every \n\
   instruction in code objects. This is useful when smaller code objects and pyc \n\
   files are desired as well as suppressing the extra visual location indicators \n\
   when the interpreter displays tracebacks.\n\
\n\
-X perf: activate support for the Linux \"perf\" profiler by activating the \"perf\"\n\
    trampoline. When this option is activated, the Linux \"perf\" profiler will be \n\
    able to report Python calls. This option is only available on some platforms and will \n\
    do nothing if is not supported on the current system. The default value is \"off\".\n\
\n\
-X frozen_modules=[on|off]: whether or not frozen modules should be used.\n\
   The default is \"on\" (or \"off\" if you are running a local build).";

/* Envvars that don't have equivalent command-line options are listed first */
static const char usage_envvars[] =
"Environment variables that change behavior:\n"
"PYTHONSTARTUP: file executed on interactive startup (no default)\n"
"PYTHONPATH   : '%lc'-separated list of directories prefixed to the\n"
"               default module search path.  The result is sys.path.\n"
"PYTHONSAFEPATH: don't prepend a potentially unsafe path to sys.path.\n"
"PYTHONHOME   : alternate <prefix> directory (or <prefix>%lc<exec_prefix>).\n"
"               The default module search path uses %s.\n"
"PYTHONPLATLIBDIR : override sys.platlibdir.\n"
"PYTHONCASEOK : ignore case in 'import' statements (Windows).\n"
"PYTHONUTF8: if set to 1, enable the UTF-8 mode.\n"
"PYTHONIOENCODING: Encoding[:errors] used for stdin/stdout/stderr.\n"
"PYTHONFAULTHANDLER: dump the Python traceback on fatal errors.\n"
"PYTHONHASHSEED: if this variable is set to 'random', a random value is used\n"
"   to seed the hashes of str and bytes objects.  It can also be set to an\n"
"   integer in the range [0,4294967295] to get hash values with a\n"
"   predictable seed.\n"
"PYTHONMALLOC: set the Python memory allocators and/or install debug hooks\n"
"   on Python memory allocators. Use PYTHONMALLOC=debug to install debug\n"
"   hooks.\n"
"PYTHONCOERCECLOCALE: if this variable is set to 0, it disables the locale\n"
"   coercion behavior. Use PYTHONCOERCECLOCALE=warn to request display of\n"
"   locale coercion and locale compatibility warnings on stderr.\n"
"PYTHONBREAKPOINT: if this variable is set to 0, it disables the default\n"
"   debugger. It can be set to the callable of your debugger of choice.\n"
"PYTHONDEVMODE: enable the development mode.\n"
"PYTHONPYCACHEPREFIX: root directory for bytecode cache (pyc) files.\n"
"PYTHONWARNDEFAULTENCODING: enable opt-in EncodingWarning for 'encoding=None'.\n"
"PYTHONNODEBUGRANGES: If this variable is set, it disables the inclusion of the \n"
"   tables mapping extra location information (end line, start column offset \n"
"   and end column offset) to every instruction in code objects. This is useful \n"
"   when smaller code objects and pyc files are desired as well as suppressing the \n"
"   extra visual location indicators when the interpreter displays tracebacks.\n"
"These variables have equivalent command-line parameters (see --help for details):\n"
"PYTHONDEBUG             : enable parser debug mode (-d)\n"
"PYTHONDONTWRITEBYTECODE : don't write .pyc files (-B)\n"
"PYTHONINSPECT           : inspect interactively after running script (-i)\n"
"PYTHONNOUSERSITE        : disable user site directory (-s)\n"
"PYTHONOPTIMIZE          : enable level 1 optimizations (-O)\n"
"PYTHONUNBUFFERED        : disable stdout/stderr buffering (-u)\n"
"PYTHONVERBOSE           : trace import statements (-v)\n"
"PYTHONWARNINGS=arg      : warning control (-W arg)\n";

#if defined(MS_WINDOWS)
#  define PYTHONHOMEHELP "<prefix>\\python{major}{minor}"
#else
#  define PYTHONHOMEHELP "<prefix>/lib/pythonX.X"
#endif


/* --- Global configuration variables ----------------------------- */

/* UTF-8 mode (PEP 540): if equals to 1, use the UTF-8 encoding, and change
   stdin and stdout error handler to "surrogateescape". */
int Py_UTF8Mode = 0;
int Py_DebugFlag = 0; /* Needed by parser.c */
int Py_VerboseFlag = 0; /* Needed by import.c */
int Py_QuietFlag = 0; /* Needed by sysmodule.c */
int Py_InteractiveFlag = 0; /* Previously, was used by Py_FdIsInteractive() */
int Py_InspectFlag = 0; /* Needed to determine whether to exit at SystemExit */
int Py_OptimizeFlag = 0; /* Needed by compile.c */
int Py_NoSiteFlag = 0; /* Suppress 'import site' */
int Py_BytesWarningFlag = 0; /* Warn on str(bytes) and str(buffer) */
int Py_FrozenFlag = 0; /* Needed by getpath.c */
int Py_IgnoreEnvironmentFlag = 0; /* e.g. PYTHONPATH, PYTHONHOME */
int Py_DontWriteBytecodeFlag = 0; /* Suppress writing bytecode files (*.pyc) */
int Py_NoUserSiteDirectory = 0; /* for -s and site.py */
int Py_UnbufferedStdioFlag = 0; /* Unbuffered binary std{in,out,err} */
int Py_HashRandomizationFlag = 0; /* for -R and PYTHONHASHSEED */
int Py_IsolatedFlag = 0; /* for -I, isolate from user's env */
#ifdef MS_WINDOWS
int Py_LegacyWindowsFSEncodingFlag = 0; /* Uses mbcs instead of utf-8 */
int Py_LegacyWindowsStdioFlag = 0; /* Uses FileIO instead of WindowsConsoleIO */
#endif


static PyObject *
_Py_GetGlobalVariablesAsDict(void)
{
_Py_COMP_DIAG_PUSH
_Py_COMP_DIAG_IGNORE_DEPR_DECLS
    PyObject *dict, *obj;

    dict = PyDict_New();
    if (dict == NULL) {
        return NULL;
    }

#define SET_ITEM(KEY, EXPR) \
        do { \
            obj = (EXPR); \
            if (obj == NULL) { \
                return NULL; \
            } \
            int res = PyDict_SetItemString(dict, (KEY), obj); \
            Py_DECREF(obj); \
            if (res < 0) { \
                goto fail; \
            } \
        } while (0)
#define SET_ITEM_INT(VAR) \
    SET_ITEM(#VAR, PyLong_FromLong(VAR))
#define FROM_STRING(STR) \
    ((STR != NULL) ? \
        PyUnicode_FromString(STR) \
        : (Py_INCREF(Py_None), Py_None))
#define SET_ITEM_STR(VAR) \
    SET_ITEM(#VAR, FROM_STRING(VAR))

    SET_ITEM_STR(Py_FileSystemDefaultEncoding);
    SET_ITEM_INT(Py_HasFileSystemDefaultEncoding);
    SET_ITEM_STR(Py_FileSystemDefaultEncodeErrors);
    SET_ITEM_INT(_Py_HasFileSystemDefaultEncodeErrors);

    SET_ITEM_INT(Py_UTF8Mode);
    SET_ITEM_INT(Py_DebugFlag);
    SET_ITEM_INT(Py_VerboseFlag);
    SET_ITEM_INT(Py_QuietFlag);
    SET_ITEM_INT(Py_InteractiveFlag);
    SET_ITEM_INT(Py_InspectFlag);

    SET_ITEM_INT(Py_OptimizeFlag);
    SET_ITEM_INT(Py_NoSiteFlag);
    SET_ITEM_INT(Py_BytesWarningFlag);
    SET_ITEM_INT(Py_FrozenFlag);
    SET_ITEM_INT(Py_IgnoreEnvironmentFlag);
    SET_ITEM_INT(Py_DontWriteBytecodeFlag);
    SET_ITEM_INT(Py_NoUserSiteDirectory);
    SET_ITEM_INT(Py_UnbufferedStdioFlag);
    SET_ITEM_INT(Py_HashRandomizationFlag);
    SET_ITEM_INT(Py_IsolatedFlag);

#ifdef MS_WINDOWS
    SET_ITEM_INT(Py_LegacyWindowsFSEncodingFlag);
    SET_ITEM_INT(Py_LegacyWindowsStdioFlag);
#endif

    return dict;

fail:
    Py_DECREF(dict);
    return NULL;

#undef FROM_STRING
#undef SET_ITEM
#undef SET_ITEM_INT
#undef SET_ITEM_STR
_Py_COMP_DIAG_POP
}
{
    memset(config, 0, sizeof(*config));

    config->_config_init = (int)_PyConfig_INIT_COMPAT;
    config->isolated = -1;
    config->use_environment = -1;
    config->dev_mode = -1;
    config->install_signal_handlers = 1;
    config->use_hash_seed = -1;
    config->faulthandler = -1;
    config->tracemalloc = -1;
    config->perf_profiling = -1;
    config->module_search_paths_set = 0;
    config->parse_argv = 0;
    config->site_import = -1;
    config->bytes_warning = -1;
    config->warn_default_encoding = 0;
    config->inspect = -1;
    config->interactive = -1;
    config->optimization_level = -1;
    config->parser_debug= -1;
    config->write_bytecode = -1;
    config->verbose = -1;
    config->quiet = -1;
    config->user_site_directory = -1;
    config->configure_c_stdio = 0;
    config->buffered_stdio = -1;
    config->_install_importlib = 1;
    config->check_hash_pycs_mode = NULL;
    config->pathconfig_warnings = -1;
    config->_init_main = 1;
    config->_isolated_interpreter = 0;
#ifdef MS_WINDOWS
    config->legacy_windows_stdio = -1;
#endif
#ifdef Py_DEBUG
    config->use_frozen_modules = 0;
#else
    config->use_frozen_modules = 1;
#endif
    config->safe_path = 0;
    config->_is_python_build = 0;
    config->code_debug_ranges = 1;
}


static void
config_init_defaults(PyConfig *config)
{
    _PyConfig_InitCompatConfig(config);

    config->isolated = 0;
    config->use_environment = 1;
    config->site_import = 1;
    config->bytes_warning = 0;
    config->inspect = 0;
    config->interactive = 0;
    config->optimization_level = 0;
    config->parser_debug= 0;
    config->write_bytecode = 1;
    config->verbose = 0;
    config->quiet = 0;
    config->user_site_directory = 1;
    config->buffered_stdio = 1;
    config->pathconfig_warnings = 1;
#ifdef MS_WINDOWS
    config->legacy_windows_stdio = 0;
#endif
}


void
PyConfig_InitPythonConfig(PyConfig *config)
{
    config_init_defaults(config);

    config->_config_init = (int)_PyConfig_INIT_PYTHON;
    config->configure_c_stdio = 1;
    config->parse_argv = 1;
}


void
PyConfig_InitIsolatedConfig(PyConfig *config)
{
    config_init_defaults(config);

    config->_config_init = (int)_PyConfig_INIT_ISOLATED;
    config->isolated = 1;
    config->use_environment = 0;
    config->user_site_directory = 0;
    config->dev_mode = 0;
    config->install_signal_handlers = 0;
    config->use_hash_seed = 0;
    config->faulthandler = 0;
    config->tracemalloc = 0;
    config->perf_profiling = 0;
    config->safe_path = 1;
    config->pathconfig_warnings = 0;
#ifdef MS_WINDOWS
    config->legacy_windows_stdio = 0;
#endif
}


/* Copy str into *config_str (duplicate the string) */
PyStatus
PyConfig_SetString(PyConfig *config, wchar_t **config_str, const wchar_t *str)
{
    PyStatus status = _Py_PreInitializeFromConfig(config, NULL);
    if (_PyStatus_EXCEPTION(status)) {
        return status;
    }
        else {
            /* -X tracemalloc behaves as -X tracemalloc=1 */
            nframe = 1;
        }
        config->tracemalloc = nframe;
    }
    return _PyStatus_OK();
}


static PyStatus
config_init_pycache_prefix(PyConfig *config)
{
    assert(config->pycache_prefix == NULL);

    const wchar_t *xoption = config_get_xoption(config, L"pycache_prefix");
    if (xoption) {
        const wchar_t *sep = wcschr(xoption, L'=');
        if (sep && wcslen(sep) > 1) {
            config->pycache_prefix = _PyMem_RawWcsdup(sep + 1);
            if (config->pycache_prefix == NULL) {
                return _PyStatus_NO_MEMORY();
            }
        }
        else {
            // PYTHONPYCACHEPREFIX env var ignored
            // if "-X pycache_prefix=" option is used
            config->pycache_prefix = NULL;
        }
        return _PyStatus_OK();
    }
        if (_PyStatus_EXCEPTION(status)) {
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
        }
    }
    return _PyStatus_OK();
}


static const wchar_t *
config_get_stdio_errors(const PyPreConfig *preconfig)
{
    if (preconfig->utf8_mode) {
        /* UTF-8 Mode uses UTF-8/surrogateescape */
        return L"surrogateescape";
    }

#ifndef MS_WINDOWS
    const char *loc = setlocale(LC_CTYPE, NULL);
    if (loc != NULL) {
        /* surrogateescape is the default in the legacy C and POSIX locales */
        if (strcmp(loc, "C") == 0 || strcmp(loc, "POSIX") == 0) {
            return L"surrogateescape";
        }

#ifdef PY_COERCE_C_LOCALE
        /* surrogateescape is the default in locale coercion target locales */
        if (_Py_IsLocaleCoercionTarget(loc)) {
            return L"surrogateescape";
        }
#endif
    }

    return L"strict";
#else
    /* On Windows, always use surrogateescape by default */
    return L"surrogateescape";
#endif
}


// See also config_get_fs_encoding()
static PyStatus
config_get_locale_encoding(PyConfig *config, const PyPreConfig *preconfig,
                           wchar_t **locale_encoding)
{
    wchar_t *encoding;
    if (preconfig->utf8_mode) {
        encoding = _PyMem_RawWcsdup(L"utf-8");
    }
    else {
        encoding = _Py_GetLocaleEncoding();
    }
    if (encoding == NULL) {
        return _PyStatus_NO_MEMORY();
    }
    PyStatus status = PyConfig_SetString(config, locale_encoding, encoding);
    PyMem_RawFree(encoding);
    return status;
}


static PyStatus
config_init_stdio_encoding(PyConfig *config,
                           const PyPreConfig *preconfig)
{
    PyStatus status;

    /* If Py_SetStandardStreamEncoding() has been called, use its
        arguments if they are not NULL. */
    if (config->stdio_encoding == NULL && _Py_StandardStreamEncoding != NULL) {
        status = CONFIG_SET_BYTES_STR(config, &config->stdio_encoding,
                                      _Py_StandardStreamEncoding,
                                      "_Py_StandardStreamEncoding");
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    if (config->stdio_errors == NULL && _Py_StandardStreamErrors != NULL) {
        status = CONFIG_SET_BYTES_STR(config, &config->stdio_errors,
                                      _Py_StandardStreamErrors,
                                      "_Py_StandardStreamErrors");
        if (_PyStatus_EXCEPTION(status)) {
            return status;
        }
    }

    // Exit if encoding and errors are defined
    if (config->stdio_encoding != NULL && config->stdio_errors != NULL) {
        return _PyStatus_OK();
    }

    /* PYTHONIOENCODING environment variable */
    const char *opt = config_get_env(config, "PYTHONIOENCODING");
    if (opt) {
        char *pythonioencoding = _PyMem_RawStrdup(opt);
        if (pythonioencoding == NULL) {
            return _PyStatus_NO_MEMORY();
        }

        char *errors = strchr(pythonioencoding, ':');
        if (errors) {
            *errors = '\0';
            errors++;
            if (!errors[0]) {
                errors = NULL;
            }