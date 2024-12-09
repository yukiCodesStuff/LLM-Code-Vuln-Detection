            BIO_printf(bio_err, "Warning, input file %s ignored\n", infile);
        }

        ctx = EVP_PKEY_CTX_new_from_name(NULL, alg, NULL);
        if (ctx == NULL) {
            BIO_printf(bio_err,
                        "Error, %s param generation context allocation failed\n",
                        alg);
        EVP_PKEY_print_params(out, pkey, 4, NULL);

    if (check) {
        ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);
        if (ctx == NULL) {
            BIO_printf(bio_err, "Error, failed to check DH parameters\n");
            goto end;
        }
        goto err;
    }

    ctx = EVP_PKEY_CTX_new_from_name(NULL, "DHX", NULL);
    if (ctx == NULL
            || EVP_PKEY_fromdata_init(ctx) <= 0
            || EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEY_PARAMETERS, params) <= 0) {
        BIO_printf(bio_err, "Error, failed to set DH parameters\n");