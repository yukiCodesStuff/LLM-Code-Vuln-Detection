/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    if (s->rlayer.rstate == SSL_ST_READ_BODY)
        return 0;

    for (i = 0; i < RECORD_LAYER_get_numrpipes(&s->rlayer); i++) {
        if (SSL3_RECORD_get_type(&s->rlayer.rrec[i])
            != SSL3_RT_APPLICATION_DATA)
            return 0;
        num += SSL3_RECORD_get_length(&s->rlayer.rrec[i]);
    }

    return num;
        int mode = EVP_CIPHER_CTX_get_mode(s->enc_write_ctx);
        if (mode == EVP_CIPH_CBC_MODE) {
            eivlen = EVP_CIPHER_CTX_get_iv_length(s->enc_write_ctx);
            if (eivlen <= 1)
                eivlen = 0;
        } else if (mode == EVP_CIPH_GCM_MODE) {
            /* Need explicit part of IV for GCM mode */