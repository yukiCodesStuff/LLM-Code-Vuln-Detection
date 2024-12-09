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

    elementtreestate *st = self->state;

    if (EXPAT(st, SetReparseDeferralEnabled) == NULL) {
        Py_RETURN_NONE;
    }

    // NOTE: The Expat parser in the C implementation of ElementTree is not
    //       exposed to the outside; as a result we known that reparse deferral
    //       is currently enabled, or we would not even have access to function
    //       XML_SetReparseDeferralEnabled in the first place (which we checked
    //       for, a few lines up).

    EXPAT(st, SetReparseDeferralEnabled)(self->parser, XML_FALSE);

    PyObject *res = expat_parse(st, self, "", 0, XML_FALSE);

    EXPAT(st, SetReparseDeferralEnabled)(self->parser, XML_TRUE);

    return res;
}

/*[clinic input]
_elementtree.XMLParser.feed

    data: object
static PyMethodDef xmlparser_methods[] = {
    _ELEMENTTREE_XMLPARSER_FEED_METHODDEF
    _ELEMENTTREE_XMLPARSER_CLOSE_METHODDEF
    _ELEMENTTREE_XMLPARSER_FLUSH_METHODDEF
    _ELEMENTTREE_XMLPARSER__PARSE_WHOLE_METHODDEF
    _ELEMENTTREE_XMLPARSER__SETEVENTS_METHODDEF
    {NULL, NULL}
};