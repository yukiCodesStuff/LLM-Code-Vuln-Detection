    return _elementtree_XMLParser_close_impl(self);
}

PyDoc_STRVAR(_elementtree_XMLParser_flush__doc__,
"flush($self, /)\n"
"--\n"
"\n");

#define _ELEMENTTREE_XMLPARSER_FLUSH_METHODDEF    \
    {"flush", (PyCFunction)_elementtree_XMLParser_flush, METH_NOARGS, _elementtree_XMLParser_flush__doc__},

static PyObject *
_elementtree_XMLParser_flush_impl(XMLParserObject *self);

static PyObject *
_elementtree_XMLParser_flush(XMLParserObject *self, PyObject *Py_UNUSED(ignored))
{
    return _elementtree_XMLParser_flush_impl(self);
}

PyDoc_STRVAR(_elementtree_XMLParser_feed__doc__,
"feed($self, data, /)\n"
"--\n"
"\n");
exit:
    return return_value;
}
/*[clinic end generated code: output=aed9f53eeb0404e0 input=a9049054013a1b77]*/