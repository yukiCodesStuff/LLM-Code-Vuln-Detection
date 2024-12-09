    if (flag == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_mdebug_impl(module, flag);

exit:
    return return_value;
}

#endif /* defined(USE_MALLOPT) */

PyDoc_STRVAR(sys_get_int_max_str_digits__doc__,
"get_int_max_str_digits($module, /)\n"
"--\n"
"\n"
"Set the maximum string digits limit for non-binary int<->str conversions.");

#define SYS_GET_INT_MAX_STR_DIGITS_METHODDEF    \
    {"get_int_max_str_digits", (PyCFunction)sys_get_int_max_str_digits, METH_NOARGS, sys_get_int_max_str_digits__doc__},

static PyObject *
sys_get_int_max_str_digits_impl(PyObject *module);

static PyObject *
sys_get_int_max_str_digits(PyObject *module, PyObject *Py_UNUSED(ignored))
{
    return sys_get_int_max_str_digits_impl(module);
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
/*[clinic end generated code: output=79228e569529129c input=a9049054013a1b77]*/