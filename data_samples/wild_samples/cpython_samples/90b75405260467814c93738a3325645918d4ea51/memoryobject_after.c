PyTypeObject _PyManagedBuffer_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "managedbuffer",
    sizeof(_PyManagedBufferObject),
    0,
    mbuf_dealloc,                            /* tp_dealloc */
    0,                                       /* tp_vectorcall_offset */
    0,                                       /* tp_getattr */
    0,                                       /* tp_setattr */
    0,                                       /* tp_as_async */
    0,                                       /* tp_repr */
    0,                                       /* tp_as_number */
    0,                                       /* tp_as_sequence */
    0,                                       /* tp_as_mapping */
    0,                                       /* tp_hash */
    0,                                       /* tp_call */
    0,                                       /* tp_str */
    PyObject_GenericGetAttr,                 /* tp_getattro */
    0,                                       /* tp_setattro */
    0,                                       /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC, /* tp_flags */
    0,                                       /* tp_doc */
    mbuf_traverse,                           /* tp_traverse */
    mbuf_clear                               /* tp_clear */
};


/****************************************************************************/
/*                             MemoryView Object                            */
/****************************************************************************/

/* In the process of breaking reference cycles mbuf_release() can be
   called before memory_release(). */
#define BASE_INACCESSIBLE(mv) \
    (((PyMemoryViewObject *)mv)->flags&_Py_MEMORYVIEW_RELEASED || \
     ((PyMemoryViewObject *)mv)->mbuf->flags&_Py_MANAGED_BUFFER_RELEASED)

#define CHECK_RELEASED(mv) \
    if (BASE_INACCESSIBLE(mv)) {                                  \
        PyErr_SetString(PyExc_ValueError,                         \
            "operation forbidden on released memoryview object"); \
        return NULL;                                              \
    }

#define CHECK_RELEASED_INT(mv) \
    if (BASE_INACCESSIBLE(mv)) {                                  \
        PyErr_SetString(PyExc_ValueError,                         \
            "operation forbidden on released memoryview object"); \
        return -1;                                                \
    }

#define CHECK_RESTRICTED(mv) \
    if (((PyMemoryViewObject *)(mv))->flags & _Py_MEMORYVIEW_RESTRICTED) { \
        PyErr_SetString(PyExc_ValueError,                                  \
            "cannot create new view on restricted memoryview");            \
        return NULL;                                                       \
    }

#define CHECK_RESTRICTED_INT(mv) \
    if (((PyMemoryViewObject *)(mv))->flags & _Py_MEMORYVIEW_RESTRICTED) { \
        PyErr_SetString(PyExc_ValueError,                                  \
            "cannot create new view on restricted memoryview");            \
        return -1;                                                       \
    }

/* See gh-92888. These macros signal that we need to check the memoryview
   again due to possible read after frees. */
#define CHECK_RELEASED_AGAIN(mv) CHECK_RELEASED(mv)
#define CHECK_RELEASED_INT_AGAIN(mv) CHECK_RELEASED_INT(mv)

#define CHECK_LIST_OR_TUPLE(v) \
    if (!PyList_Check(v) && !PyTuple_Check(v)) { \
        PyErr_SetString(PyExc_TypeError,         \
            #v " must be a list or a tuple");    \
        return NULL;                             \
    }

#define VIEW_ADDR(mv) (&((PyMemoryViewObject *)mv)->view)

/* Check for the presence of suboffsets in the first dimension. */
#define HAVE_PTR(suboffsets, dim) (suboffsets && suboffsets[dim] >= 0)
/* Adjust ptr if suboffsets are present. */
#define ADJUST_PTR(ptr, suboffsets, dim) \
    (HAVE_PTR(suboffsets, dim) ? *((char**)ptr) + suboffsets[dim] : ptr)

/* Memoryview buffer properties */
#define MV_C_CONTIGUOUS(flags) (flags&(_Py_MEMORYVIEW_SCALAR|_Py_MEMORYVIEW_C))
#define MV_F_CONTIGUOUS(flags) \
    (flags&(_Py_MEMORYVIEW_SCALAR|_Py_MEMORYVIEW_FORTRAN))
#define MV_ANY_CONTIGUOUS(flags) \
    (flags&(_Py_MEMORYVIEW_SCALAR|_Py_MEMORYVIEW_C|_Py_MEMORYVIEW_FORTRAN))

/* Fast contiguity test. Caller must ensure suboffsets==NULL and ndim==1. */
#define MV_CONTIGUOUS_NDIM1(view) \
    ((view)->shape[0] == 1 || (view)->strides[0] == (view)->itemsize)

/* getbuffer() requests */
#define REQ_INDIRECT(flags) ((flags&PyBUF_INDIRECT) == PyBUF_INDIRECT)
#define REQ_C_CONTIGUOUS(flags) ((flags&PyBUF_C_CONTIGUOUS) == PyBUF_C_CONTIGUOUS)
#define REQ_F_CONTIGUOUS(flags) ((flags&PyBUF_F_CONTIGUOUS) == PyBUF_F_CONTIGUOUS)
#define REQ_ANY_CONTIGUOUS(flags) ((flags&PyBUF_ANY_CONTIGUOUS) == PyBUF_ANY_CONTIGUOUS)
#define REQ_STRIDES(flags) ((flags&PyBUF_STRIDES) == PyBUF_STRIDES)
#define REQ_SHAPE(flags) ((flags&PyBUF_ND) == PyBUF_ND)
#define REQ_WRITABLE(flags) (flags&PyBUF_WRITABLE)
#define REQ_FORMAT(flags) (flags&PyBUF_FORMAT)


/**************************************************************************/
/*                       Copy memoryview buffers                          */
/**************************************************************************/

/* The functions in this section take a source and a destination buffer
   with the same logical structure: format, itemsize, ndim and shape
   are identical, with ndim > 0.

   NOTE: All buffers are assumed to have PyBUF_FULL information, which
   is the case for memoryviews! */


/* Assumptions: ndim >= 1. The macro tests for a corner case that should
   perhaps be explicitly forbidden in the PEP. */
#define HAVE_SUBOFFSETS_IN_LAST_DIM(view) \
    (view->suboffsets && view->suboffsets[view->ndim-1] >= 0)

static inline int
last_dim_is_contiguous(const Py_buffer *dest, const Py_buffer *src)
{
    assert(dest->ndim > 0 && src->ndim > 0);
    return (!HAVE_SUBOFFSETS_IN_LAST_DIM(dest) &&
            !HAVE_SUBOFFSETS_IN_LAST_DIM(src) &&
            dest->strides[dest->ndim-1] == dest->itemsize &&
            src->strides[src->ndim-1] == src->itemsize);
}