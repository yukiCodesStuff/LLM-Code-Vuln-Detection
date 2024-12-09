     * SUBJECT_ISSUER_MISMATCH just means 'x' is clearly not issued by 'issuer'.
     * Every other error code likely indicates a real error.
     */
    return 0;
}

/*-
    time_t *ptime;
    int i;

    if ((ctx->param->flags & X509_V_FLAG_USE_CHECK_TIME) != 0)
        ptime = &ctx->param->check_time;
    else if ((ctx->param->flags & X509_V_FLAG_NO_CHECK_TIME) != 0)
        return 1;
    else
        ptime = NULL;
    if (notify)
        ctx->current_crl = crl;

    i = X509_cmp_time(X509_CRL_get0_lastUpdate(crl), ptime);
    if (i == 0) {
        if (!notify)
int X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509,
                        STACK_OF(X509) *chain)
{
    if (ctx == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    }

    /* Inherit callbacks and flags from X509_STORE if not set use defaults. */
    if (store == NULL)
        ctx->param->inh_flags |= X509_VP_FLAG_DEFAULT | X509_VP_FLAG_ONCE;
    else if (X509_VERIFY_PARAM_inherit(ctx->param, store->param) == 0)
        goto err;

    if (!X509_STORE_CTX_set_default(ctx, "default"))
        goto err;

    /*
     * XXX: For now, continue to inherit trust from VPM, but infer from the
     * purpose if this still yields the default value.
    const X509_VERIFY_PARAM *param;

    param = X509_VERIFY_PARAM_lookup(name);
    if (param == NULL) {
        ERR_raise_data(ERR_LIB_X509, X509_R_UNKNOWN_PURPOSE_ID, "name=%s", name);
        return 0;
    }
    return X509_VERIFY_PARAM_inherit(ctx->param, param);
}

X509_VERIFY_PARAM *X509_STORE_CTX_get0_param(const X509_STORE_CTX *ctx)
    int alt_untrusted = 0;
    int max_depth;
    int ok = 0;
    int i;

    /* Our chain starts with a single untrusted element. */
    if (!ossl_assert(num == 1 && ctx->num_untrusted == num))

    switch (trust) {
    case X509_TRUST_TRUSTED:
        return 1;
    case X509_TRUST_REJECTED:
        /* Callback already issued */
        return 0;