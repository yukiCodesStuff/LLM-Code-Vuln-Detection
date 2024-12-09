/*
 * Copyright 2006-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include "internal/numbers.h"   /* includes SIZE_MAX */
#include "internal/cryptlib.h"
#include "internal/provider.h"
#include "internal/core.h"
#include "crypto/evp.h"
#include "evp_local.h"

static EVP_SIGNATURE *evp_signature_new(OSSL_PROVIDER *prov)
{
    EVP_SIGNATURE *signature = OPENSSL_zalloc(sizeof(EVP_SIGNATURE));

    if (signature == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    signature->lock = CRYPTO_THREAD_lock_new();
    if (signature->lock == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        OPENSSL_free(signature);
        return NULL;
    }
    signature->prov = prov;
    ossl_provider_up_ref(prov);
    signature->refcnt = 1;

    return signature;
}
{
    return evp_generic_fetch_from_prov(prov, OSSL_OP_SIGNATURE,
                                       algorithm, properties,
                                       evp_signature_from_algorithm,
                                       (int (*)(void *))EVP_SIGNATURE_up_ref,
                                       (void (*)(void *))EVP_SIGNATURE_free);
}

int EVP_SIGNATURE_is_a(const EVP_SIGNATURE *signature, const char *name)
{
    return signature != NULL
           && evp_is_a(signature->prov, signature->name_id, NULL, name);
}