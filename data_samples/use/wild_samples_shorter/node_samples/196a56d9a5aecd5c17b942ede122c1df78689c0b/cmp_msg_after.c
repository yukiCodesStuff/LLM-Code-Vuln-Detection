    (sk_GENERAL_NAME_num((ctx)->subjectAltNames) > 0 \
         || OSSL_CMP_CTX_reqExtensions_have_SAN(ctx) == 1)

static const X509_NAME *determine_subj(OSSL_CMP_CTX *ctx, int for_KUR,
                                       const X509_NAME *ref_subj)
{
    if (ctx->subjectName != NULL)
        return IS_NULL_DN(ctx->subjectName) ? NULL : ctx->subjectName;
    if (ctx->p10CSR != NULL) /* first default is from any given CSR */
        return X509_REQ_get_subject_name(ctx->p10CSR);
    if (for_KUR || !HAS_SAN(ctx))
        /*
         * For KUR, copy subject from any reference cert as fallback.
         * For IR or CR, do the same only if there is no subjectAltName.
         */
        return ref_subj;
    return NULL;
    EVP_PKEY *rkey = OSSL_CMP_CTX_get0_newPkey(ctx, 0);
    STACK_OF(GENERAL_NAME) *default_sans = NULL;
    const X509_NAME *ref_subj =
        refcert != NULL ? X509_get_subject_name(refcert) : NULL;
    const X509_NAME *subject = determine_subj(ctx, for_KUR, ref_subj);
    const X509_NAME *issuer = ctx->issuer != NULL || refcert == NULL
        ? (IS_NULL_DN(ctx->issuer) ? NULL : ctx->issuer)
        : X509_get_issuer_name(refcert);
    int crit = ctx->setSubjectAltNameCritical || subject == NULL;