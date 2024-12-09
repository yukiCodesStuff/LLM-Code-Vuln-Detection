/*
 * Copyright 2018-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    case SSL_AES128GCM:
    case SSL_AES256GCM:
        crypto_info->cipher_algorithm = CRYPTO_AES_NIST_GCM_16;
        if (s->version == TLS1_3_VERSION)
            crypto_info->iv_len = EVP_CIPHER_CTX_get_iv_length(dd);
        else
            crypto_info->iv_len = EVP_GCM_TLS_FIXED_IV_LEN;
        break;
    case SSL_AES128: