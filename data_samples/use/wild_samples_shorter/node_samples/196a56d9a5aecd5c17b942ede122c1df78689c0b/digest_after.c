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