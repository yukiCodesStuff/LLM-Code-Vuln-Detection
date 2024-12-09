{
    return cipher->iv_len;
}

int EVP_CIPHER_CTX_get_iv_length(const EVP_CIPHER_CTX *ctx)
{
    int rv, len = EVP_CIPHER_get_iv_length(ctx->cipher);
    size_t v = len;
    OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };

    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_IVLEN, &v);
    rv = evp_do_ciph_ctx_getparams(ctx->cipher, ctx->algctx, params);
    if (rv == EVP_CTRL_RET_UNSUPPORTED)
        goto legacy;
    return rv != 0 ? (int)v : -1;
    /* Code below to be removed when legacy support is dropped. */
legacy:
    if ((EVP_CIPHER_get_flags(ctx->cipher) & EVP_CIPH_CUSTOM_IV_LENGTH) != 0) {
        rv = EVP_CIPHER_CTX_ctrl((EVP_CIPHER_CTX *)ctx, EVP_CTRL_GET_IVLEN,
                                 0, &len);
        return (rv == 1) ? len : -1;
    }
    return len;
}
{
    return ctx->cipher->nid;
}

int EVP_CIPHER_is_a(const EVP_CIPHER *cipher, const char *name)
{
    if (cipher->prov != NULL)
        return evp_is_a(cipher->prov, cipher->name_id, NULL, name);
    return evp_is_a(NULL, 0, EVP_CIPHER_get0_name(cipher), name);
}
{
    return EVP_CIPHER_get_flags(cipher) & EVP_CIPH_MODE;
}

int EVP_MD_is_a(const EVP_MD *md, const char *name)
{
    if (md->prov != NULL)
        return evp_is_a(md->prov, md->name_id, NULL, name);
    return evp_is_a(NULL, 0, EVP_MD_get0_name(md), name);
}