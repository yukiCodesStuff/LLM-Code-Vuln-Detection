        if (ctx->p10CSR == NULL) {
            ERR_raise(ERR_LIB_CMP, CMP_R_MISSING_P10CSR);
            goto err;
        }
        if ((msg->body->value.p10cr = X509_REQ_dup(ctx->p10CSR)) == NULL)
            goto err;
        return msg;

    case OSSL_CMP_PKIBODY_IP:
    case OSSL_CMP_PKIBODY_CP:
    case OSSL_CMP_PKIBODY_KUP:
        if ((msg->body->value.ip = OSSL_CMP_CERTREPMESSAGE_new()) == NULL)
            goto err;
        return msg;

    case OSSL_CMP_PKIBODY_RR:
        if ((msg->body->value.rr = sk_OSSL_CMP_REVDETAILS_new_null()) == NULL)
            goto err;
        return msg;
    case OSSL_CMP_PKIBODY_RP:
        if ((msg->body->value.rp = OSSL_CMP_REVREPCONTENT_new()) == NULL)
            goto err;
        return msg;

    case OSSL_CMP_PKIBODY_CERTCONF:
        if ((msg->body->value.certConf =
             sk_OSSL_CMP_CERTSTATUS_new_null()) == NULL)
            goto err;
        return msg;
    case OSSL_CMP_PKIBODY_PKICONF:
        if ((msg->body->value.pkiconf = ASN1_TYPE_new()) == NULL)
            goto err;
        ASN1_TYPE_set(msg->body->value.pkiconf, V_ASN1_NULL, NULL);
        return msg;

    case OSSL_CMP_PKIBODY_POLLREQ:
        if ((msg->body->value.pollReq = sk_OSSL_CMP_POLLREQ_new_null()) == NULL)
            goto err;
        return msg;
    case OSSL_CMP_PKIBODY_POLLREP:
        if ((msg->body->value.pollRep = sk_OSSL_CMP_POLLREP_new_null()) == NULL)
            goto err;
        return msg;

    case OSSL_CMP_PKIBODY_GENM:
    case OSSL_CMP_PKIBODY_GENP:
        if ((msg->body->value.genm = sk_OSSL_CMP_ITAV_new_null()) == NULL)
            goto err;
        return msg;

    case OSSL_CMP_PKIBODY_ERROR:
        if ((msg->body->value.error = OSSL_CMP_ERRORMSGCONTENT_new()) == NULL)
            goto err;
        return msg;

    default:
        ERR_raise(ERR_LIB_CMP, CMP_R_UNEXPECTED_PKIBODY);
        goto err;
    }

 err:
    OSSL_CMP_MSG_free(msg);
    return NULL;
}

#define HAS_SAN(ctx) \
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
}
{
    OSSL_CRMF_MSG *crm = NULL;
    X509 *refcert = ctx->oldCert != NULL ? ctx->oldCert : ctx->cert;
    /* refcert defaults to current client cert */
    EVP_PKEY *rkey = OSSL_CMP_CTX_get0_newPkey(ctx, 0);
    STACK_OF(GENERAL_NAME) *default_sans = NULL;
    const X509_NAME *ref_subj =
        refcert != NULL ? X509_get_subject_name(refcert) : NULL;
    const X509_NAME *subject = determine_subj(ctx, for_KUR, ref_subj);
    const X509_NAME *issuer = ctx->issuer != NULL || refcert == NULL
        ? (IS_NULL_DN(ctx->issuer) ? NULL : ctx->issuer)
        : X509_get_issuer_name(refcert);
    int crit = ctx->setSubjectAltNameCritical || subject == NULL;
    /* RFC5280: subjectAltName MUST be critical if subject is null */
    X509_EXTENSIONS *exts = NULL;

    if (rkey == NULL && ctx->p10CSR != NULL)
        rkey = X509_REQ_get0_pubkey(ctx->p10CSR);
    if (rkey == NULL && refcert != NULL)
        rkey = X509_get0_pubkey(refcert);
    if (rkey == NULL)
        rkey = ctx->pkey; /* default is independent of ctx->oldCert */
    if (rkey == NULL) {
#ifndef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
        ERR_raise(ERR_LIB_CMP, CMP_R_NULL_ARGUMENT);
        return NULL;
#endif
    }