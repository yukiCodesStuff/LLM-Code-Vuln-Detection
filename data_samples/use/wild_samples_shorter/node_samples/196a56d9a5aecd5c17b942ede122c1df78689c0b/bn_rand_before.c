/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    int n;
    int count = 100;

    if (range->neg || BN_is_zero(range)) {
        ERR_raise(ERR_LIB_BN, BN_R_INVALID_RANGE);
        return 0;
    }