    BN_CTX *bnctx = NULL;

    ecg = EC_KEY_get0_group(eck);
    if (ecg == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_NO_PARAMETERS_SET);
        return 0;
    }

    libctx = ossl_ec_key_get_libctx(eck);
    propq = ossl_ec_key_get0_propq(eck);

    }
    if ((p = OSSL_PARAM_locate(params,
                               OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY)) != NULL) {
        const EC_POINT *ecp = EC_KEY_get0_public_key(key);

        if (ecp == NULL) {
            ERR_raise(ERR_LIB_PROV, PROV_R_NOT_A_PUBLIC_KEY);
            goto err;
        }
        p->return_size = EC_POINT_point2oct(ecg, ecp,
                                            POINT_CONVERSION_UNCOMPRESSED,
                                            p->data, p->return_size, bnctx);
        if (p->return_size == 0)
            goto err;