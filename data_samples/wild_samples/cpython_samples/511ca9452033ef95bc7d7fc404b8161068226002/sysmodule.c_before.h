    if (flag == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_mdebug_impl(module, flag);

exit:
    return return_value;
}

#endif /* defined(USE_MALLOPT) */

PyDoc_STRVAR(sys_getrefcount__doc__,
"getrefcount($module, object, /)\n"
"--\n"
"\n"
"Return the reference count of object.\n"
"\n"
"The count returned is generally one higher than you might expect,\n"
"because it includes the (temporary) reference as an argument to\n"
"getrefcount().");

#define SYS_GETREFCOUNT_METHODDEF    \
    {"getrefcount", (PyCFunction)sys_getrefcount, METH_O, sys_getrefcount__doc__},

static Py_ssize_t
sys_getrefcount_impl(PyObject *module, PyObject *object);

static PyObject *
sys_getrefcount(PyObject *module, PyObject *object)
{
    PyObject *return_value = NULL;
    Py_ssize_t _return_value;

    _return_value = sys_getrefcount_impl(module, object);
    if ((_return_value == -1) && PyErr_Occurred()) {
        goto exit;
    }
    return_value = PyLong_FromSsize_t(_return_value);

exit:
    return return_value;
}
{
    return sys_is_stack_trampoline_active_impl(module);
}

#ifndef SYS_GETWINDOWSVERSION_METHODDEF
    #define SYS_GETWINDOWSVERSION_METHODDEF
#endif /* !defined(SYS_GETWINDOWSVERSION_METHODDEF) */

#ifndef SYS__ENABLELEGACYWINDOWSFSENCODING_METHODDEF
    #define SYS__ENABLELEGACYWINDOWSFSENCODING_METHODDEF
#endif /* !defined(SYS__ENABLELEGACYWINDOWSFSENCODING_METHODDEF) */

#ifndef SYS_SETDLOPENFLAGS_METHODDEF
    #define SYS_SETDLOPENFLAGS_METHODDEF
#endif /* !defined(SYS_SETDLOPENFLAGS_METHODDEF) */

#ifndef SYS_GETDLOPENFLAGS_METHODDEF
    #define SYS_GETDLOPENFLAGS_METHODDEF
#endif /* !defined(SYS_GETDLOPENFLAGS_METHODDEF) */

#ifndef SYS_MDEBUG_METHODDEF
    #define SYS_MDEBUG_METHODDEF
#endif /* !defined(SYS_MDEBUG_METHODDEF) */

#ifndef SYS_GETTOTALREFCOUNT_METHODDEF
    #define SYS_GETTOTALREFCOUNT_METHODDEF
#endif /* !defined(SYS_GETTOTALREFCOUNT_METHODDEF) */

#ifndef SYS__STATS_ON_METHODDEF
    #define SYS__STATS_ON_METHODDEF
#endif /* !defined(SYS__STATS_ON_METHODDEF) */

#ifndef SYS__STATS_OFF_METHODDEF
    #define SYS__STATS_OFF_METHODDEF
#endif /* !defined(SYS__STATS_OFF_METHODDEF) */

#ifndef SYS__STATS_CLEAR_METHODDEF
    #define SYS__STATS_CLEAR_METHODDEF
#endif /* !defined(SYS__STATS_CLEAR_METHODDEF) */

#ifndef SYS__STATS_DUMP_METHODDEF
    #define SYS__STATS_DUMP_METHODDEF
#endif /* !defined(SYS__STATS_DUMP_METHODDEF) */

#ifndef SYS_GETANDROIDAPILEVEL_METHODDEF
    #define SYS_GETANDROIDAPILEVEL_METHODDEF
#endif /* !defined(SYS_GETANDROIDAPILEVEL_METHODDEF) */
/*[clinic end generated code: output=43b44240211afe95 input=a9049054013a1b77]*/