/*
 * Copyright 2018-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "ssl_local.h"
#include "internal/ktls.h"

#if defined(__FreeBSD__)
# include "crypto/cryptodev.h"

/*-
 * Check if a given cipher is supported by the KTLS interface.
 * The kernel might still fail the setsockopt() if no suitable
 * provider is found, but this checks if the socket option
 * supports the cipher suite used at all.
 */
int ktls_check_supported_cipher(const SSL *s, const EVP_CIPHER *c,
                                const EVP_CIPHER_CTX *dd)
{

    switch (s->version) {
    case TLS1_VERSION:
    case TLS1_1_VERSION:
    case TLS1_2_VERSION:
    case TLS1_3_VERSION:
        break;
    default:
        return 0;
    }

    switch (s->s3.tmp.new_cipher->algorithm_enc) {
    case SSL_AES128GCM:
    case SSL_AES256GCM:
        return 1;
    case SSL_AES128:
    case SSL_AES256:
        if (s->ext.use_etm)
            return 0;
        switch (s->s3.tmp.new_cipher->algorithm_mac) {
        case SSL_SHA1:
        case SSL_SHA256:
        case SSL_SHA384:
            return 1;
        default:
            return 0;
        }
    default:
        return 0;
    }
}
    switch (s->s3.tmp.new_cipher->algorithm_enc) {
    case SSL_AES128GCM:
    case SSL_AES256GCM:
        crypto_info->cipher_algorithm = CRYPTO_AES_NIST_GCM_16;
        if (s->version == TLS1_3_VERSION)
            crypto_info->iv_len = EVP_CIPHER_CTX_get_iv_length(dd);
        else
            crypto_info->iv_len = EVP_GCM_TLS_FIXED_IV_LEN;
        break;
    case SSL_AES128:
    case SSL_AES256:
        switch (s->s3.tmp.new_cipher->algorithm_mac) {
        case SSL_SHA1:
            crypto_info->auth_algorithm = CRYPTO_SHA1_HMAC;
            break;
        case SSL_SHA256:
            crypto_info->auth_algorithm = CRYPTO_SHA2_256_HMAC;
            break;
        case SSL_SHA384:
            crypto_info->auth_algorithm = CRYPTO_SHA2_384_HMAC;
            break;
        default:
            return 0;
        }