/*
 * Copyright 2007-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Siemens AG 2015-2022
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * There are some known false positives, though, which are marked below.
 */

#define F                                       \
    void f()                                    \
    {                                           \
        int i;                                  \
        int j;                                  \
                                                \
        return;                                 \
    }

/*-
 * allow extra SPC in format-tagged multi-line comment
 */
int f(void) /*
             * trailing multi-line comment
             */
{
    typedef int INT;
    void v;
    short b;
    char c;
    signed s;
    unsigned u;
    int i;
    long l;
    float f;
    double d;
    enum {} enu;
    struct {} stru;
    union {} un;
    auto a;
    extern e;
    static int stat;
    const int con;
    volatile int vola;
    register int reg;
    OSSL_x y, *p = params;
    int params[];
    OSSL_PARAM * (* params []) [MAX + 1];
    XY *(* fn)(int a, char b);
    /*
     * multi-line comment should not disturb detection of local decls
     */
    BIO1 ***b;
    /* intra-line comment should not disturb detection of local decls */
    unsigned k;

    /* intra-line comment should not disturb detection of end of local decls */

    {
        int x; /* just decls in block */
    }
    if (p != (unsigned char *)
        &(ctx->tmp[0])) {
        i -= (p - (unsigned char *) /* do not confuse with var decl */
              &(ctx->tmp[0]));
    }
    {
        ctx->buf_off = 0; /* do not confuse with var decl */
        return 0;
    }
    {
        ctx->buf_len = EVP_EncodeBlock((unsigned char *)ctx->buf,
                                       (unsigned char *)ctx->tmp, /* no decl */
                                       ctx->tmp_len);
    }
    {
        EVP_EncodeFinal(ctx->base64,
                        (unsigned char *)ctx->buf, &(ctx->len)); /* no decl */
        /* push out the bytes */
        goto again;
    }
    {
        f(1, (unsigned long)2); /* no decl */
        x;
    }
    {
        char *pass_str = get_passwd(opt_srv_secret, "x");

        if (pass_str != NULL) {
            cleanse(opt_srv_secret);
            res = OSSL_CMP_CTX_set1_secretValue(ctx, (unsigned char *)pass_str,
                                                strlen(pass_str));
            clear_free(pass_str);
        }
    }
}

int g(void)
{
    if (ctx == NULL) {    /* non-leading end-of-line comment */
        if (/* comment after '(' */ pem_name != NULL /* comment before ')' */)
            /* entire-line comment indent usually like for the following line */
        ;
    for (i = 0; i < 1;)
        ;
    for (;;)
        for (; i < n; i++)
            for (;; p++)
                ;
    for (;;) ; /* should not trigger: space before ';' */
 lab: ;  /* should not trigger: space before ';' */

#if X
    if (1) /* bad style: just part of control structure depends on #if */
#else
/* should not trigger: constant on LHS of comparison or assignment operator */
X509 *x509 = NULL;
int y = a + 1 < b;
int ret, was_NULL = *certs == NULL;

/* should not trigger: no space before binary ... operator */
float z = 1e-6 * (-1) * b[+6] * 1e+1 * (a)->f * (long)+1
    - (tmstart.tv_sec + tmstart.tv_nsec * 1e-9);
struct st = {-1, 0};

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