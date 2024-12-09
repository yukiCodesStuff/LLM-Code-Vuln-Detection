/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* Dispatch functions for gcm mode */

#include <openssl/rand.h>
#include <openssl/proverr.h>
#include "prov/ciphercommon.h"
#include "prov/ciphercommon_gcm.h"
#include "prov/providercommon.h"
#include "prov/provider_ctx.h"

static int gcm_tls_init(PROV_GCM_CTX *dat, unsigned char *aad, size_t aad_len);
static int gcm_tls_iv_set_fixed(PROV_GCM_CTX *ctx, unsigned char *iv,
                                size_t len);
static int gcm_tls_cipher(PROV_GCM_CTX *ctx, unsigned char *out, size_t *padlen,
                          const unsigned char *in, size_t len);
static int gcm_cipher_internal(PROV_GCM_CTX *ctx, unsigned char *out,
                               size_t *padlen, const unsigned char *in,
                               size_t len);

/*
 * Called from EVP_CipherInit when there is currently no context via
 * the new_ctx() function
 */
void ossl_gcm_initctx(void *provctx, PROV_GCM_CTX *ctx, size_t keybits,
                      const PROV_GCM_HW *hw)
{
    ctx->pad = 1;
    ctx->mode = EVP_CIPH_GCM_MODE;
    ctx->taglen = UNINITIALISED_SIZET;
    ctx->tls_aad_len = UNINITIALISED_SIZET;
    ctx->ivlen = (EVP_GCM_TLS_FIXED_IV_LEN + EVP_GCM_TLS_EXPLICIT_IV_LEN);
    ctx->keylen = keybits / 8;
    ctx->hw = hw;
    ctx->libctx = PROV_LIBCTX_OF(provctx);
}
/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/* Dispatch functions for gcm mode */

#include <openssl/rand.h>
#include <openssl/proverr.h>
#include "prov/ciphercommon.h"
#include "prov/ciphercommon_gcm.h"
#include "prov/providercommon.h"
#include "prov/provider_ctx.h"

static int gcm_tls_init(PROV_GCM_CTX *dat, unsigned char *aad, size_t aad_len);
static int gcm_tls_iv_set_fixed(PROV_GCM_CTX *ctx, unsigned char *iv,
                                size_t len);
static int gcm_tls_cipher(PROV_GCM_CTX *ctx, unsigned char *out, size_t *padlen,
                          const unsigned char *in, size_t len);
static int gcm_cipher_internal(PROV_GCM_CTX *ctx, unsigned char *out,
                               size_t *padlen, const unsigned char *in,
                               size_t len);

/*
 * Called from EVP_CipherInit when there is currently no context via
 * the new_ctx() function
 */
void ossl_gcm_initctx(void *provctx, PROV_GCM_CTX *ctx, size_t keybits,
                      const PROV_GCM_HW *hw)
{
    ctx->pad = 1;
    ctx->mode = EVP_CIPH_GCM_MODE;
    ctx->taglen = UNINITIALISED_SIZET;
    ctx->tls_aad_len = UNINITIALISED_SIZET;
    ctx->ivlen = (EVP_GCM_TLS_FIXED_IV_LEN + EVP_GCM_TLS_EXPLICIT_IV_LEN);
    ctx->keylen = keybits / 8;
    ctx->hw = hw;
    ctx->libctx = PROV_LIBCTX_OF(provctx);
}
{
    ctx->pad = 1;
    ctx->mode = EVP_CIPH_GCM_MODE;
    ctx->taglen = UNINITIALISED_SIZET;
    ctx->tls_aad_len = UNINITIALISED_SIZET;
    ctx->ivlen = (EVP_GCM_TLS_FIXED_IV_LEN + EVP_GCM_TLS_EXPLICIT_IV_LEN);
    ctx->keylen = keybits / 8;
    ctx->hw = hw;
    ctx->libctx = PROV_LIBCTX_OF(provctx);
}

/*
 * Called by EVP_CipherInit via the _einit and _dinit functions
 */
static int gcm_init(void *vctx, const unsigned char *key, size_t keylen,
                    const unsigned char *iv, size_t ivlen,
                    const OSSL_PARAM params[], int enc)
{
    PROV_GCM_CTX *ctx = (PROV_GCM_CTX *)vctx;

    if (!ossl_prov_is_running())
        return 0;

    ctx->enc = enc;

    if (iv != NULL) {
        if (ivlen == 0 || ivlen > sizeof(ctx->iv)) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_IV_LENGTH);
            return 0;
        }
        ctx->ivlen = ivlen;
        memcpy(ctx->iv, iv, ivlen);
        ctx->iv_state = IV_STATE_BUFFERED;
    }
        if (keylen != ctx->keylen) {
            ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY_LENGTH);
            return 0;
        }
        if (!ctx->hw->setkey(ctx, key, ctx->keylen))
            return 0;
        ctx->tls_enc_records = 0;
    }
    return ossl_gcm_set_ctx_params(ctx, params);
}

int ossl_gcm_einit(void *vctx, const unsigned char *key, size_t keylen,
                   const unsigned char *iv, size_t ivlen,
                   const OSSL_PARAM params[])
{
    return gcm_init(vctx, key, keylen, iv, ivlen, params, 1);
}

int ossl_gcm_dinit(void *vctx, const unsigned char *key, size_t keylen,
                   const unsigned char *iv, size_t ivlen,
                   const OSSL_PARAM params[])
{
    return gcm_init(vctx, key, keylen, iv, ivlen, params, 0);
}

/* increment counter (64-bit int) by 1 */
static void ctr64_inc(unsigned char *counter)
{
    int n = 8;
    unsigned char c;

    do {
        --n;
        c = counter[n];
        ++c;
        counter[n] = c;
        if (c > 0)
            return;
    } while (n > 0);
}

static int getivgen(PROV_GCM_CTX *ctx, unsigned char *out, size_t olen)
{
    if (!ctx->iv_gen
        || !ctx->key_set
        || !ctx->hw->setiv(ctx, ctx->iv, ctx->ivlen))
        return 0;
    if (olen == 0 || olen > ctx->ivlen)
        olen = ctx->ivlen;
    memcpy(out, ctx->iv + ctx->ivlen - olen, olen);
    /*
     * Invocation field will be at least 8 bytes in size and so no need
     * to check wrap around or increment more than last 8 bytes.
     */
    ctr64_inc(ctx->iv + ctx->ivlen - 8);
    ctx->iv_state = IV_STATE_COPIED;
    return 1;
}

static int setivinv(PROV_GCM_CTX *ctx, unsigned char *in, size_t inl)
{
    if (!ctx->iv_gen
        || !ctx->key_set
        || ctx->enc)
        return 0;

    memcpy(ctx->iv + ctx->ivlen - inl, in, inl);
    if (!ctx->hw->setiv(ctx, ctx->iv, ctx->ivlen))
        return 0;
    ctx->iv_state = IV_STATE_COPIED;
    return 1;
}

int ossl_gcm_get_ctx_params(void *vctx, OSSL_PARAM params[])
{
    PROV_GCM_CTX *ctx = (PROV_GCM_CTX *)vctx;
    OSSL_PARAM *p;
    size_t sz;

    p = OSSL_PARAM_locate(params, OSSL_CIPHER_PARAM_IVLEN);
    if (p != NULL && !OSSL_PARAM_set_size_t(p, ctx->ivlen)) {
        ERR_raise(ERR_LIB_PROV, PROV_R_FAILED_TO_SET_PARAMETER);
        return 0;
    }
{
    unsigned char *buf;
    size_t len;

    if (!ossl_prov_is_running() || aad_len != EVP_AEAD_TLS1_AAD_LEN)
       return 0;

    /* Save the aad for later use. */
    buf = dat->buf;
    memcpy(buf, aad, aad_len);
    dat->tls_aad_len = aad_len;

    len = buf[aad_len - 2] << 8 | buf[aad_len - 1];
    /* Correct length for explicit iv. */
    if (len < EVP_GCM_TLS_EXPLICIT_IV_LEN)
        return 0;
    len -= EVP_GCM_TLS_EXPLICIT_IV_LEN;

    /* If decrypting correct for tag too. */
    if (!dat->enc) {
        if (len < EVP_GCM_TLS_TAG_LEN)
            return 0;
        len -= EVP_GCM_TLS_TAG_LEN;
    }