/*
 * Copyright 2016-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
{
    EVP_CIPHER_CTX *ctx;
    unsigned char iv[EVP_MAX_IV_LENGTH], recheader[SSL3_RT_HEADER_LENGTH];
    size_t ivlen, taglen, offset, loop, hdrlen;
    unsigned char *staticiv;
    unsigned char *seq;
    int lenu, lenf;
    SSL3_RECORD *rec = &recs[0];
    }

    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);

    if (s->early_data_state == SSL_EARLY_DATA_WRITING
            || s->early_data_state == SSL_EARLY_DATA_WRITE_RETRY) {
        if (s->session != NULL && s->session->ext.max_early_data > 0) {