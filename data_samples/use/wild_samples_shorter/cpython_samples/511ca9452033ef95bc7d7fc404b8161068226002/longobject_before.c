#define IS_SMALL_INT(ival) (-_PY_NSMALLNEGINTS <= (ival) && (ival) < _PY_NSMALLPOSINTS)
#define IS_SMALL_UINT(ival) ((ival) < _PY_NSMALLPOSINTS)

static inline void
_Py_DECREF_INT(PyLongObject *op)
{
    assert(PyLong_CheckExact(op));
        tenpow *= 10;
        strlen++;
    }
    if (writer) {
        if (_PyUnicodeWriter_Prepare(writer, strlen, '9') == -1) {
            Py_DECREF(scratch);
            return -1;

    start = str;
    if ((base & (base - 1)) == 0) {
        int res = long_from_binary_base(&str, base, &z);
        if (res < 0) {
            /* Syntax error. */
            goto onError;
            goto onError;
        }

        /* Create an int object that can contain the largest possible
         * integer with this base and length.  Note that there's no
         * need to initialize z->ob_digit -- no slot is read up before
         * being stored into.
        }
        return PyLong_FromLong(0L);
    }
    if (obase == NULL)
        return PyNumber_Long(x);

    base = PyNumber_AsSsize_t(obase, NULL);
static PyStructSequence_Field int_info_fields[] = {
    {"bits_per_digit", "size of a digit in bits"},
    {"sizeof_digit", "size in bytes of the C type used to represent a digit"},
    {NULL, NULL}
};

static PyStructSequence_Desc int_info_desc = {
    "sys.int_info",   /* name */
    int_info__doc__,  /* doc */
    int_info_fields,  /* fields */
    2                 /* number of fields */
};

PyObject *
PyLong_GetInfo(void)
                              PyLong_FromLong(PyLong_SHIFT));
    PyStructSequence_SET_ITEM(int_info, field++,
                              PyLong_FromLong(sizeof(digit)));
    if (PyErr_Occurred()) {
        Py_CLEAR(int_info);
        return NULL;
    }
            return _PyStatus_ERR("can't init int info type");
        }
    }

    return _PyStatus_OK();
}
