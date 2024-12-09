/*
 * Copyright 1999-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* X509 v3 extension utilities */

#include <stdio.h>
#include "internal/cryptlib.h"
#include <openssl/conf.h>
#include <openssl/x509v3.h>

#include "ext_dat.h"

static STACK_OF(X509V3_EXT_METHOD) *ext_list = NULL;

static int ext_cmp(const X509V3_EXT_METHOD *const *a,
                   const X509V3_EXT_METHOD *const *b);
static void ext_list_free(X509V3_EXT_METHOD *ext);

int X509V3_EXT_add(X509V3_EXT_METHOD *ext)
{
    if (ext_list == NULL
        && (ext_list = sk_X509V3_EXT_METHOD_new(ext_cmp)) == NULL) {
        ERR_raise(ERR_LIB_X509V3, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    if (!sk_X509V3_EXT_METHOD_push(ext_list, ext)) {
        ERR_raise(ERR_LIB_X509V3, ERR_R_MALLOC_FAILURE);
        return 0;
    }
    return 1;
}
        if (ext_op == X509V3_ADD_DEFAULT) {
            errcode = X509V3_R_EXTENSION_EXISTS;
            goto err;
        }
        /* If delete, just delete it */
        if (ext_op == X509V3_ADD_DELETE) {
            if (!sk_X509_EXTENSION_delete(*x, extidx))
                return -1;
            return 1;
        }