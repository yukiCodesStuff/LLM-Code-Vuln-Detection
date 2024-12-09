/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    int ret = 0, qbits = BN_num_bits(params->q);
    BIGNUM *m, *two_powN = NULL;

    /* Deal with the edge cases where the value of N and/or s is not set */
    if (s == 0)
        goto err;
    if (N == 0)
        N = params->keylength ? params->keylength : 2 * s;

    /* Step (2) : check range of N */
    if (N < 2 * s || N > qbits)
        return 0;