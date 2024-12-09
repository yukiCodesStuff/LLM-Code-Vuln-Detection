/* Assumptions: ndim >= 1. The macro tests for a corner case that should
   perhaps be explicitly forbidden in the PEP. */
#define HAVE_SUBOFFSETS_IN_LAST_DIM(view) \
    (view->suboffsets && view->suboffsets[dest->ndim-1] >= 0)

static inline int
last_dim_is_contiguous(const Py_buffer *dest, const Py_buffer *src)
{