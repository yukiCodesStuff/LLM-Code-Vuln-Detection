    } else {
        dst->seed = NULL;
    }
    dst->nid = src->nid;
    dst->pcounter = src->pcounter;
    dst->h = src->h;
    dst->gindex = src->gindex;
    dst->flags = src->flags;
    dst->keylength = src->keylength;
    return 1;
}

int ossl_ffc_params_cmp(const FFC_PARAMS *a, const FFC_PARAMS *b, int ignore_q)
{
    return BN_cmp(a->p, b->p) == 0
           && BN_cmp(a->g, b->g) == 0
           && (ignore_q || BN_cmp(a->q, b->q) == 0); /* Note: q may be NULL */
}

int ossl_ffc_params_todata(const FFC_PARAMS *ffc, OSSL_PARAM_BLD *bld,
                      OSSL_PARAM params[])
{
    int test_flags;

    if (ffc == NULL)
        return 0;

    if (ffc->p != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_FFC_P, ffc->p))
        return 0;
    if (ffc->q != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_FFC_Q, ffc->q))
        return 0;
    if (ffc->g != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_FFC_G, ffc->g))
        return 0;
    if (ffc->j != NULL
        && !ossl_param_build_set_bn(bld, params, OSSL_PKEY_PARAM_FFC_COFACTOR,
                                    ffc->j))
        return 0;
    if (!ossl_param_build_set_int(bld, params, OSSL_PKEY_PARAM_FFC_GINDEX,
                                  ffc->gindex))
        return 0;
    if (!ossl_param_build_set_int(bld, params, OSSL_PKEY_PARAM_FFC_PCOUNTER,
                                  ffc->pcounter))
        return 0;
    if (!ossl_param_build_set_int(bld, params, OSSL_PKEY_PARAM_FFC_H, ffc->h))
        return 0;
    if (ffc->seed != NULL
        && !ossl_param_build_set_octet_string(bld, params,
                                              OSSL_PKEY_PARAM_FFC_SEED,
                                              ffc->seed, ffc->seedlen))
        return 0;
    if (ffc->nid != NID_undef) {
        const DH_NAMED_GROUP *group = ossl_ffc_uid_to_dh_named_group(ffc->nid);
        const char *name = ossl_ffc_named_group_get_name(group);

        if (name == NULL
            || !ossl_param_build_set_utf8_string(bld, params,
                                                 OSSL_PKEY_PARAM_GROUP_NAME,
                                                 name))
            return 0;
    }