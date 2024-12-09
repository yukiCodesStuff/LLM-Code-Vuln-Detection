/*
 * Copyright 2014-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at

# define ossl_test__attr__(x)
# if defined(__GNUC__) && defined(__STDC_VERSION__) \
    && !defined(__MINGW32__) && !defined(__MINGW64__) \
    && !defined(__APPLE__)
    /*
     * Because we support the 'z' modifier, which made its appearance in C99,
     * we can't use __attribute__ with pre C99 dialects.