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
    if (ecg == NULL)
        return 0;

    libctx = ossl_ec_key_get_libctx(eck);
    propq = ossl_ec_key_get0_propq(eck);

    bnctx = BN_CTX_new_ex(libctx);
    if (bnctx == NULL)
        return 0;
    BN_CTX_start(bnctx);

    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_MAX_SIZE)) != NULL
        && !OSSL_PARAM_set_int(p, ECDSA_size(eck)))
        goto err;
    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_BITS)) != NULL
        && !OSSL_PARAM_set_int(p, EC_GROUP_order_bits(ecg)))
        goto err;
    if ((p = OSSL_PARAM_locate(params, OSSL_PKEY_PARAM_SECURITY_BITS)) != NULL) {
        int ecbits, sec_bits;

        ecbits = EC_GROUP_order_bits(ecg);

        /*
         * The following estimates are based on the values published
         * in Table 2 of "NIST Special Publication 800-57 Part 1 Revision 4"
         * at http://dx.doi.org/10.6028/NIST.SP.800-57pt1r4 .
         *
         * Note that the above reference explicitly categorizes algorithms in a
         * discrete set of values {80, 112, 128, 192, 256}, and that it is
         * relevant only for NIST approved Elliptic Curves, while OpenSSL
         * applies the same logic also to other curves.
         *
         * Classifications produced by other standardazing bodies might differ,
         * so the results provided for "bits of security" by this provider are
         * to be considered merely indicative, and it is the users'
         * responsibility to compare these values against the normative
         * references that may be relevant for their intent and purposes.
         */
        if (ecbits >= 512)
            sec_bits = 256;
        else if (ecbits >= 384)
            sec_bits = 192;
        else if (ecbits >= 256)
            sec_bits = 128;
        else if (ecbits >= 224)
            sec_bits = 112;
        else if (ecbits >= 160)
            sec_bits = 80;
        else
            sec_bits = ecbits / 2;

        if (!OSSL_PARAM_set_int(p, sec_bits))
            goto err;
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
        p->return_size = EC_POINT_point2oct(EC_KEY_get0_group(key),
                                            EC_KEY_get0_public_key(key),
                                            POINT_CONVERSION_UNCOMPRESSED,
                                            p->data, p->return_size, bnctx);
        if (p->return_size == 0)
            goto err;
    }