#define IS_SMALL_INT(ival) (-_PY_NSMALLNEGINTS <= (ival) && (ival) < _PY_NSMALLPOSINTS)
#define IS_SMALL_UINT(ival) ((ival) < _PY_NSMALLPOSINTS)

#define _MAX_STR_DIGITS_ERROR_FMT "Exceeds the limit (%d) for integer string conversion: value has %zd digits"

static inline void
_Py_DECREF_INT(PyLongObject *op)
{
    assert(PyLong_CheckExact(op));
        tenpow *= 10;
        strlen++;
    }
    if (strlen > _PY_LONG_MAX_STR_DIGITS_THRESHOLD) {
        PyInterpreterState *interp = _PyInterpreterState_GET();
        int max_str_digits = interp->int_max_str_digits;
        Py_ssize_t strlen_nosign = strlen - negative;
        if ((max_str_digits > 0) && (strlen_nosign > max_str_digits)) {
            Py_DECREF(scratch);
            PyErr_Format(PyExc_ValueError, _MAX_STR_DIGITS_ERROR_FMT,
                         max_str_digits, strlen_nosign);
            return -1;
        }
    }
    if (writer) {
        if (_PyUnicodeWriter_Prepare(writer, strlen, '9') == -1) {
            Py_DECREF(scratch);
            return -1;

    start = str;
    if ((base & (base - 1)) == 0) {
        /* binary bases are not limited by int_max_str_digits */
        int res = long_from_binary_base(&str, base, &z);
        if (res < 0) {
            /* Syntax error. */
            goto onError;
            goto onError;
        }

        /* Limit the size to avoid excessive computation attacks. */
        if (digits > _PY_LONG_MAX_STR_DIGITS_THRESHOLD) {
            PyInterpreterState *interp = _PyInterpreterState_GET();
            int max_str_digits = interp->int_max_str_digits;
            if ((max_str_digits > 0) && (digits > max_str_digits)) {
                PyErr_Format(PyExc_ValueError, _MAX_STR_DIGITS_ERROR_FMT,
                             max_str_digits, digits);
                return NULL;
            }
        }

        /* Create an int object that can contain the largest possible
         * integer with this base and length.  Note that there's no
         * need to initialize z->ob_digit -- no slot is read up before
         * being stored into.
        }
        return PyLong_FromLong(0L);
    }
    /* default base and limit, forward to standard implementation */
    if (obase == NULL)
        return PyNumber_Long(x);

    base = PyNumber_AsSsize_t(obase, NULL);
static PyStructSequence_Field int_info_fields[] = {
    {"bits_per_digit", "size of a digit in bits"},
    {"sizeof_digit", "size in bytes of the C type used to represent a digit"},
    {"default_max_str_digits", "maximum string conversion digits limitation"},
    {"str_digits_check_threshold", "minimum positive value for int_max_str_digits"},
    {NULL, NULL}
};

static PyStructSequence_Desc int_info_desc = {
    "sys.int_info",   /* name */
    int_info__doc__,  /* doc */
    int_info_fields,  /* fields */
    4                 /* number of fields */
};

PyObject *
PyLong_GetInfo(void)
                              PyLong_FromLong(PyLong_SHIFT));
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(sizeof(digit)));
    /*
     * The following two fields were added after investigating uses of
     * sys.int_info in the wild: Exceedingly rarely used. The ONLY use found was
     * numba using sys.int_info.bits_per_digit as attribute access rather than
     * sequence unpacking. Cython and sympy also refer to sys.int_info but only
     * as info for debugging. No concern about adding these in a backport.
     */
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(_PY_LONG_DEFAULT_MAX_STR_DIGITS));
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(_PY_LONG_MAX_STR_DIGITS_THRESHOLD));
    if (PyErr_Occurred()) {
        Py_CLEAR(int_info);
        return NULL;
    }
            return _PyStatus_ERR("can't init int info type");
        }
    }
    interp->int_max_str_digits = _Py_global_config_int_max_str_digits;
    if (interp->int_max_str_digits == -1) {
        interp->int_max_str_digits = _PY_LONG_DEFAULT_MAX_STR_DIGITS;
    }

    return _PyStatus_OK();
}
