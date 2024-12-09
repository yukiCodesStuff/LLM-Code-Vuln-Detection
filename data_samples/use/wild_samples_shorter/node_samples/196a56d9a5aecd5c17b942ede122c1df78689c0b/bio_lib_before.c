/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
#include <stdio.h>
#include <errno.h>
#include <openssl/crypto.h>
#include "bio_local.h"

/*
 * Helper macro for the callback to determine whether an operator expects a
 */
size_t BIO_ctrl_pending(BIO *bio)
{
    return BIO_ctrl(bio, BIO_CTRL_PENDING, 0, NULL);
}

size_t BIO_ctrl_wpending(BIO *bio)
{
    return BIO_ctrl(bio, BIO_CTRL_WPENDING, 0, NULL);
}

/* put the 'bio' on the end of b's list of operators */
BIO *BIO_push(BIO *b, BIO *bio)