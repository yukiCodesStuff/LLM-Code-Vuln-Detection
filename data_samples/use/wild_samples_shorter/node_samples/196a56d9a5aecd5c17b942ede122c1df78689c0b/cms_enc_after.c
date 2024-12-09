/*
 * Copyright 2008-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        calg->algorithm = OBJ_nid2obj(EVP_CIPHER_CTX_get_type(ctx));
        /* Generate a random IV if we need one */
        ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
        if (ivlen < 0) {
            ERR_raise(ERR_LIB_CMS, ERR_R_EVP_LIB);
            goto err;
        }

        if (ivlen > 0) {
            if (RAND_bytes_ex(libctx, iv, ivlen, 0) <= 0)
                goto err;
            piv = iv;