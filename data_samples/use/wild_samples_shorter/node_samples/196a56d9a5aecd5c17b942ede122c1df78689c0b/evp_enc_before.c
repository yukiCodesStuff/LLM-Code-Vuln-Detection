    if (ctx->fetched_cipher != NULL)
        EVP_CIPHER_free(ctx->fetched_cipher);
    memset(ctx, 0, sizeof(*ctx));

    return 1;

    /* Remove legacy code below when legacy support is removed. */
    ENGINE_finish(ctx->engine);
#endif
    memset(ctx, 0, sizeof(*ctx));
    return 1;
}

EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void)
#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
    ENGINE *tmpimpl = NULL;
#endif
    /*
     * enc == 1 means we are encrypting.
     * enc == 0 means we are decrypting.
     * enc == -1 means, use the previously initialised value for encrypt/decrypt
#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
            || tmpimpl != NULL
#endif
            || impl != NULL) {
        if (ctx->cipher == ctx->fetched_cipher)
            ctx->cipher = NULL;
        EVP_CIPHER_free(ctx->fetched_cipher);
        ctx->fetched_cipher = NULL;
     * (legacy code)
     */
    if (cipher != NULL && ctx->cipher != NULL) {
        OPENSSL_clear_free(ctx->cipher_data, ctx->cipher->ctx_size);
        ctx->cipher_data = NULL;
    }


    /* Start of non-legacy code below */

    /* Ensure a context left lying around from last time is cleared */
    if (cipher != NULL && ctx->cipher != NULL) {
        if (arg < 0)
            return 0;
        params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_IVLEN, &sz);
        break;
    case EVP_CTRL_CCM_SET_L:
        if (arg < 2 || arg > 8)
            return 0;
        sz = 15 - arg;
        params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_IVLEN, &sz);
        break;
    case EVP_CTRL_AEAD_SET_IV_FIXED:
        params[0] = OSSL_PARAM_construct_octet_string(
                        OSSL_CIPHER_PARAM_AEAD_TLS1_IV_FIXED, ptr, sz);

int EVP_CIPHER_CTX_set_params(EVP_CIPHER_CTX *ctx, const OSSL_PARAM params[])
{
    if (ctx->cipher != NULL && ctx->cipher->set_ctx_params != NULL)
        return ctx->cipher->set_ctx_params(ctx->algctx, params);
    return 0;
}

int EVP_CIPHER_CTX_get_params(EVP_CIPHER_CTX *ctx, OSSL_PARAM params[])