{
    int err = ossl_x509_likely_issued(issuer, x);

    if (err == X509_V_OK)
        return 1;
    /*
     * SUBJECT_ISSUER_MISMATCH just means 'x' is clearly not issued by 'issuer'.
     * Every other error code likely indicates a real error.
     */
    if (err != X509_V_ERR_SUBJECT_ISSUER_MISMATCH)
        ctx->error = err;
    return 0;
}

/*-
 * Alternative get_issuer method: look up from a STACK_OF(X509) in other_ctx.
 * Returns -1 on internal error.
 */
static int get_issuer_sk(X509 **issuer, X509_STORE_CTX *ctx, X509 *x)
{
    *issuer = find_issuer(ctx, ctx->other_ctx, x);
    if (*issuer != NULL)
        return X509_up_ref(*issuer) ? 1 : -1;
    return 0;
}

/*-
 * Alternative lookup method: look from a STACK stored in other_ctx.
 * Returns NULL on internal error (such as out of memory).
 */
static STACK_OF(X509) *lookup_certs_sk(X509_STORE_CTX *ctx,
                                       const X509_NAME *nm)
{
    STACK_OF(X509) *sk = sk_X509_new_null();
    X509 *x;
    int i;

    if (sk == NULL)
        return NULL;
    for (i = 0; i < sk_X509_num(ctx->other_ctx); i++) {
        x = sk_X509_value(ctx->other_ctx, i);
        if (X509_NAME_cmp(nm, X509_get_subject_name(x)) == 0) {
            if (!X509_add_cert(sk, x, X509_ADD_FLAG_UP_REF)) {
                sk_X509_pop_free(sk, X509_free);
                ctx->error = X509_V_ERR_OUT_OF_MEM;
                return NULL;
            }
        }
    }
{
    time_t *ptime;
    int i;

    if (notify)
        ctx->current_crl = crl;
    if ((ctx->param->flags & X509_V_FLAG_USE_CHECK_TIME) != 0)
        ptime = &ctx->param->check_time;
    else if ((ctx->param->flags & X509_V_FLAG_NO_CHECK_TIME) != 0)
        return 1;
    else
        ptime = NULL;

    i = X509_cmp_time(X509_CRL_get0_lastUpdate(crl), ptime);
    if (i == 0) {
        if (!notify)
            return 0;
        if (!verify_cb_crl(ctx, X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD))
            return 0;
    }
{
    if (ctx == NULL)
        return;

    X509_STORE_CTX_cleanup(ctx);

    /* libctx and propq survive X509_STORE_CTX_cleanup() */
    OPENSSL_free(ctx->propq);
    OPENSSL_free(ctx);
}

int X509_STORE_CTX_init(X509_STORE_CTX *ctx, X509_STORE *store, X509 *x509,
                        STACK_OF(X509) *chain)
{
    int ret = 1;

    if (ctx == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    X509_STORE_CTX_cleanup(ctx);

    ctx->store = store;
    ctx->cert = x509;
    ctx->untrusted = chain;
    ctx->crls = NULL;
    ctx->num_untrusted = 0;
    ctx->other_ctx = NULL;
    ctx->valid = 0;
    ctx->chain = NULL;
    ctx->error = X509_V_OK;
    ctx->explicit_policy = 0;
    ctx->error_depth = 0;
    ctx->current_cert = NULL;
    ctx->current_issuer = NULL;
    ctx->current_crl = NULL;
    ctx->current_crl_score = 0;
    ctx->current_reasons = 0;
    ctx->tree = NULL;
    ctx->parent = NULL;
    ctx->dane = NULL;
    ctx->bare_ta_signed = 0;
    /* Zero ex_data to make sure we're cleanup-safe */
    memset(&ctx->ex_data, 0, sizeof(ctx->ex_data));

    /* store->cleanup is always 0 in OpenSSL, if set must be idempotent */
    if (store != NULL)
        ctx->cleanup = store->cleanup;
    else
        ctx->cleanup = NULL;

    if (store != NULL && store->check_issued != NULL)
        ctx->check_issued = store->check_issued;
    else
        ctx->check_issued = check_issued;

    if (store != NULL && store->get_issuer != NULL)
        ctx->get_issuer = store->get_issuer;
    else
        ctx->get_issuer = X509_STORE_CTX_get1_issuer;

    if (store != NULL && store->verify_cb != NULL)
        ctx->verify_cb = store->verify_cb;
    else
        ctx->verify_cb = null_callback;

    if (store != NULL && store->verify != NULL)
        ctx->verify = store->verify;
    else
        ctx->verify = internal_verify;

    if (store != NULL && store->check_revocation != NULL)
        ctx->check_revocation = store->check_revocation;
    else
        ctx->check_revocation = check_revocation;

    if (store != NULL && store->get_crl != NULL)
        ctx->get_crl = store->get_crl;
    else
        ctx->get_crl = NULL;

    if (store != NULL && store->check_crl != NULL)
        ctx->check_crl = store->check_crl;
    else
        ctx->check_crl = check_crl;

    if (store != NULL && store->cert_crl != NULL)
        ctx->cert_crl = store->cert_crl;
    else
        ctx->cert_crl = cert_crl;

    if (store != NULL && store->check_policy != NULL)
        ctx->check_policy = store->check_policy;
    else
        ctx->check_policy = check_policy;

    if (store != NULL && store->lookup_certs != NULL)
        ctx->lookup_certs = store->lookup_certs;
    else
        ctx->lookup_certs = X509_STORE_CTX_get1_certs;

    if (store != NULL && store->lookup_crls != NULL)
        ctx->lookup_crls = store->lookup_crls;
    else
        ctx->lookup_crls = X509_STORE_CTX_get1_crls;

    ctx->param = X509_VERIFY_PARAM_new();
    if (ctx->param == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    /* Inherit callbacks and flags from X509_STORE if not set use defaults. */
    if (store != NULL)
        ret = X509_VERIFY_PARAM_inherit(ctx->param, store->param);
    else
        ctx->param->inh_flags |= X509_VP_FLAG_DEFAULT | X509_VP_FLAG_ONCE;

    if (ret)
        ret = X509_VERIFY_PARAM_inherit(ctx->param,
                                        X509_VERIFY_PARAM_lookup("default"));

    if (ret == 0) {
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    /*
     * XXX: For now, continue to inherit trust from VPM, but infer from the
     * purpose if this still yields the default value.
     */
    if (ctx->param->trust == X509_TRUST_DEFAULT) {
        int idx = X509_PURPOSE_get_by_id(ctx->param->purpose);
        X509_PURPOSE *xp = X509_PURPOSE_get0(idx);

        if (xp != NULL)
            ctx->param->trust = X509_PURPOSE_get_trust(xp);
    }

    if (CRYPTO_new_ex_data(CRYPTO_EX_INDEX_X509_STORE_CTX, ctx,
                           &ctx->ex_data))
        return 1;
    ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);

 err:
    /*
     * On error clean up allocated storage, if the store context was not
     * allocated with X509_STORE_CTX_new() this is our last chance to do so.
     */
    X509_STORE_CTX_cleanup(ctx);
    return 0;
}
    if (ctx->param == NULL) {
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    /* Inherit callbacks and flags from X509_STORE if not set use defaults. */
    if (store != NULL)
        ret = X509_VERIFY_PARAM_inherit(ctx->param, store->param);
    else
        ctx->param->inh_flags |= X509_VP_FLAG_DEFAULT | X509_VP_FLAG_ONCE;

    if (ret)
        ret = X509_VERIFY_PARAM_inherit(ctx->param,
                                        X509_VERIFY_PARAM_lookup("default"));

    if (ret == 0) {
        ERR_raise(ERR_LIB_X509, ERR_R_MALLOC_FAILURE);
        goto err;
    }
{
    const X509_VERIFY_PARAM *param;

    param = X509_VERIFY_PARAM_lookup(name);
    if (param == NULL)
        return 0;
    return X509_VERIFY_PARAM_inherit(ctx->param, param);
}

X509_VERIFY_PARAM *X509_STORE_CTX_get0_param(const X509_STORE_CTX *ctx)
{
    return ctx->param;
}

void X509_STORE_CTX_set0_param(X509_STORE_CTX *ctx, X509_VERIFY_PARAM *param)
{
    X509_VERIFY_PARAM_free(ctx->param);
    ctx->param = param;
}

void X509_STORE_CTX_set0_dane(X509_STORE_CTX *ctx, SSL_DANE *dane)
{
    ctx->dane = dane;
}

static unsigned char *dane_i2d(X509 *cert, uint8_t selector,
                               unsigned int *i2dlen)
{
    unsigned char *buf = NULL;
    int len;

    /*
     * Extract ASN.1 DER form of certificate or public key.
     */
    switch (selector) {
    case DANETLS_SELECTOR_CERT:
        len = i2d_X509(cert, &buf);
        break;
    case DANETLS_SELECTOR_SPKI:
        len = i2d_X509_PUBKEY(X509_get_X509_PUBKEY(cert), &buf);
        break;
    default:
        ERR_raise(ERR_LIB_X509, X509_R_BAD_SELECTOR);
        return NULL;
    }
{
    SSL_DANE *dane = ctx->dane;
    int num = sk_X509_num(ctx->chain);
    STACK_OF(X509) *sk_untrusted = NULL;
    unsigned int search;
    int may_trusted = 0;
    int may_alternate = 0;
    int trust = X509_TRUST_UNTRUSTED;
    int alt_untrusted = 0;
    int max_depth;
    int ok = 0;
    int prev_error = ctx->error;
    int i;

    /* Our chain starts with a single untrusted element. */
    if (!ossl_assert(num == 1 && ctx->num_untrusted == num))
        goto int_err;

#define S_DOUNTRUSTED (1 << 0) /* Search untrusted chain */
#define S_DOTRUSTED   (1 << 1) /* Search trusted store */
#define S_DOALTERNATE (1 << 2) /* Retry with pruned alternate chain */
    /*
     * Set up search policy, untrusted if possible, trusted-first if enabled,
     * which is the default.
     * If we're doing DANE and not doing PKIX-TA/PKIX-EE, we never look in the
     * trust_store, otherwise we might look there first.  If not trusted-first,
     * and alternate chains are not disabled, try building an alternate chain
     * if no luck with untrusted first.
     */
    search = ctx->untrusted != NULL ? S_DOUNTRUSTED : 0;
    if (DANETLS_HAS_PKIX(dane) || !DANETLS_HAS_DANE(dane)) {
        if (search == 0 || (ctx->param->flags & X509_V_FLAG_TRUSTED_FIRST) != 0)
            search |= S_DOTRUSTED;
        else if (!(ctx->param->flags & X509_V_FLAG_NO_ALT_CHAINS))
            may_alternate = 1;
        may_trusted = 1;
    }
    switch (trust) {
    case X509_TRUST_TRUSTED:
        /* Must restore any previous error value for backward compatibility */
        ctx->error = prev_error;
        return 1;
    case X509_TRUST_REJECTED:
        /* Callback already issued */
        return 0;
    case X509_TRUST_UNTRUSTED:
    default:
        switch(ctx->error) {
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
        case X509_V_ERR_CERT_HAS_EXPIRED:
            return 0; /* Callback already issued by ossl_x509_check_cert_time() */
        default: /* A preliminary error has become final */
            return verify_cb_cert(ctx, NULL, num - 1, ctx->error);
        case X509_V_OK:
            break;
        }
        CB_FAIL_IF(num > max_depth,
                   ctx, NULL, num - 1, X509_V_ERR_CERT_CHAIN_TOO_LONG);
        CB_FAIL_IF(DANETLS_ENABLED(dane)
                       && (!DANETLS_HAS_PKIX(dane) || dane->pdpth >= 0),
                   ctx, NULL, num - 1, X509_V_ERR_DANE_NO_MATCH);
        if (X509_self_signed(sk_X509_value(ctx->chain, num - 1), 0) > 0)
            return verify_cb_cert(ctx, NULL, num - 1,
                                  num == 1
                                  ? X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT
                                  : X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN);
        return verify_cb_cert(ctx, NULL, num - 1,
                              ctx->num_untrusted < num
                              ? X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT
                              : X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY);
    }