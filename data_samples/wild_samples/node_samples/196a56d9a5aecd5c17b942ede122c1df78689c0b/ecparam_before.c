        } else if (strcmp(curve_name, "secp256r1") == 0) {
            BIO_printf(bio_err,
                       "using curve name prime256v1 instead of secp256r1\n");
            curve_name = SN_X9_62_prime256v1;
        }
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_GROUP_NAME,
                                                curve_name, 0);
        if (asn1_encoding != NULL)
            *p++ = OSSL_PARAM_construct_utf8_string(OSSL_PKEY_PARAM_EC_ENCODING,
                                                    asn1_encoding, 0);
        if (point_format != NULL)
            *p++ = OSSL_PARAM_construct_utf8_string(
                       OSSL_PKEY_PARAM_EC_POINT_CONVERSION_FORMAT,
                       point_format, 0);
        *p = OSSL_PARAM_construct_end();

        if (OPENSSL_strcasecmp(curve_name, "SM2") == 0)
            gctx_params = EVP_PKEY_CTX_new_from_name(NULL, "sm2", NULL);
        else
            gctx_params = EVP_PKEY_CTX_new_from_name(NULL, "ec", NULL);
        if (gctx_params == NULL
            || EVP_PKEY_keygen_init(gctx_params) <= 0
            || EVP_PKEY_CTX_set_params(gctx_params, params) <= 0
            || EVP_PKEY_keygen(gctx_params, &params_key) <= 0) {
            BIO_printf(bio_err, "unable to generate key\n");
            goto end;
        }
                                           OSSL_PKEY_EC_GROUP_CHECK_NAMED)) {
                BIO_printf(bio_err, "unable to set check_type\n");
                goto end;
        }
        pctx = EVP_PKEY_CTX_new_from_pkey(NULL, params_key, NULL);
        if (pctx == NULL || EVP_PKEY_param_check(pctx) <= 0) {
            BIO_printf(bio_err, "failed\n");
            goto end;
        }
        BIO_printf(bio_err, "ok\n");
    }

    if (outformat == FORMAT_ASN1 && genkey)
        noout = 1;

    if (!noout) {
        ectx_params = OSSL_ENCODER_CTX_new_for_pkey(
                          params_key, OSSL_KEYMGMT_SELECT_DOMAIN_PARAMETERS,
                          outformat == FORMAT_ASN1 ? "DER" : "PEM", NULL, NULL);
        if (!OSSL_ENCODER_to_bio(ectx_params, out)) {
            BIO_printf(bio_err, "unable to write elliptic curve parameters\n");
            goto end;
        }
    }

    if (genkey) {
        /*
         * NOTE: EC keygen does not normally need to pass in the param_key
         * for named curves. This can be achieved using:
         *    gctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
         *    EVP_PKEY_keygen_init(gctx);
         *    EVP_PKEY_CTX_set_group_name(gctx, curvename);
         *    EVP_PKEY_keygen(gctx, &key) <= 0)
         */
        gctx_key = EVP_PKEY_CTX_new_from_pkey(NULL, params_key, NULL);
        if (EVP_PKEY_keygen_init(gctx_key) <= 0
            || EVP_PKEY_keygen(gctx_key, &key) <= 0) {
            BIO_printf(bio_err, "unable to generate key\n");
            goto end;
        }
        assert(private);
        ectx_key = OSSL_ENCODER_CTX_new_for_pkey(
                       key, OSSL_KEYMGMT_SELECT_ALL,
                       outformat == FORMAT_ASN1 ? "DER" : "PEM", NULL, NULL);
        if (!OSSL_ENCODER_to_bio(ectx_key, out)) {
            BIO_printf(bio_err, "unable to write elliptic "
                       "curve parameters\n");
            goto end;
        }
    }

    ret = 0;
end:
    if (ret != 0)
        ERR_print_errors(bio_err);
    release_engine(e);
    EVP_PKEY_free(params_key);
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_CTX_free(gctx_params);
    EVP_PKEY_CTX_free(gctx_key);
    OSSL_DECODER_CTX_free(dctx_params);
    OSSL_ENCODER_CTX_free(ectx_params);
    OSSL_ENCODER_CTX_free(ectx_key);
    BIO_free_all(out);
    return ret;
}
    if (genkey) {
        /*
         * NOTE: EC keygen does not normally need to pass in the param_key
         * for named curves. This can be achieved using:
         *    gctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
         *    EVP_PKEY_keygen_init(gctx);
         *    EVP_PKEY_CTX_set_group_name(gctx, curvename);
         *    EVP_PKEY_keygen(gctx, &key) <= 0)
         */
        gctx_key = EVP_PKEY_CTX_new_from_pkey(NULL, params_key, NULL);
        if (EVP_PKEY_keygen_init(gctx_key) <= 0
            || EVP_PKEY_keygen(gctx_key, &key) <= 0) {
            BIO_printf(bio_err, "unable to generate key\n");
            goto end;
        }