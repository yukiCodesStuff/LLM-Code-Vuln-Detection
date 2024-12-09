/*
 * Copyright 1995-2018 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal/cryptlib.h"
#include <openssl/buffer.h>
#include <openssl/txt_db.h>

#undef BUFSIZE
#define BUFSIZE 512

TXT_DB *TXT_DB_read(BIO *in, int num)
{
    TXT_DB *ret = NULL;
    int esc = 0;
    long ln = 0;
    int i, add, n;
    int size = BUFSIZE;
    int offset = 0;
    char *p, *f;
    OPENSSL_STRING *pp;
    BUF_MEM *buf = NULL;

    if ((buf = BUF_MEM_new()) == NULL)
        goto err;
    if (!BUF_MEM_grow(buf, size))
        goto err;

    if ((ret = OPENSSL_malloc(sizeof(*ret))) == NULL)
        goto err;
    ret->num_fields = num;
    ret->index = NULL;
    ret->qual = NULL;
    if ((ret->data = sk_OPENSSL_PSTRING_new_null()) == NULL)
        goto err;
    if ((ret->index = OPENSSL_malloc(sizeof(*ret->index) * num)) == NULL)
        goto err;
    if ((ret->qual = OPENSSL_malloc(sizeof(*(ret->qual)) * num)) == NULL)
        goto err;
    for (i = 0; i < num; i++) {
        ret->index[i] = NULL;
        ret->qual[i] = NULL;
    }

    add = (num + 1) * sizeof(char *);
    buf->data[size - 1] = '\0';
    offset = 0;
    for (;;) {
        if (offset != 0) {
            size += BUFSIZE;
            if (!BUF_MEM_grow_clean(buf, size))
                goto err;
        }
        buf->data[offset] = '\0';
        BIO_gets(in, &(buf->data[offset]), size - offset);
        ln++;
        if (buf->data[offset] == '\0')
            break;
        if ((offset == 0) && (buf->data[0] == '#'))
            continue;
        i = strlen(&(buf->data[offset]));
        offset += i;
        if (buf->data[offset - 1] != '\n')
            continue;
        else {
            buf->data[offset - 1] = '\0'; /* blat the '\n' */
            if ((p = OPENSSL_malloc(add + offset)) == NULL)
                goto err;
            offset = 0;
        }
        pp = (char **)p;
        p += add;
        n = 0;
        pp[n++] = p;
        i = 0;
        f = buf->data;

        esc = 0;
        for (;;) {
            if (*f == '\0')
                break;
            if (*f == '\t') {
                if (esc)
                    p--;
                else {
                    *(p++) = '\0';
                    f++;
                    if (n >= num)
                        break;
                    pp[n++] = p;
                    continue;
                }
            }
            esc = (*f == '\\');
            *(p++) = *(f++);
        }
        *(p++) = '\0';
        if ((n != num) || (*f != '\0')) {
            OPENSSL_free(pp);
            ret->error = DB_ERROR_WRONG_NUM_FIELDS;
            goto err;
        }
        pp[n] = p;
        if (!sk_OPENSSL_PSTRING_push(ret->data, pp)) {
            OPENSSL_free(pp);
            goto err;
        }
    }
    BUF_MEM_free(buf);
    return ret;
 err:
    BUF_MEM_free(buf);
    if (ret != NULL) {
        sk_OPENSSL_PSTRING_free(ret->data);
        OPENSSL_free(ret->index);
        OPENSSL_free(ret->qual);
        OPENSSL_free(ret);
    }
    return NULL;
}
{
    TXT_DB *ret = NULL;
    int esc = 0;
    long ln = 0;
    int i, add, n;
    int size = BUFSIZE;
    int offset = 0;
    char *p, *f;
    OPENSSL_STRING *pp;
    BUF_MEM *buf = NULL;

    if ((buf = BUF_MEM_new()) == NULL)
        goto err;
    if (!BUF_MEM_grow(buf, size))
        goto err;

    if ((ret = OPENSSL_malloc(sizeof(*ret))) == NULL)
        goto err;
    ret->num_fields = num;
    ret->index = NULL;
    ret->qual = NULL;
    if ((ret->data = sk_OPENSSL_PSTRING_new_null()) == NULL)
        goto err;
    if ((ret->index = OPENSSL_malloc(sizeof(*ret->index) * num)) == NULL)
        goto err;
    if ((ret->qual = OPENSSL_malloc(sizeof(*(ret->qual)) * num)) == NULL)
        goto err;
    for (i = 0; i < num; i++) {
        ret->index[i] = NULL;
        ret->qual[i] = NULL;
    }
        if (offset != 0) {
            size += BUFSIZE;
            if (!BUF_MEM_grow_clean(buf, size))
                goto err;
        }
        buf->data[offset] = '\0';
        BIO_gets(in, &(buf->data[offset]), size - offset);
        ln++;
        if (buf->data[offset] == '\0')
            break;
        if ((offset == 0) && (buf->data[0] == '#'))
            continue;
        i = strlen(&(buf->data[offset]));
        offset += i;
        if (buf->data[offset - 1] != '\n')
            continue;
        else {
            buf->data[offset - 1] = '\0'; /* blat the '\n' */
            if ((p = OPENSSL_malloc(add + offset)) == NULL)
                goto err;
            offset = 0;
        }