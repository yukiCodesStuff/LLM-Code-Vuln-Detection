                INIT_ID(mapping), \
                INIT_ID(match), \
                INIT_ID(max_length), \
                INIT_ID(maxdigits), \
                INIT_ID(maxevents), \
                INIT_ID(maxmem), \
                INIT_ID(maxsplit), \
                INIT_ID(maxvalue), \
    PyUnicode_InternInPlace(&string);
    string = &_Py_ID(max_length);
    PyUnicode_InternInPlace(&string);
    string = &_Py_ID(maxdigits);
    PyUnicode_InternInPlace(&string);
    string = &_Py_ID(maxevents);
    PyUnicode_InternInPlace(&string);
    string = &_Py_ID(maxmem);
    PyUnicode_InternInPlace(&string);
        _PyObject_Dump((PyObject *)&_Py_ID(max_length));
        Py_FatalError("immortal object has less refcnt than expected _PyObject_IMMORTAL_REFCNT");
    };
    if (Py_REFCNT((PyObject *)&_Py_ID(maxdigits)) < _PyObject_IMMORTAL_REFCNT) {
        _PyObject_Dump((PyObject *)&_Py_ID(maxdigits));
        Py_FatalError("immortal object has less refcnt than expected _PyObject_IMMORTAL_REFCNT");
    };
    if (Py_REFCNT((PyObject *)&_Py_ID(maxevents)) < _PyObject_IMMORTAL_REFCNT) {
        _PyObject_Dump((PyObject *)&_Py_ID(maxevents));
        Py_FatalError("immortal object has less refcnt than expected _PyObject_IMMORTAL_REFCNT");
    };