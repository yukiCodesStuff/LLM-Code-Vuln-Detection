    if (type != NULL) {
        /*
         * Ensure an ENGINE left lying around from last time is cleared (the
         * previous check attempted to avoid this if the same ENGINE and
         * EVP_MD could be used).
         */
        ENGINE_finish(ctx->engine);
        ctx->engine = NULL;
    }

    if (type != NULL && impl == NULL)
        tmpimpl = ENGINE_get_digest_engine(type->type);
#endif

    /*
     * If there are engines involved or EVP_MD_CTX_FLAG_NO_INIT is set then we
     * should use legacy handling for now.
     */
    if (ctx->engine != NULL
            || impl != NULL
#if !defined(OPENSSL_NO_ENGINE) && !defined(FIPS_MODULE)
            || tmpimpl != NULL
#endif
            || (ctx->flags & EVP_MD_CTX_FLAG_NO_INIT) != 0
            || (type != NULL && type->origin == EVP_ORIG_METH)
            || (type == NULL && ctx->digest != NULL
                             && ctx->digest->origin == EVP_ORIG_METH)) {
        if (ctx->digest == ctx->fetched_digest)
            ctx->digest = NULL;
        EVP_MD_free(ctx->fetched_digest);
        ctx->fetched_digest = NULL;
        goto legacy;
    }