
int EVP_CIPHER_CTX_get_iv_length(const EVP_CIPHER_CTX *ctx)
{
    if (ctx->iv_len < 0) {
        int rv, len = EVP_CIPHER_get_iv_length(ctx->cipher);
        size_t v = len;
        OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };

        if (ctx->cipher->get_ctx_params != NULL) {
            params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_IVLEN,
                                                    &v);
            rv = evp_do_ciph_ctx_getparams(ctx->cipher, ctx->algctx, params);
            if (rv > 0) {
                if (OSSL_PARAM_modified(params)
                        && !OSSL_PARAM_get_int(params, &len))
                    return -1;
            } else if (rv != EVP_CTRL_RET_UNSUPPORTED) {
                return -1;
            }
        }
        /* Code below to be removed when legacy support is dropped. */
        else if ((EVP_CIPHER_get_flags(ctx->cipher)
                  & EVP_CIPH_CUSTOM_IV_LENGTH) != 0) {
            rv = EVP_CIPHER_CTX_ctrl((EVP_CIPHER_CTX *)ctx, EVP_CTRL_GET_IVLEN,
                                     0, &len);
            if (rv <= 0)
                return -1;
        }
        /*-
         * Casting away the const is annoying but required here.  We need to
         * cache the result for performance reasons.
         */
        ((EVP_CIPHER_CTX *)ctx)->iv_len = len;
    }
    return ctx->iv_len;
}

int EVP_CIPHER_CTX_get_tag_length(const EVP_CIPHER_CTX *ctx)
{

int EVP_CIPHER_is_a(const EVP_CIPHER *cipher, const char *name)
{
    if (cipher == NULL)
        return 0;
    if (cipher->prov != NULL)
        return evp_is_a(cipher->prov, cipher->name_id, NULL, name);
    return evp_is_a(NULL, 0, EVP_CIPHER_get0_name(cipher), name);
}

int EVP_MD_is_a(const EVP_MD *md, const char *name)
{
    if (md == NULL)
        return 0;
    if (md->prov != NULL)
        return evp_is_a(md->prov, md->name_id, NULL, name);
    return evp_is_a(NULL, 0, EVP_MD_get0_name(md), name);
}