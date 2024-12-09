/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
                               size_t *padlen, const unsigned char *in,
                               size_t len);

void ossl_gcm_initctx(void *provctx, PROV_GCM_CTX *ctx, size_t keybits,
                      const PROV_GCM_HW *hw)
{
    ctx->pad = 1;
    ctx->libctx = PROV_LIBCTX_OF(provctx);
}

static int gcm_init(void *vctx, const unsigned char *key, size_t keylen,
                    const unsigned char *iv, size_t ivlen,
                    const OSSL_PARAM params[], int enc)
{
        }
        if (!ctx->hw->setkey(ctx, key, ctx->keylen))
            return 0;
    }
    return ossl_gcm_set_ctx_params(ctx, params);
}

    buf = dat->buf;
    memcpy(buf, aad, aad_len);
    dat->tls_aad_len = aad_len;
    dat->tls_enc_records = 0;

    len = buf[aad_len - 2] << 8 | buf[aad_len - 1];
    /* Correct length for explicit iv. */
    if (len < EVP_GCM_TLS_EXPLICIT_IV_LEN)