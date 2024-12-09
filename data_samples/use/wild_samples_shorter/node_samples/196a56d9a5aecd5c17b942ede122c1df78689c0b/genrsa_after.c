/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    if (out == NULL)
        goto end;

    if (!init_gen_str(&ctx, "RSA", eng, 0, app_get0_libctx(),
                      app_get0_propq()))
        goto end;

    EVP_PKEY_CTX_set_cb(ctx, genrsa_cb);
    EVP_PKEY_CTX_set_app_data(ctx, bio_err);