/*
 * Copyright 2007-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * There are some known false positives, though, which are marked below.
 */

/*-
 * allow extra SPC in format-tagged multi-line comment
 */
int f(void) /*
             * trailing multi-line comment
             */
{
    if (ctx == NULL) {    /* non-leading end-of-line comment */
        if (/* comment after '(' */ pem_name != NULL /* comment before ')' */)
            /* entire-line comment indent usually like for the following line */
        ;
    for (i = 0; i < 1;)
        ;

#if X
    if (1) /* bad style: just part of control structure depends on #if */
#else
/* should not trigger: constant on LHS of comparison or assignment operator */
X509 *x509 = NULL;
int y = a + 1 < b;

const OPTIONS passwd_options[] = {
    {"aixmd5", OPT_AIXMD5, '-', "AIX MD5-based password algorithm"},
#if !defined(OPENSSL_NO_DES) && !defined(OPENSSL_NO_DEPRECATED_3_0)
typedef OSSL_CMP_MSG *(*cmp_srv_process_cb_t)
    (OSSL_CMP_SRV_CTX *ctx, OSSL_CMP_MSG *msg)
    xx;
int f()
{
    c;
    if (1) {