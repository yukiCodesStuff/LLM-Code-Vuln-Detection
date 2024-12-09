{
    return _elementtree_XMLParser_close_impl(self);
}

PyDoc_STRVAR(_elementtree_XMLParser_feed__doc__,
"feed($self, data, /)\n"
"--\n"
"\n");

#define _ELEMENTTREE_XMLPARSER_FEED_METHODDEF    \
    {"feed", (PyCFunction)_elementtree_XMLParser_feed, METH_O, _elementtree_XMLParser_feed__doc__},

PyDoc_STRVAR(_elementtree_XMLParser__parse_whole__doc__,
"_parse_whole($self, file, /)\n"
"--\n"
"\n");

#define _ELEMENTTREE_XMLPARSER__PARSE_WHOLE_METHODDEF    \
    {"_parse_whole", (PyCFunction)_elementtree_XMLParser__parse_whole, METH_O, _elementtree_XMLParser__parse_whole__doc__},

PyDoc_STRVAR(_elementtree_XMLParser__setevents__doc__,
"_setevents($self, events_queue, events_to_report=None, /)\n"
"--\n"
"\n");

#define _ELEMENTTREE_XMLPARSER__SETEVENTS_METHODDEF    \
    {"_setevents", _PyCFunction_CAST(_elementtree_XMLParser__setevents), METH_FASTCALL, _elementtree_XMLParser__setevents__doc__},

static PyObject *
_elementtree_XMLParser__setevents_impl(XMLParserObject *self,
                                       PyObject *events_queue,
                                       PyObject *events_to_report);

static PyObject *
_elementtree_XMLParser__setevents(XMLParserObject *self, PyObject *const *args, Py_ssize_t nargs)
{
    PyObject *return_value = NULL;
    PyObject *events_queue;
    PyObject *events_to_report = Py_None;

    if (!_PyArg_CheckPositional("_setevents", nargs, 1, 2)) {
        goto exit;
    }
    if (nargs < 2) {
        goto skip_optional;
    }
    events_to_report = args[1];
skip_optional:
    return_value = _elementtree_XMLParser__setevents_impl(self, events_queue, events_to_report);

exit:
    return return_value;
}
/*[clinic end generated code: output=218ec9e6a889f796 input=a9049054013a1b77]*/