/*
 * Copyright 2009-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    }

    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);

    if (ivlen > 0) {
        if (RAND_bytes_ex(ossl_cms_ctx_get0_libctx(cms_ctx), iv, ivlen, 0) <= 0)
            goto err;