    BN_CTX *bnctx = NULL;

    ecg = EC_KEY_get0_group(eck);
    if (ecg == NULL)
        return 0;

    libctx = ossl_ec_key_get_libctx(eck);
    propq = ossl_ec_key_get0_propq(eck);

    }
    if ((p = OSSL_PARAM_locate(params,
                               OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY)) != NULL) {
        p->return_size = EC_POINT_point2oct(EC_KEY_get0_group(key),
                                            EC_KEY_get0_public_key(key),
                                            POINT_CONVERSION_UNCOMPRESSED,
                                            p->data, p->return_size, bnctx);
        if (p->return_size == 0)
            goto err;