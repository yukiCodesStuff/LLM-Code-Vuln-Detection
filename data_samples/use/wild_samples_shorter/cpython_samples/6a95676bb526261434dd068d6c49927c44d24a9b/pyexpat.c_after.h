#endif
#include "pycore_modsupport.h"    // _PyArg_UnpackKeywords()

PyDoc_STRVAR(pyexpat_xmlparser_SetReparseDeferralEnabled__doc__,
"SetReparseDeferralEnabled($self, enabled, /)\n"
"--\n"
"\n"
"Enable/Disable reparse deferral; enabled by default with Expat >=2.6.0.");

#define PYEXPAT_XMLPARSER_SETREPARSEDEFERRALENABLED_METHODDEF    \
    {"SetReparseDeferralEnabled", (PyCFunction)pyexpat_xmlparser_SetReparseDeferralEnabled, METH_O, pyexpat_xmlparser_SetReparseDeferralEnabled__doc__},

static PyObject *
pyexpat_xmlparser_SetReparseDeferralEnabled_impl(xmlparseobject *self,
                                                 int enabled);

static PyObject *
pyexpat_xmlparser_SetReparseDeferralEnabled(xmlparseobject *self, PyObject *arg)
{
    PyObject *return_value = NULL;
    int enabled;

    enabled = PyObject_IsTrue(arg);
    if (enabled < 0) {
        goto exit;
    }
    return_value = pyexpat_xmlparser_SetReparseDeferralEnabled_impl(self, enabled);

exit:
    return return_value;
}

PyDoc_STRVAR(pyexpat_xmlparser_GetReparseDeferralEnabled__doc__,
"GetReparseDeferralEnabled($self, /)\n"
"--\n"
"\n"
"Retrieve reparse deferral enabled status; always returns false with Expat <2.6.0.");

#define PYEXPAT_XMLPARSER_GETREPARSEDEFERRALENABLED_METHODDEF    \
    {"GetReparseDeferralEnabled", (PyCFunction)pyexpat_xmlparser_GetReparseDeferralEnabled, METH_NOARGS, pyexpat_xmlparser_GetReparseDeferralEnabled__doc__},

static PyObject *
pyexpat_xmlparser_GetReparseDeferralEnabled_impl(xmlparseobject *self);

static PyObject *
pyexpat_xmlparser_GetReparseDeferralEnabled(xmlparseobject *self, PyObject *Py_UNUSED(ignored))
{
    return pyexpat_xmlparser_GetReparseDeferralEnabled_impl(self);
}

PyDoc_STRVAR(pyexpat_xmlparser_Parse__doc__,
"Parse($self, data, isfinal=False, /)\n"
"--\n"
"\n"
#ifndef PYEXPAT_XMLPARSER_USEFOREIGNDTD_METHODDEF
    #define PYEXPAT_XMLPARSER_USEFOREIGNDTD_METHODDEF
#endif /* !defined(PYEXPAT_XMLPARSER_USEFOREIGNDTD_METHODDEF) */
/*[clinic end generated code: output=892e48e41f9b6e4b input=a9049054013a1b77]*/