    if (prm != NULL) {
        /*
         * In a no-dh build we just go straight to err because we have no
         * support for this.
         */
#ifndef OPENSSL_NO_DH
        const DH_NAMED_GROUP *group = NULL;

        if (prm->data_type != OSSL_PARAM_UTF8_STRING
            || prm->data == NULL
            || (group = ossl_ffc_name_to_dh_named_group(prm->data)) == NULL
            || !ossl_ffc_named_group_set_pqg(ffc, group))
#endif
            goto err;
    }

    param_p = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_P);
    param_g = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_G);
    param_q = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_Q);

    if ((param_p != NULL && !OSSL_PARAM_get_BN(param_p, &p))
        || (param_q != NULL && !OSSL_PARAM_get_BN(param_q, &q))
        || (param_g != NULL && !OSSL_PARAM_get_BN(param_g, &g)))
        goto err;

    prm = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_GINDEX);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ffc->gindex =  i;
    }
    prm = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_PCOUNTER);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ffc->pcounter = i;
    }
    prm = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_COFACTOR);
    if (prm != NULL && !OSSL_PARAM_get_BN(prm, &j))
        goto err;
    prm = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_H);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ffc->h =  i;
    }
    prm  = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_SEED);
    if (prm != NULL) {
        if (prm->data_type != OSSL_PARAM_OCTET_STRING)
            goto err;
        if (!ossl_ffc_params_set_seed(ffc, prm->data, prm->data_size))
            goto err;
    }
    prm  = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_VALIDATE_PQ);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ossl_ffc_params_enable_flags(ffc, FFC_PARAM_FLAG_VALIDATE_PQ, i);
    }
    prm  = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_VALIDATE_G);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ossl_ffc_params_enable_flags(ffc, FFC_PARAM_FLAG_VALIDATE_G, i);
    }
    prm  = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_VALIDATE_LEGACY);
    if (prm != NULL) {
        if (!OSSL_PARAM_get_int(prm, &i))
            goto err;
        ossl_ffc_params_enable_flags(ffc, FFC_PARAM_FLAG_VALIDATE_LEGACY, i);
    }

    prm = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_DIGEST);
    if (prm != NULL) {
        const OSSL_PARAM *p1;
        const char *props = NULL;

        if (prm->data_type != OSSL_PARAM_UTF8_STRING)
            goto err;
        p1 = OSSL_PARAM_locate_const(params, OSSL_PKEY_PARAM_FFC_DIGEST_PROPS);
        if (p1 != NULL) {
            if (p1->data_type != OSSL_PARAM_UTF8_STRING)
                goto err;
        }