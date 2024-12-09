#include <Python.h>
#include "pycore_ast.h"           // _PyAST_Validate(),
#include <errcode.h>

#include "tokenizer.h"
#include "pegen.h"

    if (c == NULL) {
        p->error_indicator = 1;
        return NULL;
    }

    if (_PyArena_AddPyObject(p->arena, c) < 0) {