/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        goto err;
#endif

    if (!ossl_bn_miller_rabin_is_prime(w, checks, ctx, cb, 0, &status)) {
        ret = -1;
        goto err;
    }
    ret = (status == BN_PRIMETEST_PROBABLY_PRIME);
err:
#ifndef FIPS_MODULE
    BN_CTX_free(ctxlocal);