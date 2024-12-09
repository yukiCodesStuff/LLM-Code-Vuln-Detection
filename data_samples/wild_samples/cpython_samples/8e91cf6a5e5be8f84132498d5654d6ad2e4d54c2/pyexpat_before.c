    else {
        self->itself = XML_ParserCreate(encoding);
    }
    self->intern = intern;
    Py_XINCREF(self->intern);
#ifdef Py_TPFLAGS_HAVE_GC
    PyObject_GC_Track(self);
#else
    PyObject_GC_Init(self);
#endif
    if (self->itself == NULL) {
        PyErr_SetString(PyExc_RuntimeError,
                        "XML_ParserCreate failed");
        Py_DECREF(self);
        return NULL;
    }
    XML_SetUserData(self->itself, (void *)self);
    XML_SetUnknownEncodingHandler(self->itself,
                  (XML_UnknownEncodingHandler) PyUnknownEncodingHandler, NULL);

    for (i = 0; handler_info[i].name != NULL; i++)
        /* do nothing */;

    self->handlers = malloc(sizeof(PyObject *) * i);
    if (!self->handlers) {
        Py_DECREF(self);
        return PyErr_NoMemory();
    }
    clear_handlers(self, 1);

    return (PyObject*)self;
}


static void
xmlparse_dealloc(xmlparseobject *self)
{
    int i;
#ifdef Py_TPFLAGS_HAVE_GC
    PyObject_GC_UnTrack(self);
#else
    PyObject_GC_Fini(self);
#endif
    if (self->itself != NULL)
        XML_ParserFree(self->itself);
    self->itself = NULL;

    if (self->handlers != NULL) {
        PyObject *temp;
        for (i = 0; handler_info[i].name != NULL; i++) {
            temp = self->handlers[i];
            self->handlers[i] = NULL;
            Py_XDECREF(temp);
        }