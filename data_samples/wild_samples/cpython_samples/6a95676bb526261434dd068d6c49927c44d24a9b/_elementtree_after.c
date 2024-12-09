    else {
        return res;
    }
}

/*[clinic input]
_elementtree.XMLParser.flush

[clinic start generated code]*/

static PyObject *
_elementtree_XMLParser_flush_impl(XMLParserObject *self)
/*[clinic end generated code: output=42fdb8795ca24509 input=effbecdb28715949]*/
{
    if (!_check_xmlparser(self)) {
        return NULL;
    }
static PyMethodDef xmlparser_methods[] = {
    _ELEMENTTREE_XMLPARSER_FEED_METHODDEF
    _ELEMENTTREE_XMLPARSER_CLOSE_METHODDEF
    _ELEMENTTREE_XMLPARSER_FLUSH_METHODDEF
    _ELEMENTTREE_XMLPARSER__PARSE_WHOLE_METHODDEF
    _ELEMENTTREE_XMLPARSER__SETEVENTS_METHODDEF
    {NULL, NULL}
};

static PyType_Slot xmlparser_slots[] = {
    {Py_tp_dealloc, xmlparser_dealloc},
    {Py_tp_traverse, xmlparser_gc_traverse},
    {Py_tp_clear, xmlparser_gc_clear},
    {Py_tp_methods, xmlparser_methods},
    {Py_tp_members, xmlparser_members},
    {Py_tp_getset, xmlparser_getsetlist},
    {Py_tp_init, _elementtree_XMLParser___init__},
    {Py_tp_alloc, PyType_GenericAlloc},
    {Py_tp_new, xmlparser_new},
    {0, NULL},
};

static PyType_Spec xmlparser_spec = {
    .name = "xml.etree.ElementTree.XMLParser",
    .basicsize = sizeof(XMLParserObject),
    .flags = (Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC |
              Py_TPFLAGS_IMMUTABLETYPE),
    .slots = xmlparser_slots,
};

/* ==================================================================== */
/* python module interface */

static PyMethodDef _functions[] = {
    {"SubElement", _PyCFunction_CAST(subelement), METH_VARARGS | METH_KEYWORDS},
    _ELEMENTTREE__SET_FACTORIES_METHODDEF
    {NULL, NULL}
};

#define CREATE_TYPE(module, type, spec) \
do {                                                                     \
    if (type != NULL) {                                                  \
        break;                                                           \
    }                                                                    \
    type = (PyTypeObject *)PyType_FromModuleAndSpec(module, spec, NULL); \
    if (type == NULL) {                                                  \
        goto error;                                                      \
    }                                                                    \
} while (0)

static int
module_exec(PyObject *m)
{
    elementtreestate *st = get_elementtree_state(m);

    /* Initialize object types */
    CREATE_TYPE(m, st->ElementIter_Type, &elementiter_spec);
    CREATE_TYPE(m, st->TreeBuilder_Type, &treebuilder_spec);
    CREATE_TYPE(m, st->Element_Type, &element_spec);
    CREATE_TYPE(m, st->XMLParser_Type, &xmlparser_spec);

    st->deepcopy_obj = _PyImport_GetModuleAttrString("copy", "deepcopy");
    if (st->deepcopy_obj == NULL) {
        goto error;
    }

    assert(!PyErr_Occurred());
    if (!(st->elementpath_obj = PyImport_ImportModule("xml.etree.ElementPath")))
        goto error;

    /* link against pyexpat */
    if (!(st->expat_capsule = _PyImport_GetModuleAttrString("pyexpat", "expat_CAPI")))
        goto error;
    if (!(st->expat_capi = PyCapsule_GetPointer(st->expat_capsule, PyExpat_CAPSULE_NAME)))
        goto error;
    if (st->expat_capi) {
        /* check that it's usable */
        if (strcmp(st->expat_capi->magic, PyExpat_CAPI_MAGIC) != 0 ||
            (size_t)st->expat_capi->size < sizeof(struct PyExpat_CAPI) ||
            st->expat_capi->MAJOR_VERSION != XML_MAJOR_VERSION ||
            st->expat_capi->MINOR_VERSION != XML_MINOR_VERSION ||
            st->expat_capi->MICRO_VERSION != XML_MICRO_VERSION) {
            PyErr_SetString(PyExc_ImportError,
                            "pyexpat version is incompatible");
            goto error;
        }
    } else {
        goto error;
    }

    st->str_append = PyUnicode_InternFromString("append");
    if (st->str_append == NULL) {
        goto error;
    }
    st->str_find = PyUnicode_InternFromString("find");
    if (st->str_find == NULL) {
        goto error;
    }
    st->str_findall = PyUnicode_InternFromString("findall");
    if (st->str_findall == NULL) {
        goto error;
    }
    st->str_findtext = PyUnicode_InternFromString("findtext");
    if (st->str_findtext == NULL) {
        goto error;
    }
    st->str_iterfind = PyUnicode_InternFromString("iterfind");
    if (st->str_iterfind == NULL) {
        goto error;
    }
    st->str_tail = PyUnicode_InternFromString("tail");
    if (st->str_tail == NULL) {
        goto error;
    }
    st->str_text = PyUnicode_InternFromString("text");
    if (st->str_text == NULL) {
        goto error;
    }
    st->str_doctype = PyUnicode_InternFromString("doctype");
    if (st->str_doctype == NULL) {
        goto error;
    }
    st->parseerror_obj = PyErr_NewException(
        "xml.etree.ElementTree.ParseError", PyExc_SyntaxError, NULL
        );
    if (PyModule_AddObjectRef(m, "ParseError", st->parseerror_obj) < 0) {
        goto error;
    }

    PyTypeObject *types[] = {
        st->Element_Type,
        st->TreeBuilder_Type,
        st->XMLParser_Type
    };

    for (size_t i = 0; i < Py_ARRAY_LENGTH(types); i++) {
        if (PyModule_AddType(m, types[i]) < 0) {
            goto error;
        }
    }

    return 0;

error:
    return -1;
}

static struct PyModuleDef_Slot elementtree_slots[] = {
    {Py_mod_exec, module_exec},
    {Py_mod_multiple_interpreters, Py_MOD_PER_INTERPRETER_GIL_SUPPORTED},
    {0, NULL},
};

static struct PyModuleDef elementtreemodule = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "_elementtree",
    .m_size = sizeof(elementtreestate),
    .m_methods = _functions,
    .m_slots = elementtree_slots,
    .m_traverse = elementtree_traverse,
    .m_clear = elementtree_clear,
    .m_free = elementtree_free,
};

PyMODINIT_FUNC
PyInit__elementtree(void)
{
    return PyModuleDef_Init(&elementtreemodule);
}