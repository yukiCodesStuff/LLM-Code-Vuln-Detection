/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#define OPENSSL_SUPPRESS_DEPRECATED

#include <stdio.h>
#include <errno.h>
#include <openssl/crypto.h>
#include "internal/numbers.h"
#include "bio_local.h"

/*
 * Helper macro for the callback to determine whether an operator expects a
 * len parameter or not
 */
#define HAS_LEN_OPER(o) ((o) == BIO_CB_READ || (o) == BIO_CB_WRITE \
                         || (o) == BIO_CB_GETS)

#ifndef OPENSSL_NO_DEPRECATED_3_0
# define HAS_CALLBACK(b) ((b)->callback != NULL || (b)->callback_ex != NULL)
#else
# define HAS_CALLBACK(b) ((b)->callback_ex != NULL)
#endif
/*
 * Helper function to work out whether to call the new style callback or the old
 * one, and translate between the two.
 *
 * This has a long return type for consistency with the old callback. Similarly
 * for the "long" used for "inret"
 */
static long bio_call_callback(BIO *b, int oper, const char *argp, size_t len,
                              int argi, long argl, long inret,
                              size_t *processed)
{
    long ret = inret;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    int bareoper;

    if (b->callback_ex != NULL)
#endif
        return b->callback_ex(b, oper, argp, len, argi, argl, inret, processed);

#ifndef OPENSSL_NO_DEPRECATED_3_0
    /* Strip off any BIO_CB_RETURN flag */
    bareoper = oper & ~BIO_CB_RETURN;

    /*
     * We have an old style callback, so we will have to do nasty casts and
     * check for overflows.
     */
    if (HAS_LEN_OPER(bareoper)) {
        /* In this case |len| is set, and should be used instead of |argi| */
        if (len > INT_MAX)
            return -1;

        argi = (int)len;
    }

    if (inret > 0 && (oper & BIO_CB_RETURN) && bareoper != BIO_CB_CTRL) {
        if (*processed > INT_MAX)
            return -1;
        inret = *processed;
    }

    ret = b->callback(b, oper, argp, argi, argl, inret);

    if (ret > 0 && (oper & BIO_CB_RETURN) && bareoper != BIO_CB_CTRL) {
        *processed = (size_t)ret;
        ret = 1;
    }
#endif
    return ret;
}
/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#define OPENSSL_SUPPRESS_DEPRECATED

#include <stdio.h>
#include <errno.h>
#include <openssl/crypto.h>
#include "internal/numbers.h"
#include "bio_local.h"

/*
 * Helper macro for the callback to determine whether an operator expects a
 * len parameter or not
 */
#define HAS_LEN_OPER(o) ((o) == BIO_CB_READ || (o) == BIO_CB_WRITE \
                         || (o) == BIO_CB_GETS)

#ifndef OPENSSL_NO_DEPRECATED_3_0
# define HAS_CALLBACK(b) ((b)->callback != NULL || (b)->callback_ex != NULL)
#else
# define HAS_CALLBACK(b) ((b)->callback_ex != NULL)
#endif
/*
 * Helper function to work out whether to call the new style callback or the old
 * one, and translate between the two.
 *
 * This has a long return type for consistency with the old callback. Similarly
 * for the "long" used for "inret"
 */
static long bio_call_callback(BIO *b, int oper, const char *argp, size_t len,
                              int argi, long argl, long inret,
                              size_t *processed)
{
    long ret = inret;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    int bareoper;

    if (b->callback_ex != NULL)
#endif
        return b->callback_ex(b, oper, argp, len, argi, argl, inret, processed);

#ifndef OPENSSL_NO_DEPRECATED_3_0
    /* Strip off any BIO_CB_RETURN flag */
    bareoper = oper & ~BIO_CB_RETURN;

    /*
     * We have an old style callback, so we will have to do nasty casts and
     * check for overflows.
     */
    if (HAS_LEN_OPER(bareoper)) {
        /* In this case |len| is set, and should be used instead of |argi| */
        if (len > INT_MAX)
            return -1;

        argi = (int)len;
    }

    if (inret > 0 && (oper & BIO_CB_RETURN) && bareoper != BIO_CB_CTRL) {
        if (*processed > INT_MAX)
            return -1;
        inret = *processed;
    }

    ret = b->callback(b, oper, argp, argi, argl, inret);

    if (ret > 0 && (oper & BIO_CB_RETURN) && bareoper != BIO_CB_CTRL) {
        *processed = (size_t)ret;
        ret = 1;
    }
#endif
    return ret;
}
    if (HAS_CALLBACK(b)) {
        ret = bio_call_callback(b, BIO_CB_CTRL, (void *)&fp, 0, cmd, 0, 1L,
                                NULL);
        if (ret <= 0)
            return ret;
    }

    ret = b->method->callback_ctrl(b, cmd, fp);

    if (HAS_CALLBACK(b))
        ret = bio_call_callback(b, BIO_CB_CTRL | BIO_CB_RETURN, (void *)&fp, 0,
                                cmd, 0, ret, NULL);

    return ret;
}

/*
 * It is unfortunate to duplicate in functions what the BIO_(w)pending macros
 * do; but those macros have inappropriate return type, and for interfacing
 * from other programming languages, C macros aren't much of a help anyway.
 */
size_t BIO_ctrl_pending(BIO *bio)
{
    long ret = BIO_ctrl(bio, BIO_CTRL_PENDING, 0, NULL);

    if (ret < 0)
        ret = 0;
#if LONG_MAX > SIZE_MAX
    if (ret > SIZE_MAX)
        ret = SIZE_MAX;
#endif
    return (size_t)ret;
}