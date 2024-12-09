#include "pycore_pyhash.h"        // _Py_HashSecret
#include "pycore_traceback.h"     // _PyTraceback_Add()

#include <stddef.h>               // offsetof()
#include "expat.h"
#include "pyexpat.h"

                                /* NULL if not enabled */
    int buffer_size;            /* Size of buffer, in XML_Char units */
    int buffer_used;            /* Buffer units in use */
    PyObject *intern;           /* Dictionary to intern strings */
    PyObject **handlers;
} xmlparseobject;


#define MAX_CHUNK_SIZE (1 << 20)

/*[clinic input]
pyexpat.xmlparser.Parse

    cls: defining_class
#if XML_COMBINED_VERSION >= 19505
    PYEXPAT_XMLPARSER_USEFOREIGNDTD_METHODDEF
#endif
    {NULL, NULL}  /* sentinel */
};

/* ---------- */
    self->ns_prefixes = 0;
    self->handlers = NULL;
    self->intern = Py_XNewRef(intern);

    /* namespace_separator is either NULL or contains one char + \0 */
    self->itself = XML_ParserCreate_MM(encoding, &ExpatMemoryHandler,
                                       namespace_separator);
#else
    capi->SetHashSalt = NULL;
#endif

    /* export using capsule */
    PyObject *capi_object = PyCapsule_New(capi, PyExpat_CAPSULE_NAME,
                                          pyexpat_capsule_destructor);