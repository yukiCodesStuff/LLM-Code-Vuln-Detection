/*
 * Copyright 2007-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright Nokia 2007-2019
 * Copyright Siemens AG 2015-2019
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <stdio.h>

#include <openssl/asn1t.h>
#include <openssl/http.h>
#include "internal/sockets.h"

#include <openssl/cmp.h>
#include "cmp_local.h"

/* explicit #includes not strictly needed since implied by the above: */
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/cmp.h>
#include <openssl/err.h>

static int keep_alive(int keep_alive, int body_type)
{
    if (keep_alive != 0
        /* Ask for persistent connection only if may need more round trips */
            && body_type != OSSL_CMP_PKIBODY_IR
            && body_type != OSSL_CMP_PKIBODY_CR
            && body_type != OSSL_CMP_PKIBODY_P10CR
            && body_type != OSSL_CMP_PKIBODY_KUR
            && body_type != OSSL_CMP_PKIBODY_POLLREQ)
        keep_alive = 0;
    return keep_alive;
}
{
    if (keep_alive != 0
        /* Ask for persistent connection only if may need more round trips */
            && body_type != OSSL_CMP_PKIBODY_IR
            && body_type != OSSL_CMP_PKIBODY_CR
            && body_type != OSSL_CMP_PKIBODY_P10CR
            && body_type != OSSL_CMP_PKIBODY_KUR
            && body_type != OSSL_CMP_PKIBODY_POLLREQ)
        keep_alive = 0;
    return keep_alive;
}