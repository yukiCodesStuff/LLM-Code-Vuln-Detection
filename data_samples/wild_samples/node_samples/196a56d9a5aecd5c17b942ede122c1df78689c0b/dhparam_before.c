        if (infile != NULL) {
            BIO_printf(bio_err, "Warning, input file %s ignored\n", infile);
        }

        ctx = EVP_PKEY_CTX_new_from_name(NULL, alg, NULL);
        if (ctx == NULL) {
            BIO_printf(bio_err,
                        "Error, %s param generation context allocation failed\n",
                        alg);
            goto end;
        }
        EVP_PKEY_CTX_set_cb(ctx, gendh_cb);
        EVP_PKEY_CTX_set_app_data(ctx, bio_err);
        BIO_printf(bio_err,
                    "Generating %s parameters, %d bit long %sprime\n",
                    alg, num, dsaparam ? "" : "safe ");

        if (EVP_PKEY_paramgen_init(ctx) <= 0) {
            BIO_printf(bio_err,
                        "Error, unable to initialise %s parameters\n",
                        alg);
            goto end;
        }

        if (dsaparam) {
            if (EVP_PKEY_CTX_set_dsa_paramgen_bits(ctx, num) <= 0) {
                BIO_printf(bio_err, "Error, unable to set DSA prime length\n");
                goto end;
            }
                    && !EVP_PKEY_is_a(tmppkey, "DHX")) {
                BIO_printf(bio_err, "Error, unable to load DH parameters\n");
                goto end;
            }
            pkey = tmppkey;
            tmppkey = NULL;
        }
    }

    if (text)
        EVP_PKEY_print_params(out, pkey, 4, NULL);

    if (check) {
        ctx = EVP_PKEY_CTX_new_from_pkey(NULL, pkey, NULL);
        if (ctx == NULL) {
            BIO_printf(bio_err, "Error, failed to check DH parameters\n");
            goto end;
        }
        if (EVP_PKEY_param_check(ctx) <= 0) {
            BIO_printf(bio_err, "Error, invalid parameters generated\n");
            goto end;
        }
        BIO_printf(bio_err, "DH parameters appear to be ok.\n");
    }
            || (params = OSSL_PARAM_BLD_to_param(tmpl)) == NULL) {
        BIO_printf(bio_err, "Error, failed to set DH parameters\n");
        goto err;
    }

    ctx = EVP_PKEY_CTX_new_from_name(NULL, "DHX", NULL);
    if (ctx == NULL
            || EVP_PKEY_fromdata_init(ctx) <= 0
            || EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_KEY_PARAMETERS, params) <= 0) {
        BIO_printf(bio_err, "Error, failed to set DH parameters\n");
        goto err;
    }

 err:
    EVP_PKEY_CTX_free(ctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(tmpl);
    BN_free(bn_p);
    BN_free(bn_q);
    BN_free(bn_g);
    return pkey;
}

static int gendh_cb(EVP_PKEY_CTX *ctx)
{
    int p = EVP_PKEY_CTX_get_keygen_info(ctx, 0);
    BIO *b = EVP_PKEY_CTX_get_app_data(ctx);
    static const char symbols[] = ".+*\n";
    char c = (p >= 0 && (size_t)p < sizeof(symbols) - 1) ? symbols[p] : '?';

    BIO_write(b, &c, 1);
    (void)BIO_flush(b);
    return 1;
}