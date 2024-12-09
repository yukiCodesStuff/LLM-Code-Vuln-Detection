/*
 * Copyright 2007-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Siemens AG 2015-2022
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
#endif              /*@ unexpected #endif */
int f (int a,       /*@ space after fn before '(', reported unless sloppy-spc */
      int b,        /*@ hanging expr indent off by -1 */
       long I)      /*@ single-letter name 'I' */
{ int x;            /*@ code after '{' opening a block */
    int xx = 1) +   /*@ unexpected closing parenthesis */
        0L <        /*@ constant on LHS of comparison operator */
        a] -        /*@ unexpected closing bracket */
        3: *        /*@ unexpected ':' (without preceding '?') within expr */
         2,         /*@ hanging expr indent (for lines after '{') off by 1 */
        (xx         /*@0 unclosed parenthesis in expression */
         ? y        /*@0 unclosed '? (conditional expression) */
         [0;        /*@4 unclosed bracket in expression */
    /*@ blank line within local decls */
   s_type s;        /*@2 local variable declaration indent off by -1 */
   t_type t;        /*@ local variable declaration indent again off by -1 */
    /* */           /*@0 missing blank line after local decls */
   somefunc(a,      /*@2 statement indent off by -1 */
          "aligned" /*@ expr indent off by -2 accepted if sloppy-hang */ "right"
           , b,     /*@ expr indent off by -1 */
           b,       /*@ expr indent as on line above, accepted if sloppy-hang */
    ;
    ;


    ;               /*@ 2 essentially blank lines before, if !sloppy-spc */
}                   /*@ function body length > 200 lines */
#if 0               /*@0 unclosed #if */
struct t {          /*@0 unclosed brace at decl/block level */
    enum {          /*@0 unclosed brace at enum/expression level */
          v = (1    /*@0 unclosed parenthesis */
               etyp /*@0 blank line follows just before EOF, if !sloppy-spc: */
