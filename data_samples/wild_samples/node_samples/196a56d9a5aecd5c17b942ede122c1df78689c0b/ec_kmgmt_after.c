{
    int ret = 0;
    EC_KEY *eck = key;
    const EC_GROUP *ecg = NULL;
    OSSL_PARAM *p;
    unsigned char *pub_key = NULL, *genbuf = NULL;
    OSSL_LIB_CTX *libctx;
    const char *propq;
    BN_CTX *bnctx = NULL;

    ecg = EC_KEY_get0_group(eck);
    if (ecg == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_NO_PARAMETERS_SET);
        return 0;
    }
        if (p != NULL) {
            int ecdh_cofactor_mode = 0;

            ecdh_cofactor_mode =
                (EC_KEY_get_flags(eck) & EC_FLAG_COFACTOR_ECDH) ? 1 : 0;

            if (!OSSL_PARAM_set_int(p, ecdh_cofactor_mode))
                goto err;
        }
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
    }