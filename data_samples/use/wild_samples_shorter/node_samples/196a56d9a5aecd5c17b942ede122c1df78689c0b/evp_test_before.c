                  expected->mac_name, expected->alg);

    if (expected->alg != NULL) {
        /*
         * The underlying algorithm may be a cipher or a digest.
         * We don't know which it is, but we can ask the MAC what it
         * should be and bet on that.
         */
        if (OSSL_PARAM_locate_const(defined_params,
                                    OSSL_MAC_PARAM_CIPHER) != NULL) {
            params[params_n++] =
                OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_CIPHER,
                                                 expected->alg, 0);
        } else if (OSSL_PARAM_locate_const(defined_params,
                                           OSSL_MAC_PARAM_DIGEST) != NULL) {
            params[params_n++] =
                OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
                                                 expected->alg, 0);
        } else {
            t->err = "MAC_BAD_PARAMS";
            goto err;
        }
    }
    if (expected->custom != NULL)
        params[params_n++] =
            OSSL_PARAM_construct_octet_string(OSSL_MAC_PARAM_CUSTOM,
            goto err;
        }
    }
    if (reinit--) {
        OSSL_PARAM ivparams[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
        int ret;

        /* If the MAC uses IV, we have to set it again */
    unsigned char *got = NULL;
    size_t got_len = 0;

    /* Find out the KDF output size */
    if (EVP_PKEY_derive(expected->ctx, NULL, &got_len) <= 0) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }

    /*
     * We may get an absurd output size, which signals that anything goes.
     * If not, we specify a too big buffer for the output, to test that
     * EVP_PKEY_derive() can cope with it.
     */
    if (got_len == SIZE_MAX || got_len == 0)
        got_len = expected->output_len;
    else
        got_len = expected->output_len * 2;

    if (!TEST_ptr(got = OPENSSL_malloc(got_len == 0 ? 1 : got_len))) {
        t->err = "INTERNAL_ERROR";
        goto err;
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSignFinal(expected->ctx, got, &got_len)) {
        t->err = "DIGESTSIGNFINAL_ERROR";
        goto err;
    }
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSign(expected->ctx, got, &got_len,
                        expected->osin, expected->osin_len)) {
        t->err = "DIGESTSIGN_ERROR";
        goto err;
    KEY_LIST *key, **klist;
    EVP_PKEY *pkey;
    PAIR *pp;
    int i, skip_availablein = 0;

top:
    do {
        if (BIO_eof(t->s.fp))
                t->skip = 1;
                return 0;
        }
        skip_availablein++;
        pp++;
        goto start;
    }

        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())