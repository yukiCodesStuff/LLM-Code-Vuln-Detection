/*
 * Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
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
#include "internal/cryptlib.h"
#include "internal/provider.h"
#include "internal/core.h"
#include "crypto/evp.h"
#include "evp_local.h"

static int evp_kem_init(EVP_PKEY_CTX *ctx, int operation,
                        const OSSL_PARAM params[])
{
    int ret = 0;
    EVP_KEM *kem = NULL;
    EVP_KEYMGMT *tmp_keymgmt = NULL;
    const OSSL_PROVIDER *tmp_prov = NULL;
    void *provkey = NULL;
    const char *supported_kem = NULL;
    int iter;

    if (ctx == NULL || ctx->keytype == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        return 0;
    }

    evp_pkey_ctx_free_old_ops(ctx);
    ctx->operation = operation;

    if (ctx->pkey == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_NO_KEY_SET);
        goto err;
    }

    /*
     * Try to derive the supported kem from |ctx->keymgmt|.
     */
    if (!ossl_assert(ctx->pkey->keymgmt == NULL
                     || ctx->pkey->keymgmt == ctx->keymgmt)) {
        ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
        goto err;
    }
    supported_kem = evp_keymgmt_util_query_operation_name(ctx->keymgmt,
                                                          OSSL_OP_KEM);
    if (supported_kem == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    /*
     * Because we cleared out old ops, we shouldn't need to worry about
     * checking if kem is already there.
     * We perform two iterations:
     *
     * 1.  Do the normal kem fetch, using the fetching data given by
     *     the EVP_PKEY_CTX.
     * 2.  Do the provider specific kem fetch, from the same provider
     *     as |ctx->keymgmt|
     *
     * We then try to fetch the keymgmt from the same provider as the
     * kem, and try to export |ctx->pkey| to that keymgmt (when this
     * keymgmt happens to be the same as |ctx->keymgmt|, the export is
     * a no-op, but we call it anyway to not complicate the code even
     * more).
     * If the export call succeeds (returns a non-NULL provider key pointer),
     * we're done and can perform the operation itself.  If not, we perform
     * the second iteration, or jump to legacy.
     */
    for (iter = 1, provkey = NULL; iter < 3 && provkey == NULL; iter++) {
        EVP_KEYMGMT *tmp_keymgmt_tofree = NULL;

        /*
         * If we're on the second iteration, free the results from the first.
         * They are NULL on the first iteration, so no need to check what
         * iteration we're on.
         */
        EVP_KEM_free(kem);
        EVP_KEYMGMT_free(tmp_keymgmt);

        switch (iter) {
        case 1:
            kem = EVP_KEM_fetch(ctx->libctx, supported_kem, ctx->propquery);
            if (kem != NULL)
                tmp_prov = EVP_KEM_get0_provider(kem);
            break;
        case 2:
            tmp_prov = EVP_KEYMGMT_get0_provider(ctx->keymgmt);
            kem = evp_kem_fetch_from_prov((OSSL_PROVIDER *)tmp_prov,
                                          supported_kem, ctx->propquery);

            if (kem == NULL) {
                ERR_raise(ERR_LIB_EVP,
                          EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
                ret = -2;
                goto err;
            }
        }
        if (kem == NULL)
            continue;

        /*
         * Ensure that the key is provided, either natively, or as a cached
         * export.  We start by fetching the keymgmt with the same name as
         * |ctx->pkey|, but from the provider of the kem method, using the
         * same property query as when fetching the kem method.
         * With the keymgmt we found (if we did), we try to export |ctx->pkey|
         * to it (evp_pkey_export_to_provider() is smart enough to only actually

         * export it if |tmp_keymgmt| is different from |ctx->pkey|'s keymgmt)
         */
        tmp_keymgmt_tofree = tmp_keymgmt =
            evp_keymgmt_fetch_from_prov((OSSL_PROVIDER *)tmp_prov,
                                        EVP_KEYMGMT_get0_name(ctx->keymgmt),
                                        ctx->propquery);
        if (tmp_keymgmt != NULL)
            provkey = evp_pkey_export_to_provider(ctx->pkey, ctx->libctx,
                                                  &tmp_keymgmt, ctx->propquery);
        if (tmp_keymgmt == NULL)
            EVP_KEYMGMT_free(tmp_keymgmt_tofree);
    }

    if (provkey == NULL) {
        EVP_KEM_free(kem);
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    ctx->op.encap.kem = kem;
    ctx->op.encap.algctx = kem->newctx(ossl_provider_ctx(kem->prov));
    if (ctx->op.encap.algctx == NULL) {
        /* The provider key can stay in the cache */
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    switch (operation) {
    case EVP_PKEY_OP_ENCAPSULATE:
        if (kem->encapsulate_init == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            ret = -2;
            goto err;
        }
        ret = kem->encapsulate_init(ctx->op.encap.algctx, provkey, params);
        break;
    case EVP_PKEY_OP_DECAPSULATE:
        if (kem->decapsulate_init == NULL) {
            ERR_raise(ERR_LIB_EVP, EVP_R_OPERATION_NOT_SUPPORTED_FOR_THIS_KEYTYPE);
            ret = -2;
            goto err;
        }
        ret = kem->decapsulate_init(ctx->op.encap.algctx, provkey, params);
        break;
    default:
        ERR_raise(ERR_LIB_EVP, EVP_R_INITIALIZATION_ERROR);
        goto err;
    }

    EVP_KEYMGMT_free(tmp_keymgmt);
    tmp_keymgmt = NULL;

    if (ret > 0)
        return 1;
 err:
    if (ret <= 0) {
        evp_pkey_ctx_free_old_ops(ctx);
        ctx->operation = EVP_PKEY_OP_UNDEFINED;
    }
    EVP_KEYMGMT_free(tmp_keymgmt);
    return ret;
}
{
    return evp_generic_fetch_from_prov(prov, OSSL_OP_KEM, algorithm, properties,
                                       evp_kem_from_algorithm,
                                       (int (*)(void *))EVP_KEM_up_ref,
                                       (void (*)(void *))EVP_KEM_free);
}

int EVP_KEM_is_a(const EVP_KEM *kem, const char *name)
{
    return evp_is_a(kem->prov, kem->name_id, NULL, name);
}