    else {
        self->itself = XML_ParserCreate(encoding);
    }
    self->intern = intern;
    Py_XINCREF(self->intern);
#ifdef Py_TPFLAGS_HAVE_GC
    PyObject_GC_Track(self);