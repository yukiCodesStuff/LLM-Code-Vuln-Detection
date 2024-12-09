    else {
        self->itself = XML_ParserCreate(encoding);
    }
    self->intern = intern;
    Py_XINCREF(self->intern);
    PyObject_GC_Track(self);
    if (self->itself == NULL) {