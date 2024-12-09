/*
 * Copyright 1999-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
        if (EVP_CIPHER_CTX_is_encrypting(ctx)) {
            if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG,
                (int)mac_len, out+outlen) < 0) {
                OPENSSL_free(out);
                out = NULL;
                ERR_raise(ERR_LIB_PKCS12, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            outlen += mac_len;