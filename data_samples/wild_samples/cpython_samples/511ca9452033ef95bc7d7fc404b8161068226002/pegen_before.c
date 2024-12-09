#include <Python.h>
#include "pycore_ast.h"           // _PyAST_Validate(),
#include <errcode.h>

#include "tokenizer.h"
#include "pegen.h"

// Internal parser functions

asdl_stmt_seq*
_PyPegen_interactive_exit(Parser *p)
{
    if (p->errcode) {
        *(p->errcode) = E_EOF;
    }
    return NULL;
}
    if (c == NULL) {
        p->error_indicator = 1;
        return NULL;
    }