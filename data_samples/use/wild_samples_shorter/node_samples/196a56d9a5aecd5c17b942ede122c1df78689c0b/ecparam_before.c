        *p = OSSL_PARAM_construct_end();

        if (OPENSSL_strcasecmp(curve_name, "SM2") == 0)
            gctx_params = EVP_PKEY_CTX_new_from_name(NULL, "sm2", NULL);
        else
            gctx_params = EVP_PKEY_CTX_new_from_name(NULL, "ec", NULL);
        if (gctx_params == NULL
            || EVP_PKEY_keygen_init(gctx_params) <= 0
            || EVP_PKEY_CTX_set_params(gctx_params, params) <= 0
            || EVP_PKEY_keygen(gctx_params, &params_key) <= 0) {
                BIO_printf(bio_err, "unable to set check_type\n");
                goto end;
        }
        pctx = EVP_PKEY_CTX_new_from_pkey(NULL, params_key, NULL);
        if (pctx == NULL || EVP_PKEY_param_check(pctx) <= 0) {
            BIO_printf(bio_err, "failed\n");
            goto end;
        }
         *    EVP_PKEY_CTX_set_group_name(gctx, curvename);
         *    EVP_PKEY_keygen(gctx, &key) <= 0)
         */
        gctx_key = EVP_PKEY_CTX_new_from_pkey(NULL, params_key, NULL);
        if (EVP_PKEY_keygen_init(gctx_key) <= 0
            || EVP_PKEY_keygen(gctx_key, &key) <= 0) {
            BIO_printf(bio_err, "unable to generate key\n");
            goto end;