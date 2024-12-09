    (sk_GENERAL_NAME_num((ctx)->subjectAltNames) > 0 \
         || OSSL_CMP_CTX_reqExtensions_have_SAN(ctx) == 1)

static const X509_NAME *determine_subj(OSSL_CMP_CTX *ctx,
                                       const X509_NAME *ref_subj,
                                       int for_KUR)
{
    if (ctx->subjectName != NULL)
        return IS_NULL_DN(ctx->subjectName) ? NULL : ctx->subjectName;

    if (ref_subj != NULL && (ctx->p10CSR != NULL || for_KUR || !HAS_SAN(ctx)))
        /*
         * For KUR, copy subject from the reference.
         * For IR or CR, do the same only if there is no subjectAltName.
         */
        return ref_subj;
    return NULL;
    EVP_PKEY *rkey = OSSL_CMP_CTX_get0_newPkey(ctx, 0);
    STACK_OF(GENERAL_NAME) *default_sans = NULL;
    const X509_NAME *ref_subj =
        ctx->p10CSR != NULL ? X509_REQ_get_subject_name(ctx->p10CSR) :
        refcert != NULL ? X509_get_subject_name(refcert) : NULL;
    const X509_NAME *subject = determine_subj(ctx, ref_subj, for_KUR);
    const X509_NAME *issuer = ctx->issuer != NULL || refcert == NULL
        ? (IS_NULL_DN(ctx->issuer) ? NULL : ctx->issuer)
        : X509_get_issuer_name(refcert);
    int crit = ctx->setSubjectAltNameCritical || subject == NULL;