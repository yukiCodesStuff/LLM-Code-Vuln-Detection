    struct pythreads {
        uint64_t next_unique_id;
        /* The linked list of threads, newest first. */
        PyThreadState *head;
        /* Used in Modules/_threadmodule.c. */
        long count;
        /* Support for runtime thread stack size tuning.
           A value of 0 means using the platform's default stack size
           or the size specified by the THREAD_STACK_SIZE macro. */
        /* Used in Python/thread.c. */
        size_t stacksize;
    } threads;

    /* Reference to the _PyRuntime global variable. This field exists
       to not have to pass runtime in addition to tstate to a function.
       Get runtime from tstate: tstate->interp->runtime. */
    struct pyruntimestate *runtime;

    int64_t id;
    int64_t id_refcount;
    int requires_idref;
    PyThread_type_lock id_mutex;

    /* Has been initialized to a safe state.

       In order to be effective, this must be set to 0 during or right
       after allocation. */
    int _initialized;
    int finalizing;

    /* Was this interpreter statically allocated? */
    bool _static;

    struct _ceval_state ceval;
    struct _gc_runtime_state gc;

    // sys.modules dictionary
    PyObject *modules;
    PyObject *modules_by_index;
    // Dictionary of the sys module
    PyObject *sysdict;
    // Dictionary of the builtins module
    PyObject *builtins;
    // importlib module
    PyObject *importlib;
    // override for config->use_frozen_modules (for tests)
    // (-1: "off", 1: "on", 0: no override)
    int override_frozen_modules;

    PyObject *codec_search_path;
    PyObject *codec_search_cache;
    PyObject *codec_error_registry;
    int codecs_initialized;

    PyConfig config;
#ifdef HAVE_DLOPEN
    int dlopenflags;
#endif

    PyObject *dict;  /* Stores per-interpreter state */

    PyObject *builtins_copy;
    PyObject *import_func;
    // Initialized to _PyEval_EvalFrameDefault().
    _PyFrameEvalFunction eval_frame;

    Py_ssize_t co_extra_user_count;
    freefunc co_extra_freefuncs[MAX_CO_EXTRA_USERS];

#ifdef HAVE_FORK
    PyObject *before_forkers;
    PyObject *after_forkers_parent;
    PyObject *after_forkers_child;
#endif

    struct _warnings_runtime_state warnings;
    struct atexit_state atexit;

    PyObject *audit_hooks;

    struct _Py_unicode_state unicode;
    struct _Py_float_state float_state;
    /* Using a cache is very effective since typically only a single slice is
       created and then deleted again. */
    PySliceObject *slice_cache;

    struct _Py_tuple_state tuple;
    struct _Py_list_state list;
    struct _Py_dict_state dict_state;
    struct _Py_async_gen_state async_gen;
    struct _Py_context_state context;
    struct _Py_exc_state exc_state;

    struct ast_state ast;
    struct types_state types;
    struct callable_cache callable_cache;

    /* The following fields are here to avoid allocation during init.
       The data is exposed through PyInterpreterState pointer fields.
       These fields should not be accessed directly outside of init.

       All other PyInterpreterState pointer fields are populated when
       needed and default to NULL.

       For now there are some exceptions to that rule, which require
       allocation during init.  These will be addressed on a case-by-case
       basis.  Also see _PyRuntimeState regarding the various mutex fields.
       */

    /* the initial PyInterpreterState.threads.head */
    PyThreadState _initial_thread;
};


/* other API */

extern void _PyInterpreterState_ClearModules(PyInterpreterState *interp);
extern void _PyInterpreterState_Clear(PyThreadState *tstate);


/* cross-interpreter data registry */

/* For now we use a global registry of shareable classes.  An
   alternative would be to add a tp_* slot for a class's
   crossinterpdatafunc. It would be simpler and more efficient. */

struct _xidregitem;

struct _xidregitem {
    PyTypeObject *cls;
    crossinterpdatafunc getdata;
    struct _xidregitem *next;
};

PyAPI_FUNC(PyInterpreterState*) _PyInterpreterState_LookUpID(int64_t);

PyAPI_FUNC(int) _PyInterpreterState_IDInitref(PyInterpreterState *);
PyAPI_FUNC(int) _PyInterpreterState_IDIncref(PyInterpreterState *);
PyAPI_FUNC(void) _PyInterpreterState_IDDecref(PyInterpreterState *);

#ifdef __cplusplus
}