    else {
        self->itself = XML_ParserCreate(encoding);
    }
    XML_SetHashSalt(self->itself,
                    (unsigned long)_Py_HashSecret.prefix);
    self->intern = intern;
    Py_XINCREF(self->intern);
    PyObject_GC_Track(self);
    if (self->itself == NULL) {