/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
#include <stdio.h>
#include <errno.h>
#include <openssl/crypto.h>
#include "internal/numbers.h"
#include "bio_local.h"

/*
 * Helper macro for the callback to determine whether an operator expects a
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

size_t BIO_ctrl_wpending(BIO *bio)
{
    long ret = BIO_ctrl(bio, BIO_CTRL_WPENDING, 0, NULL);

    if (ret < 0)
        ret = 0;
#if LONG_MAX > SIZE_MAX
    if (ret > SIZE_MAX)
        ret = SIZE_MAX;
#endif
    return (size_t)ret;
}

/* put the 'bio' on the end of b's list of operators */
BIO *BIO_push(BIO *b, BIO *bio)