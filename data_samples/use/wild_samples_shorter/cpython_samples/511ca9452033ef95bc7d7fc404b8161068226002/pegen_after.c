#include <Python.h>
#include "pycore_ast.h"           // _PyAST_Validate(),
#include "pycore_pystate.h"       // _PyThreadState_GET()
#include <errcode.h>

#include "tokenizer.h"
#include "pegen.h"

    if (c == NULL) {
        p->error_indicator = 1;
        PyThreadState *tstate = _PyThreadState_GET();
        // The only way a ValueError should happen in _this_ code is via
        // PyLong_FromString hitting a length limit.
        if (tstate->curexc_type == PyExc_ValueError &&
            tstate->curexc_value != NULL) {
            PyObject *type, *value, *tb;
            // This acts as PyErr_Clear() as we're replacing curexc.
            PyErr_Fetch(&type, &value, &tb);
            Py_XDECREF(tb);
            Py_DECREF(type);
            /* Intentionally omitting columns to avoid a wall of 1000s of '^'s
             * on the error message. Nobody is going to overlook their huge
             * numeric literal once given the line. */
            RAISE_ERROR_KNOWN_LOCATION(
                p, PyExc_SyntaxError,
                t->lineno, -1 /* col_offset */,
                t->end_lineno, -1 /* end_col_offset */,
                "%S - Consider hexadecimal for huge integer literals "
                "to avoid decimal conversion limits.",
                value);
            Py_DECREF(value);
        }
        return NULL;
    }

    if (_PyArena_AddPyObject(p->arena, c) < 0) {