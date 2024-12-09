/*
 * Copyright 2018-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 */

#include "../testutil.h"
#include <openssl/provider.h>
#include <string.h>

int test_get_libctx(OSSL_LIB_CTX **libctx, OSSL_PROVIDER **default_null_prov,
                    const char *config_file,
    return test_get_libctx(libctx, default_null_prov,
                           test_get_argument(argn + 1), provider, module_name);
}