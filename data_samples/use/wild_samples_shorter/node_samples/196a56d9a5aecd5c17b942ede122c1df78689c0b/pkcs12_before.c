/*
 * Copyright 1999-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
     "Encrypt output with 3DES (default PBES2 with PBKDF2 and AES-256 CBC)"},
#endif
    {"macalg", OPT_MACALG, 's',
     "Digest algorithm to use in MAC (default SHA1)"},
    {"iter", OPT_ITER, 'p', "Specify the iteration count for encryption and MAC"},
    {"noiter", OPT_NOITER, '-', "Don't use encryption iteration"},
    {"nomaciter", OPT_NOMACITER, '-', "Don't use MAC iteration)"},
    {"maciter", OPT_MACITER, '-', "Unused, kept for backwards compatibility"},