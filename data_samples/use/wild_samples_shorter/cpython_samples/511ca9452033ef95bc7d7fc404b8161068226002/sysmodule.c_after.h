
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

PyDoc_STRVAR(sys_set_int_max_str_digits__doc__,
"set_int_max_str_digits($module, /, maxdigits)\n"
"--\n"
"\n"
"Set the maximum string digits limit for non-binary int<->str conversions.");

#define SYS_SET_INT_MAX_STR_DIGITS_METHODDEF    \
    {"set_int_max_str_digits", _PyCFunction_CAST(sys_set_int_max_str_digits), METH_FASTCALL|METH_KEYWORDS, sys_set_int_max_str_digits__doc__},

static PyObject *
sys_set_int_max_str_digits_impl(PyObject *module, int maxdigits);

static PyObject *
sys_set_int_max_str_digits(PyObject *module, PyObject *const *args, Py_ssize_t nargs, PyObject *kwnames)
{
    PyObject *return_value = NULL;
    #if defined(Py_BUILD_CORE) && !defined(Py_BUILD_CORE_MODULE)

    #define NUM_KEYWORDS 1
    static struct {
        PyGC_Head _this_is_not_used;
        PyObject_VAR_HEAD
        PyObject *ob_item[NUM_KEYWORDS];
    } _kwtuple = {
        .ob_base = PyVarObject_HEAD_INIT(&PyTuple_Type, NUM_KEYWORDS)
        .ob_item = { &_Py_ID(maxdigits), },
    };
    #undef NUM_KEYWORDS
    #define KWTUPLE (&_kwtuple.ob_base.ob_base)

    #else  // !Py_BUILD_CORE
    #  define KWTUPLE NULL
    #endif  // !Py_BUILD_CORE

    static const char * const _keywords[] = {"maxdigits", NULL};
    static _PyArg_Parser _parser = {
        .keywords = _keywords,
        .fname = "set_int_max_str_digits",
        .kwtuple = KWTUPLE,
    };
    #undef KWTUPLE
    PyObject *argsbuf[1];
    int maxdigits;

    args = _PyArg_UnpackKeywords(args, nargs, NULL, kwnames, &_parser, 1, 1, 0, argsbuf);
    if (!args) {
        goto exit;
    }
    maxdigits = _PyLong_AsInt(args[0]);
    if (maxdigits == -1 && PyErr_Occurred()) {
        goto exit;
    }
    return_value = sys_set_int_max_str_digits_impl(module, maxdigits);

exit:
    return return_value;
}

PyDoc_STRVAR(sys_getrefcount__doc__,
"getrefcount($module, object, /)\n"
"--\n"
"\n"
#ifndef SYS_GETANDROIDAPILEVEL_METHODDEF
    #define SYS_GETANDROIDAPILEVEL_METHODDEF
#endif /* !defined(SYS_GETANDROIDAPILEVEL_METHODDEF) */
/*[clinic end generated code: output=15318cdd96b62b06 input=a9049054013a1b77]*/