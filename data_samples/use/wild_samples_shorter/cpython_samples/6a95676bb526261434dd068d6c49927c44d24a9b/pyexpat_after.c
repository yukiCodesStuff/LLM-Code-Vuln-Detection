#include "pycore_pyhash.h"        // _Py_HashSecret
#include "pycore_traceback.h"     // _PyTraceback_Add()

#include <stdbool.h>
#include <stddef.h>               // offsetof()
#include "expat.h"
#include "pyexpat.h"

                                /* NULL if not enabled */
    int buffer_size;            /* Size of buffer, in XML_Char units */
    int buffer_used;            /* Buffer units in use */
    bool reparse_deferral_enabled; /* Whether to defer reparsing of
                                   unfinished XML tokens; a de-facto cache of
                                   what Expat has the authority on, for lack
                                   of a getter API function
                                   "XML_GetReparseDeferralEnabled" in Expat
                                   2.6.0 */
    PyObject *intern;           /* Dictionary to intern strings */
    PyObject **handlers;
} xmlparseobject;


#define MAX_CHUNK_SIZE (1 << 20)

/*[clinic input]
pyexpat.xmlparser.SetReparseDeferralEnabled

    enabled: bool
    /

Enable/Disable reparse deferral; enabled by default with Expat >=2.6.0.
[clinic start generated code]*/

static PyObject *
pyexpat_xmlparser_SetReparseDeferralEnabled_impl(xmlparseobject *self,
                                                 int enabled)
/*[clinic end generated code: output=5ec539e3b63c8c49 input=021eb9e0bafc32c5]*/
{
#if XML_COMBINED_VERSION >= 20600
    XML_SetReparseDeferralEnabled(self->itself, enabled ? XML_TRUE : XML_FALSE);
    self->reparse_deferral_enabled = (bool)enabled;
#endif
    Py_RETURN_NONE;
}

/*[clinic input]
pyexpat.xmlparser.GetReparseDeferralEnabled

Retrieve reparse deferral enabled status; always returns false with Expat <2.6.0.
[clinic start generated code]*/

static PyObject *
pyexpat_xmlparser_GetReparseDeferralEnabled_impl(xmlparseobject *self)
/*[clinic end generated code: output=4e91312e88a595a8 input=54b5f11d32b20f3e]*/
{
    return PyBool_FromLong(self->reparse_deferral_enabled);
}

/*[clinic input]
pyexpat.xmlparser.Parse

    cls: defining_class
#if XML_COMBINED_VERSION >= 19505
    PYEXPAT_XMLPARSER_USEFOREIGNDTD_METHODDEF
#endif
    PYEXPAT_XMLPARSER_SETREPARSEDEFERRALENABLED_METHODDEF
    PYEXPAT_XMLPARSER_GETREPARSEDEFERRALENABLED_METHODDEF
    {NULL, NULL}  /* sentinel */
};

/* ---------- */
    self->ns_prefixes = 0;
    self->handlers = NULL;
    self->intern = Py_XNewRef(intern);
#if XML_COMBINED_VERSION >= 20600
    self->reparse_deferral_enabled = true;
#else
    self->reparse_deferral_enabled = false;
#endif

    /* namespace_separator is either NULL or contains one char + \0 */
    self->itself = XML_ParserCreate_MM(encoding, &ExpatMemoryHandler,
                                       namespace_separator);
#else
    capi->SetHashSalt = NULL;
#endif
#if XML_COMBINED_VERSION >= 20600
    capi->SetReparseDeferralEnabled = XML_SetReparseDeferralEnabled;
#else
    capi->SetReparseDeferralEnabled = NULL;
#endif

    /* export using capsule */
    PyObject *capi_object = PyCapsule_New(capi, PyExpat_CAPSULE_NAME,
                                          pyexpat_capsule_destructor);