{
    MAC_DATA *expected = t->data;
    EVP_MAC_CTX *ctx = NULL;
    unsigned char *got = NULL;
    size_t got_len = 0, size = 0;
    int i, block_size = -1, output_size = -1;
    OSSL_PARAM params[21], sizes[3], *psizes = sizes;
    size_t params_n = 0;
    size_t params_n_allocstart = 0;
    const OSSL_PARAM *defined_params =
        EVP_MAC_settable_ctx_params(expected->mac);
    int xof;
    int reinit = 1;

    if (expected->alg == NULL)
        TEST_info("Trying the EVP_MAC %s test", expected->mac_name);
    else
        TEST_info("Trying the EVP_MAC %s test with %s",
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
                                              expected->custom,
                                              expected->custom_len);
    if (expected->salt != NULL)
        params[params_n++] =
            OSSL_PARAM_construct_octet_string(OSSL_MAC_PARAM_SALT,
                                              expected->salt,
                                              expected->salt_len);
    if (expected->iv != NULL)
        params[params_n++] =
            OSSL_PARAM_construct_octet_string(OSSL_MAC_PARAM_IV,
                                              expected->iv,
                                              expected->iv_len);

    /* Unknown controls.  They must match parameters that the MAC recognizes */
    if (params_n + sk_OPENSSL_STRING_num(expected->controls)
        >= OSSL_NELEM(params)) {
        t->err = "MAC_TOO_MANY_PARAMETERS";
        goto err;
    }
    params_n_allocstart = params_n;
    for (i = 0; i < sk_OPENSSL_STRING_num(expected->controls); i++) {
        char *tmpkey, *tmpval;
        char *value = sk_OPENSSL_STRING_value(expected->controls, i);

        if (!TEST_ptr(tmpkey = OPENSSL_strdup(value))) {
            t->err = "MAC_PARAM_ERROR";
            goto err;
        }
        tmpval = strchr(tmpkey, ':');
        if (tmpval != NULL)
            *tmpval++ = '\0';

        if (tmpval == NULL
            || !OSSL_PARAM_allocate_from_text(&params[params_n],
                                              defined_params,
                                              tmpkey, tmpval,
                                              strlen(tmpval), NULL)) {
            OPENSSL_free(tmpkey);
            t->err = "MAC_PARAM_ERROR";
            goto err;
        }
        params_n++;

        OPENSSL_free(tmpkey);
    }
    params[params_n] = OSSL_PARAM_construct_end();

    if ((ctx = EVP_MAC_CTX_new(expected->mac)) == NULL) {
        t->err = "MAC_CREATE_ERROR";
        goto err;
    }

    if (!EVP_MAC_init(ctx, expected->key, expected->key_len, params)) {
        t->err = "MAC_INIT_ERROR";
        goto err;
    }
    if (expected->output_size >= 0)
        *psizes++ = OSSL_PARAM_construct_int(OSSL_MAC_PARAM_SIZE,
                                             &output_size);
    if (expected->block_size >= 0)
        *psizes++ = OSSL_PARAM_construct_int(OSSL_MAC_PARAM_BLOCK_SIZE,
                                             &block_size);
    if (psizes != sizes) {
        *psizes = OSSL_PARAM_construct_end();
        if (!TEST_true(EVP_MAC_CTX_get_params(ctx, sizes))) {
            t->err = "INTERNAL_ERROR";
            goto err;
        }
        if (expected->output_size >= 0
                && !TEST_int_eq(output_size, expected->output_size)) {
            t->err = "TEST_FAILURE";
            goto err;
        }
        if (expected->block_size >= 0
                && !TEST_int_eq(block_size, expected->block_size)) {
            t->err = "TEST_FAILURE";
            goto err;
        }
    }
 retry:
    if (!EVP_MAC_update(ctx, expected->input, expected->input_len)) {
        t->err = "MAC_UPDATE_ERROR";
        goto err;
    }
    xof = expected->xof;
    if (xof) {
        if (!TEST_ptr(got = OPENSSL_malloc(expected->output_len))) {
            t->err = "TEST_FAILURE";
            goto err;
        }
        if (!EVP_MAC_finalXOF(ctx, got, expected->output_len)
            || !memory_err_compare(t, "TEST_MAC_ERR",
                                   expected->output, expected->output_len,
                                   got, expected->output_len)) {
            t->err = "MAC_FINAL_ERROR";
            goto err;
        }
    } else {
        if (!EVP_MAC_final(ctx, NULL, &got_len, 0)) {
            t->err = "MAC_FINAL_LENGTH_ERROR";
            goto err;
        }
        if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
            t->err = "TEST_FAILURE";
            goto err;
        }
        if (!EVP_MAC_final(ctx, got, &got_len, got_len)
            || !memory_err_compare(t, "TEST_MAC_ERR",
                                   expected->output, expected->output_len,
                                   got, got_len)) {
            t->err = "TEST_MAC_ERR";
            goto err;
        }
    }
    if (reinit--) {
        OSSL_PARAM ivparams[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
        int ret;

        /* If the MAC uses IV, we have to set it again */
        if (expected->iv != NULL) {
            ivparams[0] =
                OSSL_PARAM_construct_octet_string(OSSL_MAC_PARAM_IV,
                                                  expected->iv,
                                                  expected->iv_len);
            ivparams[1] = OSSL_PARAM_construct_end();
        }
        ERR_set_mark();
        ret = EVP_MAC_init(ctx, NULL, 0, ivparams);
        if (expected->no_reinit) {
            if (ret) {
                ERR_clear_last_mark();
                t->err = "MAC_REINIT_SHOULD_FAIL";
                goto err;
            }
        } else if (ret) {
            ERR_clear_last_mark();
            OPENSSL_free(got);
            got = NULL;
            goto retry;
        } else {
            ERR_clear_last_mark();
            t->err = "MAC_REINIT_ERROR";
            goto err;
        }
        /* If reinitialization fails, it is unsupported by the algorithm */
        ERR_pop_to_mark();
    }
    t->err = NULL;

    /* Test the EVP_Q_mac interface as well */
    if (!xof) {
        OPENSSL_cleanse(got, got_len);
        if (!TEST_true(EVP_Q_mac(libctx, expected->mac_name, NULL,
                                 expected->alg, params,
                                 expected->key, expected->key_len,
                                 expected->input, expected->input_len,
                                 got, got_len, &size))
                || !TEST_mem_eq(got, size,
                                expected->output, expected->output_len)) {
            t->err = "EVP_Q_mac failed";
            goto err;
        }
    }
 err:
    while (params_n-- > params_n_allocstart) {
        OPENSSL_free(params[params_n].data);
    }
    EVP_MAC_CTX_free(ctx);
    OPENSSL_free(got);
    return 1;
}

static int mac_test_run(EVP_TEST *t)
{
    MAC_DATA *expected = t->data;

    if (expected->mac != NULL)
        return mac_test_run_mac(t);
    return mac_test_run_pkey(t);
}

static const EVP_TEST_METHOD mac_test_method = {
    "MAC",
    mac_test_init,
    mac_test_cleanup,
    mac_test_parse,
    mac_test_run
};


/**
 **  PUBLIC KEY TESTS
 **  These are all very similar and share much common code.
 **/

typedef struct pkey_data_st {
    /* Context for this operation */
    EVP_PKEY_CTX *ctx;
    /* Key operation to perform */
    int (*keyop) (EVP_PKEY_CTX *ctx,
                  unsigned char *sig, size_t *siglen,
                  const unsigned char *tbs, size_t tbslen);
    /* Input to MAC */
    unsigned char *input;
    size_t input_len;
    /* Expected output */
    unsigned char *output;
    size_t output_len;
} PKEY_DATA;

/*
 * Perform public key operation setup: lookup key, allocated ctx and call
 * the appropriate initialisation function
 */
static int pkey_test_init(EVP_TEST *t, const char *name,
                          int use_public,
                          int (*keyopinit) (EVP_PKEY_CTX *ctx),
                          int (*keyop)(EVP_PKEY_CTX *ctx,
                                       unsigned char *sig, size_t *siglen,
                                       const unsigned char *tbs,
                                       size_t tbslen))
{
    PKEY_DATA *kdata;
    EVP_PKEY *pkey = NULL;
    int rv = 0;

    if (use_public)
        rv = find_key(&pkey, name, public_keys);
    if (rv == 0)
        rv = find_key(&pkey, name, private_keys);
    if (rv == 0 || pkey == NULL) {
        TEST_info("skipping, key '%s' is disabled", name);
        t->skip = 1;
        return 1;
    }

    if (!TEST_ptr(kdata = OPENSSL_zalloc(sizeof(*kdata)))) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    kdata->keyop = keyop;
    if (!TEST_ptr(kdata->ctx = EVP_PKEY_CTX_new_from_pkey(libctx, pkey, NULL))) {
        EVP_PKEY_free(pkey);
        OPENSSL_free(kdata);
        return 0;
    }
    if (keyopinit(kdata->ctx) <= 0)
        t->err = "KEYOP_INIT_ERROR";
    t->data = kdata;
    return 1;
}

static void pkey_test_cleanup(EVP_TEST *t)
{
    PKEY_DATA *kdata = t->data;

    OPENSSL_free(kdata->input);
    OPENSSL_free(kdata->output);
    EVP_PKEY_CTX_free(kdata->ctx);
}

static int pkey_test_ctrl(EVP_TEST *t, EVP_PKEY_CTX *pctx,
                          const char *value)
{
    int rv = 0;
    char *p, *tmpval;

    if (!TEST_ptr(tmpval = OPENSSL_strdup(value)))
        return 0;
    p = strchr(tmpval, ':');
    if (p != NULL) {
        *p++ = '\0';
        rv = EVP_PKEY_CTX_ctrl_str(pctx, tmpval, p);
    }
    if (rv == -2) {
        t->err = "PKEY_CTRL_INVALID";
        rv = 1;
    } else if (p != NULL && rv <= 0) {
        if (is_digest_disabled(p) || is_cipher_disabled(p)) {
            TEST_info("skipping, '%s' is disabled", p);
            t->skip = 1;
            rv = 1;
        } else {
            t->err = "PKEY_CTRL_ERROR";
            rv = 1;
        }
    }
    OPENSSL_free(tmpval);
    return rv > 0;
}

static int pkey_test_parse(EVP_TEST *t,
                           const char *keyword, const char *value)
{
    PKEY_DATA *kdata = t->data;
    if (strcmp(keyword, "Input") == 0)
        return parse_bin(value, &kdata->input, &kdata->input_len);
    if (strcmp(keyword, "Output") == 0)
        return parse_bin(value, &kdata->output, &kdata->output_len);
    if (strcmp(keyword, "Ctrl") == 0)
        return pkey_test_ctrl(t, kdata->ctx, value);
    return 0;
}

static int pkey_test_run(EVP_TEST *t)
{
    PKEY_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len;
    EVP_PKEY_CTX *copy = NULL;

    if (expected->keyop(expected->ctx, NULL, &got_len,
                        expected->input, expected->input_len) <= 0
            || !TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "KEYOP_LENGTH_ERROR";
        goto err;
    }
    if (expected->keyop(expected->ctx, got, &got_len,
                        expected->input, expected->input_len) <= 0) {
        t->err = "KEYOP_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "KEYOP_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
    OPENSSL_free(got);
    got = NULL;

    /* Repeat the test on a copy. */
    if (!TEST_ptr(copy = EVP_PKEY_CTX_dup(expected->ctx))) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }
    if (expected->keyop(copy, NULL, &got_len, expected->input,
                        expected->input_len) <= 0
            || !TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "KEYOP_LENGTH_ERROR";
        goto err;
    }
    if (expected->keyop(copy, got, &got_len, expected->input,
                        expected->input_len) <= 0) {
        t->err = "KEYOP_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "KEYOP_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

 err:
    OPENSSL_free(got);
    EVP_PKEY_CTX_free(copy);
    return 1;
}

static int sign_test_init(EVP_TEST *t, const char *name)
{
    return pkey_test_init(t, name, 0, EVP_PKEY_sign_init, EVP_PKEY_sign);
}

static const EVP_TEST_METHOD psign_test_method = {
    "Sign",
    sign_test_init,
    pkey_test_cleanup,
    pkey_test_parse,
    pkey_test_run
};

static int verify_recover_test_init(EVP_TEST *t, const char *name)
{
    return pkey_test_init(t, name, 1, EVP_PKEY_verify_recover_init,
                          EVP_PKEY_verify_recover);
}

static const EVP_TEST_METHOD pverify_recover_test_method = {
    "VerifyRecover",
    verify_recover_test_init,
    pkey_test_cleanup,
    pkey_test_parse,
    pkey_test_run
};

static int decrypt_test_init(EVP_TEST *t, const char *name)
{
    return pkey_test_init(t, name, 0, EVP_PKEY_decrypt_init,
                          EVP_PKEY_decrypt);
}

static const EVP_TEST_METHOD pdecrypt_test_method = {
    "Decrypt",
    decrypt_test_init,
    pkey_test_cleanup,
    pkey_test_parse,
    pkey_test_run
};

static int verify_test_init(EVP_TEST *t, const char *name)
{
    return pkey_test_init(t, name, 1, EVP_PKEY_verify_init, 0);
}

static int verify_test_run(EVP_TEST *t)
{
    PKEY_DATA *kdata = t->data;

    if (EVP_PKEY_verify(kdata->ctx, kdata->output, kdata->output_len,
                        kdata->input, kdata->input_len) <= 0)
        t->err = "VERIFY_ERROR";
    return 1;
}

static const EVP_TEST_METHOD pverify_test_method = {
    "Verify",
    verify_test_init,
    pkey_test_cleanup,
    pkey_test_parse,
    verify_test_run
};

static int pderive_test_init(EVP_TEST *t, const char *name)
{
    return pkey_test_init(t, name, 0, EVP_PKEY_derive_init, 0);
}

static int pderive_test_parse(EVP_TEST *t,
                              const char *keyword, const char *value)
{
    PKEY_DATA *kdata = t->data;
    int validate = 0;

    if (strcmp(keyword, "PeerKeyValidate") == 0)
        validate = 1;

    if (validate || strcmp(keyword, "PeerKey") == 0) {
        EVP_PKEY *peer;
        if (find_key(&peer, value, public_keys) == 0)
            return -1;
        if (EVP_PKEY_derive_set_peer_ex(kdata->ctx, peer, validate) <= 0) {
            t->err = "DERIVE_SET_PEER_ERROR";
            return 1;
        }
        t->err = NULL;
        return 1;
    }
    if (strcmp(keyword, "SharedSecret") == 0)
        return parse_bin(value, &kdata->output, &kdata->output_len);
    if (strcmp(keyword, "Ctrl") == 0)
        return pkey_test_ctrl(t, kdata->ctx, value);
    if (strcmp(keyword, "KDFType") == 0) {
        OSSL_PARAM params[2];

        params[0] = OSSL_PARAM_construct_utf8_string(OSSL_EXCHANGE_PARAM_KDF_TYPE,
                                                     (char *)value, 0);
        params[1] = OSSL_PARAM_construct_end();
        if (EVP_PKEY_CTX_set_params(kdata->ctx, params) == 0)
            return -1;
        return 1;
    }
    if (strcmp(keyword, "KDFDigest") == 0) {
        OSSL_PARAM params[2];

        params[0] = OSSL_PARAM_construct_utf8_string(OSSL_EXCHANGE_PARAM_KDF_DIGEST,
                                                     (char *)value, 0);
        params[1] = OSSL_PARAM_construct_end();
        if (EVP_PKEY_CTX_set_params(kdata->ctx, params) == 0)
            return -1;
        return 1;
    }
    if (strcmp(keyword, "CEKAlg") == 0) {
        OSSL_PARAM params[2];

        params[0] = OSSL_PARAM_construct_utf8_string(OSSL_KDF_PARAM_CEK_ALG,
                                                     (char *)value, 0);
        params[1] = OSSL_PARAM_construct_end();
        if (EVP_PKEY_CTX_set_params(kdata->ctx, params) == 0)
            return -1;
        return 1;
    }
    if (strcmp(keyword, "KDFOutlen") == 0) {
        OSSL_PARAM params[2];
        char *endptr;
        size_t outlen = (size_t)strtoul(value, &endptr, 0);

        if (endptr[0] != '\0')
            return -1;

        params[0] = OSSL_PARAM_construct_size_t(OSSL_EXCHANGE_PARAM_KDF_OUTLEN,
                                                &outlen);
        params[1] = OSSL_PARAM_construct_end();
        if (EVP_PKEY_CTX_set_params(kdata->ctx, params) == 0)
            return -1;
        return 1;
    }
    return 0;
}

static int pderive_test_run(EVP_TEST *t)
{
    EVP_PKEY_CTX *dctx = NULL;
    PKEY_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len;

    if (!TEST_ptr(dctx = EVP_PKEY_CTX_dup(expected->ctx))) {
        t->err = "DERIVE_ERROR";
        goto err;
    }

    if (EVP_PKEY_derive(dctx, NULL, &got_len) <= 0
        || !TEST_size_t_ne(got_len, 0)) {
        t->err = "DERIVE_ERROR";
        goto err;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "DERIVE_ERROR";
        goto err;
    }
    if (EVP_PKEY_derive(dctx, got, &got_len) <= 0) {
        t->err = "DERIVE_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "SHARED_SECRET_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
 err:
    OPENSSL_free(got);
    EVP_PKEY_CTX_free(dctx);
    return 1;
}

static const EVP_TEST_METHOD pderive_test_method = {
    "Derive",
    pderive_test_init,
    pkey_test_cleanup,
    pderive_test_parse,
    pderive_test_run
};


/**
 **  PBE TESTS
 **/

typedef enum pbe_type_enum {
    PBE_TYPE_INVALID = 0,
    PBE_TYPE_SCRYPT, PBE_TYPE_PBKDF2, PBE_TYPE_PKCS12
} PBE_TYPE;

typedef struct pbe_data_st {
    PBE_TYPE pbe_type;
        /* scrypt parameters */
    uint64_t N, r, p, maxmem;
        /* PKCS#12 parameters */
    int id, iter;
    const EVP_MD *md;
        /* password */
    unsigned char *pass;
    size_t pass_len;
        /* salt */
    unsigned char *salt;
    size_t salt_len;
        /* Expected output */
    unsigned char *key;
    size_t key_len;
} PBE_DATA;

#ifndef OPENSSL_NO_SCRYPT
/* Parse unsigned decimal 64 bit integer value */
static int parse_uint64(const char *value, uint64_t *pr)
{
    const char *p = value;

    if (!TEST_true(*p)) {
        TEST_info("Invalid empty integer value");
        return -1;
    }
    for (*pr = 0; *p; ) {
        if (*pr > UINT64_MAX / 10) {
            TEST_error("Integer overflow in string %s", value);
            return -1;
        }
        *pr *= 10;
        if (!TEST_true(isdigit((unsigned char)*p))) {
            TEST_error("Invalid character in string %s", value);
            return -1;
        }
        *pr += *p - '0';
        p++;
    }
    return 1;
}

static int scrypt_test_parse(EVP_TEST *t,
                             const char *keyword, const char *value)
{
    PBE_DATA *pdata = t->data;

    if (strcmp(keyword, "N") == 0)
        return parse_uint64(value, &pdata->N);
    if (strcmp(keyword, "p") == 0)
        return parse_uint64(value, &pdata->p);
    if (strcmp(keyword, "r") == 0)
        return parse_uint64(value, &pdata->r);
    if (strcmp(keyword, "maxmem") == 0)
        return parse_uint64(value, &pdata->maxmem);
    return 0;
}
#endif

static int pbkdf2_test_parse(EVP_TEST *t,
                             const char *keyword, const char *value)
{
    PBE_DATA *pdata = t->data;

    if (strcmp(keyword, "iter") == 0) {
        pdata->iter = atoi(value);
        if (pdata->iter <= 0)
            return -1;
        return 1;
    }
    if (strcmp(keyword, "MD") == 0) {
        pdata->md = EVP_get_digestbyname(value);
        if (pdata->md == NULL)
            return -1;
        return 1;
    }
    return 0;
}

static int pkcs12_test_parse(EVP_TEST *t,
                             const char *keyword, const char *value)
{
    PBE_DATA *pdata = t->data;

    if (strcmp(keyword, "id") == 0) {
        pdata->id = atoi(value);
        if (pdata->id <= 0)
            return -1;
        return 1;
    }
    return pbkdf2_test_parse(t, keyword, value);
}

static int pbe_test_init(EVP_TEST *t, const char *alg)
{
    PBE_DATA *pdat;
    PBE_TYPE pbe_type = PBE_TYPE_INVALID;

    if (is_kdf_disabled(alg)) {
        TEST_info("skipping, '%s' is disabled", alg);
        t->skip = 1;
        return 1;
    }
    if (strcmp(alg, "scrypt") == 0) {
        pbe_type = PBE_TYPE_SCRYPT;
    } else if (strcmp(alg, "pbkdf2") == 0) {
        pbe_type = PBE_TYPE_PBKDF2;
    } else if (strcmp(alg, "pkcs12") == 0) {
        pbe_type = PBE_TYPE_PKCS12;
    } else {
        TEST_error("Unknown pbe algorithm %s", alg);
        return 0;
    }
    if (!TEST_ptr(pdat = OPENSSL_zalloc(sizeof(*pdat))))
        return 0;
    pdat->pbe_type = pbe_type;
    t->data = pdat;
    return 1;
}

static void pbe_test_cleanup(EVP_TEST *t)
{
    PBE_DATA *pdat = t->data;

    OPENSSL_free(pdat->pass);
    OPENSSL_free(pdat->salt);
    OPENSSL_free(pdat->key);
}

static int pbe_test_parse(EVP_TEST *t,
                          const char *keyword, const char *value)
{
    PBE_DATA *pdata = t->data;

    if (strcmp(keyword, "Password") == 0)
        return parse_bin(value, &pdata->pass, &pdata->pass_len);
    if (strcmp(keyword, "Salt") == 0)
        return parse_bin(value, &pdata->salt, &pdata->salt_len);
    if (strcmp(keyword, "Key") == 0)
        return parse_bin(value, &pdata->key, &pdata->key_len);
    if (pdata->pbe_type == PBE_TYPE_PBKDF2)
        return pbkdf2_test_parse(t, keyword, value);
    else if (pdata->pbe_type == PBE_TYPE_PKCS12)
        return pkcs12_test_parse(t, keyword, value);
#ifndef OPENSSL_NO_SCRYPT
    else if (pdata->pbe_type == PBE_TYPE_SCRYPT)
        return scrypt_test_parse(t, keyword, value);
#endif
    return 0;
}

static int pbe_test_run(EVP_TEST *t)
{
    PBE_DATA *expected = t->data;
    unsigned char *key;
    EVP_MD *fetched_digest = NULL;
    OSSL_LIB_CTX *save_libctx;

    save_libctx = OSSL_LIB_CTX_set0_default(libctx);

    if (!TEST_ptr(key = OPENSSL_malloc(expected->key_len))) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }
    if (expected->pbe_type == PBE_TYPE_PBKDF2) {
        if (PKCS5_PBKDF2_HMAC((char *)expected->pass, expected->pass_len,
                              expected->salt, expected->salt_len,
                              expected->iter, expected->md,
                              expected->key_len, key) == 0) {
            t->err = "PBKDF2_ERROR";
            goto err;
        }
#ifndef OPENSSL_NO_SCRYPT
    } else if (expected->pbe_type == PBE_TYPE_SCRYPT) {
        if (EVP_PBE_scrypt((const char *)expected->pass, expected->pass_len,
                            expected->salt, expected->salt_len,
                            expected->N, expected->r, expected->p,
                            expected->maxmem, key, expected->key_len) == 0) {
            t->err = "SCRYPT_ERROR";
            goto err;
        }
#endif
    } else if (expected->pbe_type == PBE_TYPE_PKCS12) {
        fetched_digest = EVP_MD_fetch(libctx, EVP_MD_get0_name(expected->md),
                                      NULL);
        if (fetched_digest == NULL) {
            t->err = "PKCS12_ERROR";
            goto err;
        }
        if (PKCS12_key_gen_uni(expected->pass, expected->pass_len,
                               expected->salt, expected->salt_len,
                               expected->id, expected->iter, expected->key_len,
                               key, fetched_digest) == 0) {
            t->err = "PKCS12_ERROR";
            goto err;
        }
    }
    if (!memory_err_compare(t, "KEY_MISMATCH", expected->key, expected->key_len,
                            key, expected->key_len))
        goto err;

    t->err = NULL;
err:
    EVP_MD_free(fetched_digest);
    OPENSSL_free(key);
    OSSL_LIB_CTX_set0_default(save_libctx);
    return 1;
}

static const EVP_TEST_METHOD pbe_test_method = {
    "PBE",
    pbe_test_init,
    pbe_test_cleanup,
    pbe_test_parse,
    pbe_test_run
};


/**
 **  BASE64 TESTS
 **/

typedef enum {
    BASE64_CANONICAL_ENCODING = 0,
    BASE64_VALID_ENCODING = 1,
    BASE64_INVALID_ENCODING = 2
} base64_encoding_type;

typedef struct encode_data_st {
    /* Input to encoding */
    unsigned char *input;
    size_t input_len;
    /* Expected output */
    unsigned char *output;
    size_t output_len;
    base64_encoding_type encoding;
} ENCODE_DATA;

static int encode_test_init(EVP_TEST *t, const char *encoding)
{
    ENCODE_DATA *edata;

    if (!TEST_ptr(edata = OPENSSL_zalloc(sizeof(*edata))))
        return 0;
    if (strcmp(encoding, "canonical") == 0) {
        edata->encoding = BASE64_CANONICAL_ENCODING;
    } else if (strcmp(encoding, "valid") == 0) {
        edata->encoding = BASE64_VALID_ENCODING;
    } else if (strcmp(encoding, "invalid") == 0) {
        edata->encoding = BASE64_INVALID_ENCODING;
        if (!TEST_ptr(t->expected_err = OPENSSL_strdup("DECODE_ERROR")))
            goto err;
    } else {
        TEST_error("Bad encoding: %s."
                   " Should be one of {canonical, valid, invalid}",
                   encoding);
        goto err;
    }
    t->data = edata;
    return 1;
err:
    OPENSSL_free(edata);
    return 0;
}

static void encode_test_cleanup(EVP_TEST *t)
{
    ENCODE_DATA *edata = t->data;

    OPENSSL_free(edata->input);
    OPENSSL_free(edata->output);
    memset(edata, 0, sizeof(*edata));
}

static int encode_test_parse(EVP_TEST *t,
                             const char *keyword, const char *value)
{
    ENCODE_DATA *edata = t->data;

    if (strcmp(keyword, "Input") == 0)
        return parse_bin(value, &edata->input, &edata->input_len);
    if (strcmp(keyword, "Output") == 0)
        return parse_bin(value, &edata->output, &edata->output_len);
    return 0;
}

static int encode_test_run(EVP_TEST *t)
{
    ENCODE_DATA *expected = t->data;
    unsigned char *encode_out = NULL, *decode_out = NULL;
    int output_len, chunk_len;
    EVP_ENCODE_CTX *decode_ctx = NULL, *encode_ctx = NULL;

    if (!TEST_ptr(decode_ctx = EVP_ENCODE_CTX_new())) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }

    if (expected->encoding == BASE64_CANONICAL_ENCODING) {

        if (!TEST_ptr(encode_ctx = EVP_ENCODE_CTX_new())
                || !TEST_ptr(encode_out =
                        OPENSSL_malloc(EVP_ENCODE_LENGTH(expected->input_len))))
            goto err;

        EVP_EncodeInit(encode_ctx);
        if (!TEST_true(EVP_EncodeUpdate(encode_ctx, encode_out, &chunk_len,
                                        expected->input, expected->input_len)))
            goto err;

        output_len = chunk_len;

        EVP_EncodeFinal(encode_ctx, encode_out + chunk_len, &chunk_len);
        output_len += chunk_len;

        if (!memory_err_compare(t, "BAD_ENCODING",
                                expected->output, expected->output_len,
                                encode_out, output_len))
            goto err;
    }

    if (!TEST_ptr(decode_out =
                OPENSSL_malloc(EVP_DECODE_LENGTH(expected->output_len))))
        goto err;

    EVP_DecodeInit(decode_ctx);
    if (EVP_DecodeUpdate(decode_ctx, decode_out, &chunk_len, expected->output,
                         expected->output_len) < 0) {
        t->err = "DECODE_ERROR";
        goto err;
    }
    output_len = chunk_len;

    if (EVP_DecodeFinal(decode_ctx, decode_out + chunk_len, &chunk_len) != 1) {
        t->err = "DECODE_ERROR";
        goto err;
    }
    output_len += chunk_len;

    if (expected->encoding != BASE64_INVALID_ENCODING
            && !memory_err_compare(t, "BAD_DECODING",
                                   expected->input, expected->input_len,
                                   decode_out, output_len)) {
        t->err = "BAD_DECODING";
        goto err;
    }

    t->err = NULL;
 err:
    OPENSSL_free(encode_out);
    OPENSSL_free(decode_out);
    EVP_ENCODE_CTX_free(decode_ctx);
    EVP_ENCODE_CTX_free(encode_ctx);
    return 1;
}

static const EVP_TEST_METHOD encode_test_method = {
    "Encoding",
    encode_test_init,
    encode_test_cleanup,
    encode_test_parse,
    encode_test_run,
};


/**
 **  RAND TESTS
 **/
#define MAX_RAND_REPEATS    15

typedef struct rand_data_pass_st {
    unsigned char *entropy;
    unsigned char *reseed_entropy;
    unsigned char *nonce;
    unsigned char *pers;
    unsigned char *reseed_addin;
    unsigned char *addinA;
    unsigned char *addinB;
    unsigned char *pr_entropyA;
    unsigned char *pr_entropyB;
    unsigned char *output;
    size_t entropy_len, nonce_len, pers_len, addinA_len, addinB_len,
           pr_entropyA_len, pr_entropyB_len, output_len, reseed_entropy_len,
           reseed_addin_len;
} RAND_DATA_PASS;

typedef struct rand_data_st {
    /* Context for this operation */
    EVP_RAND_CTX *ctx;
    EVP_RAND_CTX *parent;
    int n;
    int prediction_resistance;
    int use_df;
    unsigned int generate_bits;
    char *cipher;
    char *digest;

    /* Expected output */
    RAND_DATA_PASS data[MAX_RAND_REPEATS];
} RAND_DATA;

static int rand_test_init(EVP_TEST *t, const char *name)
{
    RAND_DATA *rdata;
    EVP_RAND *rand;
    OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
    unsigned int strength = 256;

    if (!TEST_ptr(rdata = OPENSSL_zalloc(sizeof(*rdata))))
        return 0;

    /* TEST-RAND is available in the FIPS provider but not with "fips=yes" */
    rand = EVP_RAND_fetch(libctx, "TEST-RAND", "-fips");
    if (rand == NULL)
        goto err;
    rdata->parent = EVP_RAND_CTX_new(rand, NULL);
    EVP_RAND_free(rand);
    if (rdata->parent == NULL)
        goto err;

    *params = OSSL_PARAM_construct_uint(OSSL_RAND_PARAM_STRENGTH, &strength);
    if (!EVP_RAND_CTX_set_params(rdata->parent, params))
        goto err;

    rand = EVP_RAND_fetch(libctx, name, NULL);
    if (rand == NULL)
        goto err;
    rdata->ctx = EVP_RAND_CTX_new(rand, rdata->parent);
    EVP_RAND_free(rand);
    if (rdata->ctx == NULL)
        goto err;

    rdata->n = -1;
    t->data = rdata;
    return 1;
 err:
    EVP_RAND_CTX_free(rdata->parent);
    OPENSSL_free(rdata);
    return 0;
}

static void rand_test_cleanup(EVP_TEST *t)
{
    RAND_DATA *rdata = t->data;
    int i;

    OPENSSL_free(rdata->cipher);
    OPENSSL_free(rdata->digest);

    for (i = 0; i <= rdata->n; i++) {
        OPENSSL_free(rdata->data[i].entropy);
        OPENSSL_free(rdata->data[i].reseed_entropy);
        OPENSSL_free(rdata->data[i].nonce);
        OPENSSL_free(rdata->data[i].pers);
        OPENSSL_free(rdata->data[i].reseed_addin);
        OPENSSL_free(rdata->data[i].addinA);
        OPENSSL_free(rdata->data[i].addinB);
        OPENSSL_free(rdata->data[i].pr_entropyA);
        OPENSSL_free(rdata->data[i].pr_entropyB);
        OPENSSL_free(rdata->data[i].output);
    }
    EVP_RAND_CTX_free(rdata->ctx);
    EVP_RAND_CTX_free(rdata->parent);
}

static int rand_test_parse(EVP_TEST *t,
                          const char *keyword, const char *value)
{
    RAND_DATA *rdata = t->data;
    RAND_DATA_PASS *item;
    const char *p;
    int n;

    if ((p = strchr(keyword, '.')) != NULL) {
        n = atoi(++p);
        if (n >= MAX_RAND_REPEATS)
            return 0;
        if (n > rdata->n)
            rdata->n = n;
        item = rdata->data + n;
        if (strncmp(keyword, "Entropy.", sizeof("Entropy")) == 0)
            return parse_bin(value, &item->entropy, &item->entropy_len);
        if (strncmp(keyword, "ReseedEntropy.", sizeof("ReseedEntropy")) == 0)
            return parse_bin(value, &item->reseed_entropy,
                             &item->reseed_entropy_len);
        if (strncmp(keyword, "Nonce.", sizeof("Nonce")) == 0)
            return parse_bin(value, &item->nonce, &item->nonce_len);
        if (strncmp(keyword, "PersonalisationString.",
                    sizeof("PersonalisationString")) == 0)
            return parse_bin(value, &item->pers, &item->pers_len);
        if (strncmp(keyword, "ReseedAdditionalInput.",
                    sizeof("ReseedAdditionalInput")) == 0)
            return parse_bin(value, &item->reseed_addin,
                             &item->reseed_addin_len);
        if (strncmp(keyword, "AdditionalInputA.",
                    sizeof("AdditionalInputA")) == 0)
            return parse_bin(value, &item->addinA, &item->addinA_len);
        if (strncmp(keyword, "AdditionalInputB.",
                    sizeof("AdditionalInputB")) == 0)
            return parse_bin(value, &item->addinB, &item->addinB_len);
        if (strncmp(keyword, "EntropyPredictionResistanceA.",
                    sizeof("EntropyPredictionResistanceA")) == 0)
            return parse_bin(value, &item->pr_entropyA, &item->pr_entropyA_len);
        if (strncmp(keyword, "EntropyPredictionResistanceB.",
                    sizeof("EntropyPredictionResistanceB")) == 0)
            return parse_bin(value, &item->pr_entropyB, &item->pr_entropyB_len);
        if (strncmp(keyword, "Output.", sizeof("Output")) == 0)
            return parse_bin(value, &item->output, &item->output_len);
    } else {
        if (strcmp(keyword, "Cipher") == 0)
            return TEST_ptr(rdata->cipher = OPENSSL_strdup(value));
        if (strcmp(keyword, "Digest") == 0)
            return TEST_ptr(rdata->digest = OPENSSL_strdup(value));
        if (strcmp(keyword, "DerivationFunction") == 0) {
            rdata->use_df = atoi(value) != 0;
            return 1;
        }
        if (strcmp(keyword, "GenerateBits") == 0) {
            if ((n = atoi(value)) <= 0 || n % 8 != 0)
                return 0;
            rdata->generate_bits = (unsigned int)n;
            return 1;
        }
        if (strcmp(keyword, "PredictionResistance") == 0) {
            rdata->prediction_resistance = atoi(value) != 0;
            return 1;
        }
    }
    return 0;
}

static int rand_test_run(EVP_TEST *t)
{
    RAND_DATA *expected = t->data;
    RAND_DATA_PASS *item;
    unsigned char *got;
    size_t got_len = expected->generate_bits / 8;
    OSSL_PARAM params[5], *p = params;
    int i = -1, ret = 0;
    unsigned int strength;
    unsigned char *z;

    if (!TEST_ptr(got = OPENSSL_malloc(got_len)))
        return 0;

    *p++ = OSSL_PARAM_construct_int(OSSL_DRBG_PARAM_USE_DF, &expected->use_df);
    if (expected->cipher != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_DRBG_PARAM_CIPHER,
                                                expected->cipher, 0);
    if (expected->digest != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_DRBG_PARAM_DIGEST,
                                                expected->digest, 0);
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_DRBG_PARAM_MAC, "HMAC", 0);
    *p = OSSL_PARAM_construct_end();
    if (!TEST_true(EVP_RAND_CTX_set_params(expected->ctx, params)))
        goto err;

    strength = EVP_RAND_get_strength(expected->ctx);
    for (i = 0; i <= expected->n; i++) {
        item = expected->data + i;

        p = params;
        z = item->entropy != NULL ? item->entropy : (unsigned char *)"";
        *p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY,
                                                 z, item->entropy_len);
        z = item->nonce != NULL ? item->nonce : (unsigned char *)"";
        *p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_NONCE,
                                                 z, item->nonce_len);
        *p = OSSL_PARAM_construct_end();
        if (!TEST_true(EVP_RAND_instantiate(expected->parent, strength,
                                            0, NULL, 0, params)))
            goto err;

        z = item->pers != NULL ? item->pers : (unsigned char *)"";
        if (!TEST_true(EVP_RAND_instantiate
                           (expected->ctx, strength,
                            expected->prediction_resistance, z,
                            item->pers_len, NULL)))
            goto err;

        if (item->reseed_entropy != NULL) {
            params[0] = OSSL_PARAM_construct_octet_string
                           (OSSL_RAND_PARAM_TEST_ENTROPY, item->reseed_entropy,
                            item->reseed_entropy_len);
            params[1] = OSSL_PARAM_construct_end();
            if (!TEST_true(EVP_RAND_CTX_set_params(expected->parent, params)))
                goto err;

            if (!TEST_true(EVP_RAND_reseed
                               (expected->ctx, expected->prediction_resistance,
                                NULL, 0, item->reseed_addin,
                                item->reseed_addin_len)))
                goto err;
        }
        if (item->pr_entropyA != NULL) {
            params[0] = OSSL_PARAM_construct_octet_string
                           (OSSL_RAND_PARAM_TEST_ENTROPY, item->pr_entropyA,
                            item->pr_entropyA_len);
            params[1] = OSSL_PARAM_construct_end();
            if (!TEST_true(EVP_RAND_CTX_set_params(expected->parent, params)))
                goto err;
        }
        if (!TEST_true(EVP_RAND_generate
                           (expected->ctx, got, got_len,
                            strength, expected->prediction_resistance,
                            item->addinA, item->addinA_len)))
            goto err;

        if (item->pr_entropyB != NULL) {
            params[0] = OSSL_PARAM_construct_octet_string
                           (OSSL_RAND_PARAM_TEST_ENTROPY, item->pr_entropyB,
                            item->pr_entropyB_len);
            params[1] = OSSL_PARAM_construct_end();
            if (!TEST_true(EVP_RAND_CTX_set_params(expected->parent, params)))
                goto err;
        }
        if (!TEST_true(EVP_RAND_generate
                           (expected->ctx, got, got_len,
                            strength, expected->prediction_resistance,
                            item->addinB, item->addinB_len)))
            goto err;
        if (!TEST_mem_eq(got, got_len, item->output, item->output_len))
            goto err;
        if (!TEST_true(EVP_RAND_uninstantiate(expected->ctx))
                || !TEST_true(EVP_RAND_uninstantiate(expected->parent))
                || !TEST_true(EVP_RAND_verify_zeroization(expected->ctx))
                || !TEST_int_eq(EVP_RAND_get_state(expected->ctx),
                                EVP_RAND_STATE_UNINITIALISED))
            goto err;
    }
    t->err = NULL;
    ret = 1;

 err:
    if (ret == 0 && i >= 0)
        TEST_info("Error in test case %d of %d\n", i, expected->n + 1);
    OPENSSL_free(got);
    return ret;
}

static const EVP_TEST_METHOD rand_test_method = {
    "RAND",
    rand_test_init,
    rand_test_cleanup,
    rand_test_parse,
    rand_test_run
};


/**
 **  KDF TESTS
 **/
typedef struct kdf_data_st {
    /* Context for this operation */
    EVP_KDF_CTX *ctx;
    /* Expected output */
    unsigned char *output;
    size_t output_len;
    OSSL_PARAM params[20];
    OSSL_PARAM *p;
} KDF_DATA;

/*
 * Perform public key operation setup: lookup key, allocated ctx and call
 * the appropriate initialisation function
 */
static int kdf_test_init(EVP_TEST *t, const char *name)
{
    KDF_DATA *kdata;
    EVP_KDF *kdf;

    if (is_kdf_disabled(name)) {
        TEST_info("skipping, '%s' is disabled", name);
        t->skip = 1;
        return 1;
    }

    if (!TEST_ptr(kdata = OPENSSL_zalloc(sizeof(*kdata))))
        return 0;
    kdata->p = kdata->params;
    *kdata->p = OSSL_PARAM_construct_end();

    kdf = EVP_KDF_fetch(libctx, name, NULL);
    if (kdf == NULL) {
        OPENSSL_free(kdata);
        return 0;
    }
    kdata->ctx = EVP_KDF_CTX_new(kdf);
    EVP_KDF_free(kdf);
    if (kdata->ctx == NULL) {
        OPENSSL_free(kdata);
        return 0;
    }
    t->data = kdata;
    return 1;
}

static void kdf_test_cleanup(EVP_TEST *t)
{
    KDF_DATA *kdata = t->data;
    OSSL_PARAM *p;

    for (p = kdata->params; p->key != NULL; p++)
        OPENSSL_free(p->data);
    OPENSSL_free(kdata->output);
    EVP_KDF_CTX_free(kdata->ctx);
}

static int kdf_test_ctrl(EVP_TEST *t, EVP_KDF_CTX *kctx,
                         const char *value)
{
    KDF_DATA *kdata = t->data;
    int rv;
    char *p, *name;
    const OSSL_PARAM *defs = EVP_KDF_settable_ctx_params(EVP_KDF_CTX_kdf(kctx));

    if (!TEST_ptr(name = OPENSSL_strdup(value)))
        return 0;
    p = strchr(name, ':');
    if (p != NULL)
        *p++ = '\0';

    rv = OSSL_PARAM_allocate_from_text(kdata->p, defs, name, p,
                                       p != NULL ? strlen(p) : 0, NULL);
    *++kdata->p = OSSL_PARAM_construct_end();
    if (!rv) {
        t->err = "KDF_PARAM_ERROR";
        OPENSSL_free(name);
        return 0;
    }
    if (p != NULL && strcmp(name, "digest") == 0) {
        if (is_digest_disabled(p)) {
            TEST_info("skipping, '%s' is disabled", p);
            t->skip = 1;
        }
    }
    if (p != NULL
        && (strcmp(name, "cipher") == 0
            || strcmp(name, "cekalg") == 0)
        && is_cipher_disabled(p)) {
        TEST_info("skipping, '%s' is disabled", p);
        t->skip = 1;
    }
    OPENSSL_free(name);
    return 1;
}

static int kdf_test_parse(EVP_TEST *t,
                          const char *keyword, const char *value)
{
    KDF_DATA *kdata = t->data;

    if (strcmp(keyword, "Output") == 0)
        return parse_bin(value, &kdata->output, &kdata->output_len);
    if (strncmp(keyword, "Ctrl", 4) == 0)
        return kdf_test_ctrl(t, kdata->ctx, value);
    return 0;
}

static int kdf_test_run(EVP_TEST *t)
{
    KDF_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len = expected->output_len;

    if (!EVP_KDF_CTX_set_params(expected->ctx, expected->params)) {
        t->err = "KDF_CTRL_ERROR";
        return 1;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len == 0 ? 1 : got_len))) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }
    if (EVP_KDF_derive(expected->ctx, got, got_len, NULL) <= 0) {
        t->err = "KDF_DERIVE_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "KDF_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;

 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD kdf_test_method = {
    "KDF",
    kdf_test_init,
    kdf_test_cleanup,
    kdf_test_parse,
    kdf_test_run
};

/**
 **  PKEY KDF TESTS
 **/

typedef struct pkey_kdf_data_st {
    /* Context for this operation */
    EVP_PKEY_CTX *ctx;
    /* Expected output */
    unsigned char *output;
    size_t output_len;
} PKEY_KDF_DATA;

/*
 * Perform public key operation setup: lookup key, allocated ctx and call
 * the appropriate initialisation function
 */
static int pkey_kdf_test_init(EVP_TEST *t, const char *name)
{
    PKEY_KDF_DATA *kdata = NULL;

    if (is_kdf_disabled(name)) {
        TEST_info("skipping, '%s' is disabled", name);
        t->skip = 1;
        return 1;
    }

    if (!TEST_ptr(kdata = OPENSSL_zalloc(sizeof(*kdata))))
        return 0;

    kdata->ctx = EVP_PKEY_CTX_new_from_name(libctx, name, NULL);
    if (kdata->ctx == NULL
        || EVP_PKEY_derive_init(kdata->ctx) <= 0)
        goto err;

    t->data = kdata;
    return 1;
err:
    EVP_PKEY_CTX_free(kdata->ctx);
    OPENSSL_free(kdata);
    return 0;
}

static void pkey_kdf_test_cleanup(EVP_TEST *t)
{
    PKEY_KDF_DATA *kdata = t->data;

    OPENSSL_free(kdata->output);
    EVP_PKEY_CTX_free(kdata->ctx);
}

static int pkey_kdf_test_parse(EVP_TEST *t,
                               const char *keyword, const char *value)
{
    PKEY_KDF_DATA *kdata = t->data;

    if (strcmp(keyword, "Output") == 0)
        return parse_bin(value, &kdata->output, &kdata->output_len);
    if (strncmp(keyword, "Ctrl", 4) == 0)
        return pkey_test_ctrl(t, kdata->ctx, value);
    return 0;
}

static int pkey_kdf_test_run(EVP_TEST *t)
{
    PKEY_KDF_DATA *expected = t->data;
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
    }
    if (EVP_PKEY_derive(expected->ctx, got, &got_len) <= 0) {
        t->err = "KDF_DERIVE_ERROR";
        goto err;
    }
    if (!TEST_mem_eq(expected->output, expected->output_len, got, got_len)) {
        t->err = "KDF_MISMATCH";
        goto err;
    }
    t->err = NULL;

 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD pkey_kdf_test_method = {
    "PKEYKDF",
    pkey_kdf_test_init,
    pkey_kdf_test_cleanup,
    pkey_kdf_test_parse,
    pkey_kdf_test_run
};

/**
 **  KEYPAIR TESTS
 **/

typedef struct keypair_test_data_st {
    EVP_PKEY *privk;
    EVP_PKEY *pubk;
} KEYPAIR_TEST_DATA;

static int keypair_test_init(EVP_TEST *t, const char *pair)
{
    KEYPAIR_TEST_DATA *data;
    int rv = 0;
    EVP_PKEY *pk = NULL, *pubk = NULL;
    char *pub, *priv = NULL;

    /* Split private and public names. */
    if (!TEST_ptr(priv = OPENSSL_strdup(pair))
            || !TEST_ptr(pub = strchr(priv, ':'))) {
        t->err = "PARSING_ERROR";
        goto end;
    }
    *pub++ = '\0';

    if (!TEST_true(find_key(&pk, priv, private_keys))) {
        TEST_info("Can't find private key: %s", priv);
        t->err = "MISSING_PRIVATE_KEY";
        goto end;
    }
    if (!TEST_true(find_key(&pubk, pub, public_keys))) {
        TEST_info("Can't find public key: %s", pub);
        t->err = "MISSING_PUBLIC_KEY";
        goto end;
    }

    if (pk == NULL && pubk == NULL) {
        /* Both keys are listed but unsupported: skip this test */
        t->skip = 1;
        rv = 1;
        goto end;
    }

    if (!TEST_ptr(data = OPENSSL_malloc(sizeof(*data))))
        goto end;
    data->privk = pk;
    data->pubk = pubk;
    t->data = data;
    rv = 1;
    t->err = NULL;

end:
    OPENSSL_free(priv);
    return rv;
}

static void keypair_test_cleanup(EVP_TEST *t)
{
    OPENSSL_free(t->data);
    t->data = NULL;
}

/*
 * For tests that do not accept any custom keywords.
 */
static int void_test_parse(EVP_TEST *t, const char *keyword, const char *value)
{
    return 0;
}

static int keypair_test_run(EVP_TEST *t)
{
    int rv = 0;
    const KEYPAIR_TEST_DATA *pair = t->data;

    if (pair->privk == NULL || pair->pubk == NULL) {
        /*
         * this can only happen if only one of the keys is not set
         * which means that one of them was unsupported while the
         * other isn't: hence a key type mismatch.
         */
        t->err = "KEYPAIR_TYPE_MISMATCH";
        rv = 1;
        goto end;
    }

    if ((rv = EVP_PKEY_eq(pair->privk, pair->pubk)) != 1 ) {
        if ( 0 == rv ) {
            t->err = "KEYPAIR_MISMATCH";
        } else if ( -1 == rv ) {
            t->err = "KEYPAIR_TYPE_MISMATCH";
        } else if ( -2 == rv ) {
            t->err = "UNSUPPORTED_KEY_COMPARISON";
        } else {
            TEST_error("Unexpected error in key comparison");
            rv = 0;
            goto end;
        }
        rv = 1;
        goto end;
    }

    rv = 1;
    t->err = NULL;

end:
    return rv;
}

static const EVP_TEST_METHOD keypair_test_method = {
    "PrivPubKeyPair",
    keypair_test_init,
    keypair_test_cleanup,
    void_test_parse,
    keypair_test_run
};

/**
 **  KEYGEN TEST
 **/

typedef struct keygen_test_data_st {
    EVP_PKEY_CTX *genctx; /* Keygen context to use */
    char *keyname; /* Key name to store key or NULL */
} KEYGEN_TEST_DATA;

static int keygen_test_init(EVP_TEST *t, const char *alg)
{
    KEYGEN_TEST_DATA *data;
    EVP_PKEY_CTX *genctx;
    int nid = OBJ_sn2nid(alg);

    if (nid == NID_undef) {
        nid = OBJ_ln2nid(alg);
        if (nid == NID_undef)
            return 0;
    }

    if (is_pkey_disabled(alg)) {
        t->skip = 1;
        return 1;
    }
    if (!TEST_ptr(genctx = EVP_PKEY_CTX_new_from_name(libctx, alg, NULL)))
        goto err;

    if (EVP_PKEY_keygen_init(genctx) <= 0) {
        t->err = "KEYGEN_INIT_ERROR";
        goto err;
    }

    if (!TEST_ptr(data = OPENSSL_malloc(sizeof(*data))))
        goto err;
    data->genctx = genctx;
    data->keyname = NULL;
    t->data = data;
    t->err = NULL;
    return 1;

err:
    EVP_PKEY_CTX_free(genctx);
    return 0;
}

static void keygen_test_cleanup(EVP_TEST *t)
{
    KEYGEN_TEST_DATA *keygen = t->data;

    EVP_PKEY_CTX_free(keygen->genctx);
    OPENSSL_free(keygen->keyname);
    OPENSSL_free(t->data);
    t->data = NULL;
}

static int keygen_test_parse(EVP_TEST *t,
                             const char *keyword, const char *value)
{
    KEYGEN_TEST_DATA *keygen = t->data;

    if (strcmp(keyword, "KeyName") == 0)
        return TEST_ptr(keygen->keyname = OPENSSL_strdup(value));
    if (strcmp(keyword, "Ctrl") == 0)
        return pkey_test_ctrl(t, keygen->genctx, value);
    return 0;
}

static int keygen_test_run(EVP_TEST *t)
{
    KEYGEN_TEST_DATA *keygen = t->data;
    EVP_PKEY *pkey = NULL;
    int rv = 1;

    if (EVP_PKEY_keygen(keygen->genctx, &pkey) <= 0) {
        t->err = "KEYGEN_GENERATE_ERROR";
        goto err;
    }

    if (!evp_pkey_is_provided(pkey)) {
        TEST_info("Warning: legacy key generated %s", keygen->keyname);
        goto err;
    }
    if (keygen->keyname != NULL) {
        KEY_LIST *key;

        rv = 0;
        if (find_key(NULL, keygen->keyname, private_keys)) {
            TEST_info("Duplicate key %s", keygen->keyname);
            goto err;
        }

        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            goto err;
        key->name = keygen->keyname;
        keygen->keyname = NULL;
        key->key = pkey;
        key->next = private_keys;
        private_keys = key;
        rv = 1;
    } else {
        EVP_PKEY_free(pkey);
    }

    t->err = NULL;

err:
    return rv;
}

static const EVP_TEST_METHOD keygen_test_method = {
    "KeyGen",
    keygen_test_init,
    keygen_test_cleanup,
    keygen_test_parse,
    keygen_test_run,
};

/**
 **  DIGEST SIGN+VERIFY TESTS
 **/

typedef struct {
    int is_verify; /* Set to 1 if verifying */
    int is_oneshot; /* Set to 1 for one shot operation */
    const EVP_MD *md; /* Digest to use */
    EVP_MD_CTX *ctx; /* Digest context */
    EVP_PKEY_CTX *pctx;
    STACK_OF(EVP_TEST_BUFFER) *input; /* Input data: streaming */
    unsigned char *osin; /* Input data if one shot */
    size_t osin_len; /* Input length data if one shot */
    unsigned char *output; /* Expected output */
    size_t output_len; /* Expected output length */
} DIGESTSIGN_DATA;

static int digestsigver_test_init(EVP_TEST *t, const char *alg, int is_verify,
                                  int is_oneshot)
{
    const EVP_MD *md = NULL;
    DIGESTSIGN_DATA *mdat;

    if (strcmp(alg, "NULL") != 0) {
        if (is_digest_disabled(alg)) {
            t->skip = 1;
            return 1;
        }
        md = EVP_get_digestbyname(alg);
        if (md == NULL)
            return 0;
    }
    if (!TEST_ptr(mdat = OPENSSL_zalloc(sizeof(*mdat))))
        return 0;
    mdat->md = md;
    if (!TEST_ptr(mdat->ctx = EVP_MD_CTX_new())) {
        OPENSSL_free(mdat);
        return 0;
    }
    mdat->is_verify = is_verify;
    mdat->is_oneshot = is_oneshot;
    t->data = mdat;
    return 1;
}

static int digestsign_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 0, 0);
}

static void digestsigver_test_cleanup(EVP_TEST *t)
{
    DIGESTSIGN_DATA *mdata = t->data;

    EVP_MD_CTX_free(mdata->ctx);
    sk_EVP_TEST_BUFFER_pop_free(mdata->input, evp_test_buffer_free);
    OPENSSL_free(mdata->osin);
    OPENSSL_free(mdata->output);
    OPENSSL_free(mdata);
    t->data = NULL;
}

static int digestsigver_test_parse(EVP_TEST *t,
                                   const char *keyword, const char *value)
{
    DIGESTSIGN_DATA *mdata = t->data;

    if (strcmp(keyword, "Key") == 0) {
        EVP_PKEY *pkey = NULL;
        int rv = 0;
        const char *name = mdata->md == NULL ? NULL : EVP_MD_get0_name(mdata->md);

        if (mdata->is_verify)
            rv = find_key(&pkey, value, public_keys);
        if (rv == 0)
            rv = find_key(&pkey, value, private_keys);
        if (rv == 0 || pkey == NULL) {
            t->skip = 1;
            return 1;
        }
        if (mdata->is_verify) {
            if (!EVP_DigestVerifyInit_ex(mdata->ctx, &mdata->pctx, name, libctx,
                                         NULL, pkey, NULL))
                t->err = "DIGESTVERIFYINIT_ERROR";
            return 1;
        }
        if (!EVP_DigestSignInit_ex(mdata->ctx, &mdata->pctx, name, libctx, NULL,
                                   pkey, NULL))
            t->err = "DIGESTSIGNINIT_ERROR";
        return 1;
    }

    if (strcmp(keyword, "Input") == 0) {
        if (mdata->is_oneshot)
            return parse_bin(value, &mdata->osin, &mdata->osin_len);
        return evp_test_buffer_append(value, &mdata->input);
    }
    if (strcmp(keyword, "Output") == 0)
        return parse_bin(value, &mdata->output, &mdata->output_len);

    if (!mdata->is_oneshot) {
        if (strcmp(keyword, "Count") == 0)
            return evp_test_buffer_set_count(value, mdata->input);
        if (strcmp(keyword, "Ncopy") == 0)
            return evp_test_buffer_ncopy(value, mdata->input);
    }
    if (strcmp(keyword, "Ctrl") == 0) {
        if (mdata->pctx == NULL)
            return -1;
        return pkey_test_ctrl(t, mdata->pctx, value);
    }
    return 0;
}

static int digestsign_update_fn(void *ctx, const unsigned char *buf,
                                size_t buflen)
{
    return EVP_DigestSignUpdate(ctx, buf, buflen);
}

static int digestsign_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len;

    if (!evp_test_buffer_do(expected->input, digestsign_update_fn,
                            expected->ctx)) {
        t->err = "DIGESTUPDATE_ERROR";
        goto err;
    }

    if (!EVP_DigestSignFinal(expected->ctx, NULL, &got_len)) {
        t->err = "DIGESTSIGNFINAL_LENGTH_ERROR";
        goto err;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSignFinal(expected->ctx, got, &got_len)) {
        t->err = "DIGESTSIGNFINAL_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "SIGNATURE_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD digestsign_test_method = {
    "DigestSign",
    digestsign_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    digestsign_test_run
};

static int digestverify_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 1, 0);
}

static int digestverify_update_fn(void *ctx, const unsigned char *buf,
                                  size_t buflen)
{
    return EVP_DigestVerifyUpdate(ctx, buf, buflen);
}

static int digestverify_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *mdata = t->data;

    if (!evp_test_buffer_do(mdata->input, digestverify_update_fn, mdata->ctx)) {
        t->err = "DIGESTUPDATE_ERROR";
        return 1;
    }

    if (EVP_DigestVerifyFinal(mdata->ctx, mdata->output,
                              mdata->output_len) <= 0)
        t->err = "VERIFY_ERROR";
    return 1;
}

static const EVP_TEST_METHOD digestverify_test_method = {
    "DigestVerify",
    digestverify_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    digestverify_test_run
};

static int oneshot_digestsign_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 0, 1);
}

static int oneshot_digestsign_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len;

    if (!EVP_DigestSign(expected->ctx, NULL, &got_len,
                        expected->osin, expected->osin_len)) {
        t->err = "DIGESTSIGN_LENGTH_ERROR";
        goto err;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSign(expected->ctx, got, &got_len,
                        expected->osin, expected->osin_len)) {
        t->err = "DIGESTSIGN_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "SIGNATURE_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD oneshot_digestsign_test_method = {
    "OneShotDigestSign",
    oneshot_digestsign_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    oneshot_digestsign_test_run
};

static int oneshot_digestverify_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 1, 1);
}

static int oneshot_digestverify_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *mdata = t->data;

    if (EVP_DigestVerify(mdata->ctx, mdata->output, mdata->output_len,
                         mdata->osin, mdata->osin_len) <= 0)
        t->err = "VERIFY_ERROR";
    return 1;
}

static const EVP_TEST_METHOD oneshot_digestverify_test_method = {
    "OneShotDigestVerify",
    oneshot_digestverify_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    oneshot_digestverify_test_run
};


/**
 **  PARSING AND DISPATCH
 **/

static const EVP_TEST_METHOD *evp_test_list[] = {
    &rand_test_method,
    &cipher_test_method,
    &digest_test_method,
    &digestsign_test_method,
    &digestverify_test_method,
    &encode_test_method,
    &kdf_test_method,
    &pkey_kdf_test_method,
    &keypair_test_method,
    &keygen_test_method,
    &mac_test_method,
    &oneshot_digestsign_test_method,
    &oneshot_digestverify_test_method,
    &pbe_test_method,
    &pdecrypt_test_method,
    &pderive_test_method,
    &psign_test_method,
    &pverify_recover_test_method,
    &pverify_test_method,
    NULL
};

static const EVP_TEST_METHOD *find_test(const char *name)
{
    const EVP_TEST_METHOD **tt;

    for (tt = evp_test_list; *tt; tt++) {
        if (strcmp(name, (*tt)->name) == 0)
            return *tt;
    }
    return NULL;
}

static void clear_test(EVP_TEST *t)
{
    test_clearstanza(&t->s);
    ERR_clear_error();
    if (t->data != NULL) {
        if (t->meth != NULL)
            t->meth->cleanup(t);
        OPENSSL_free(t->data);
        t->data = NULL;
    }
    OPENSSL_free(t->expected_err);
    t->expected_err = NULL;
    OPENSSL_free(t->reason);
    t->reason = NULL;

    /* Text literal. */
    t->err = NULL;
    t->skip = 0;
    t->meth = NULL;
}

/* Check for errors in the test structure; return 1 if okay, else 0. */
static int check_test_error(EVP_TEST *t)
{
    unsigned long err;
    const char *reason;

    if (t->err == NULL && t->expected_err == NULL)
        return 1;
    if (t->err != NULL && t->expected_err == NULL) {
        if (t->aux_err != NULL) {
            TEST_info("%s:%d: Source of above error (%s); unexpected error %s",
                      t->s.test_file, t->s.start, t->aux_err, t->err);
        } else {
            TEST_info("%s:%d: Source of above error; unexpected error %s",
                      t->s.test_file, t->s.start, t->err);
        }
        return 0;
    }
    if (t->err == NULL && t->expected_err != NULL) {
        TEST_info("%s:%d: Succeeded but was expecting %s",
                  t->s.test_file, t->s.start, t->expected_err);
        return 0;
    }

    if (strcmp(t->err, t->expected_err) != 0) {
        TEST_info("%s:%d: Expected %s got %s",
                  t->s.test_file, t->s.start, t->expected_err, t->err);
        return 0;
    }

    if (t->reason == NULL)
        return 1;

    if (t->reason == NULL) {
        TEST_info("%s:%d: Test is missing function or reason code",
                  t->s.test_file, t->s.start);
        return 0;
    }

    err = ERR_peek_error();
    if (err == 0) {
        TEST_info("%s:%d: Expected error \"%s\" not set",
                  t->s.test_file, t->s.start, t->reason);
        return 0;
    }

    reason = ERR_reason_error_string(err);
    if (reason == NULL) {
        TEST_info("%s:%d: Expected error \"%s\", no strings available."
                  " Assuming ok.",
                  t->s.test_file, t->s.start, t->reason);
        return 1;
    }

    if (strcmp(reason, t->reason) == 0)
        return 1;

    TEST_info("%s:%d: Expected error \"%s\", got \"%s\"",
              t->s.test_file, t->s.start, t->reason, reason);

    return 0;
}

/* Run a parsed test. Log a message and return 0 on error. */
static int run_test(EVP_TEST *t)
{
    if (t->meth == NULL)
        return 1;
    t->s.numtests++;
    if (t->skip) {
        t->s.numskip++;
    } else {
        /* run the test */
        if (t->err == NULL && t->meth->run_test(t) != 1) {
            TEST_info("%s:%d %s error",
                      t->s.test_file, t->s.start, t->meth->name);
            return 0;
        }
        if (!check_test_error(t)) {
            TEST_openssl_errors();
            t->s.errors++;
        }
    }

    /* clean it up */
    return 1;
}

static int find_key(EVP_PKEY **ppk, const char *name, KEY_LIST *lst)
{
    for (; lst != NULL; lst = lst->next) {
        if (strcmp(lst->name, name) == 0) {
            if (ppk != NULL)
                *ppk = lst->key;
            return 1;
        }
    }
    return 0;
}

static void free_key_list(KEY_LIST *lst)
{
    while (lst != NULL) {
        KEY_LIST *next = lst->next;

        EVP_PKEY_free(lst->key);
        OPENSSL_free(lst->name);
        OPENSSL_free(lst);
        lst = next;
    }
}

/*
 * Is the key type an unsupported algorithm?
 */
static int key_unsupported(void)
{
    long err = ERR_peek_last_error();
    int lib = ERR_GET_LIB(err);
    long reason = ERR_GET_REASON(err);

    if ((lib == ERR_LIB_EVP && reason == EVP_R_UNSUPPORTED_ALGORITHM)
        || (lib == ERR_LIB_EVP && reason == EVP_R_DECODE_ERROR)
        || reason == ERR_R_UNSUPPORTED) {
        ERR_clear_error();
        return 1;
    }
#ifndef OPENSSL_NO_EC
    /*
     * If EC support is enabled we should catch also EC_R_UNKNOWN_GROUP as an
     * hint to an unsupported algorithm/curve (e.g. if binary EC support is
     * disabled).
     */
    if (lib == ERR_LIB_EC
        && (reason == EC_R_UNKNOWN_GROUP
            || reason == EC_R_INVALID_CURVE)) {
        ERR_clear_error();
        return 1;
    }
#endif /* OPENSSL_NO_EC */
    return 0;
}

/* NULL out the value from |pp| but return it.  This "steals" a pointer. */
static char *take_value(PAIR *pp)
{
    char *p = pp->value;

    pp->value = NULL;
    return p;
}

#if !defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
static int securitycheck_enabled(void)
{
    static int enabled = -1;

    if (enabled == -1) {
        if (OSSL_PROVIDER_available(libctx, "fips")) {
            OSSL_PARAM params[2];
            OSSL_PROVIDER *prov = NULL;
            int check = 1;

            prov = OSSL_PROVIDER_load(libctx, "fips");
            if (prov != NULL) {
                params[0] =
                    OSSL_PARAM_construct_int(OSSL_PROV_PARAM_SECURITY_CHECKS,
                                             &check);
                params[1] = OSSL_PARAM_construct_end();
                OSSL_PROVIDER_get_params(prov, params);
                OSSL_PROVIDER_unload(prov);
            }
            enabled = check;
            return enabled;
        }
        enabled = 0;
    }
    return enabled;
}
#endif

/*
 * Return 1 if one of the providers named in the string is available.
 * The provider names are separated with whitespace.
 * NOTE: destructive function, it inserts '\0' after each provider name.
 */
static int prov_available(char *providers)
{
    char *p;
    int more = 1;

    while (more) {
        for (; isspace(*providers); providers++)
            continue;
        if (*providers == '\0')
            break;               /* End of the road */
        for (p = providers; *p != '\0' && !isspace(*p); p++)
            continue;
        if (*p == '\0')
            more = 0;
        else
            *p = '\0';
        if (OSSL_PROVIDER_available(libctx, providers))
            return 1;            /* Found one */
    }
    return 0;
}

/* Read and parse one test.  Return 0 if failure, 1 if okay. */
static int parse(EVP_TEST *t)
{
    KEY_LIST *key, **klist;
    EVP_PKEY *pkey;
    PAIR *pp;
    int i, skip_availablein = 0;

top:
    do {
        if (BIO_eof(t->s.fp))
            return EOF;
        clear_test(t);
        if (!test_readstanza(&t->s))
            return 0;
    } while (t->s.numpairs == 0);
    pp = &t->s.pairs[0];

    /* Are we adding a key? */
    klist = NULL;
    pkey = NULL;
start:
    if (strcmp(pp->key, "PrivateKey") == 0) {
        pkey = PEM_read_bio_PrivateKey_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read private key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &private_keys;
    } else if (strcmp(pp->key, "PublicKey") == 0) {
        pkey = PEM_read_bio_PUBKEY_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read public key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &public_keys;
    } else if (strcmp(pp->key, "PrivateKeyRaw") == 0
               || strcmp(pp->key, "PublicKeyRaw") == 0 ) {
        char *strnid = NULL, *keydata = NULL;
        unsigned char *keybin;
        size_t keylen;
        int nid;

        if (strcmp(pp->key, "PrivateKeyRaw") == 0)
            klist = &private_keys;
        else
            klist = &public_keys;

        strnid = strchr(pp->value, ':');
        if (strnid != NULL) {
            *strnid++ = '\0';
            keydata = strchr(strnid, ':');
            if (keydata != NULL)
                *keydata++ = '\0';
        }
        if (keydata == NULL) {
            TEST_info("Failed to parse %s value", pp->key);
            return 0;
        }

        nid = OBJ_txt2nid(strnid);
        if (nid == NID_undef) {
            TEST_info("Unrecognised algorithm NID");
            return 0;
        }
        if (!parse_bin(keydata, &keybin, &keylen)) {
            TEST_info("Failed to create binary key");
            return 0;
        }
        if (klist == &private_keys)
            pkey = EVP_PKEY_new_raw_private_key_ex(libctx, strnid, NULL, keybin,
                                                   keylen);
        else
            pkey = EVP_PKEY_new_raw_public_key_ex(libctx, strnid, NULL, keybin,
                                                  keylen);
        if (pkey == NULL && !key_unsupported()) {
            TEST_info("Can't read %s data", pp->key);
            OPENSSL_free(keybin);
            TEST_openssl_errors();
            return 0;
        }
        OPENSSL_free(keybin);
    } else if (strcmp(pp->key, "Availablein") == 0) {
        if (!prov_available(pp->value)) {
            TEST_info("skipping, '%s' provider not available: %s:%d",
                      pp->value, t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
        }
        skip_availablein++;
        pp++;
        goto start;
    }

    /* If we have a key add to list */
    if (klist != NULL) {
        if (find_key(NULL, pp->value, *klist)) {
            TEST_info("Duplicate key %s", pp->value);
            return 0;
        }
        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            return 0;
        key->name = take_value(pp);
        key->key = pkey;
        key->next = *klist;
        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

    /* Find the test, based on first keyword. */
    if (!TEST_ptr(t->meth = find_test(pp->key)))
        return 0;
    if (!t->meth->init(t, pp->value)) {
        TEST_error("unknown %s: %s\n", pp->key, pp->value);
        return 0;
    }
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        } else if (strcmp(pp->key, "Availablein") == 0) {
            TEST_info("Line %d: 'Availablein' should be the first option",
                      t->s.curr);
            return 0;
        } else if (strcmp(pp->key, "Result") == 0) {
            if (t->expected_err != NULL) {
                TEST_info("Line %d: multiple result lines", t->s.curr);
                return 0;
            }
            t->expected_err = take_value(pp);
        } else if (strcmp(pp->key, "Function") == 0) {
            /* Ignore old line. */
        } else if (strcmp(pp->key, "Reason") == 0) {
            if (t->reason != NULL) {
                TEST_info("Line %d: multiple reason lines", t->s.curr);
                return 0;
            }
            t->reason = take_value(pp);
        } else {
            /* Must be test specific line: try to parse it */
            int rv = t->meth->parse(t, pp->key, pp->value);

            if (rv == 0) {
                TEST_info("Line %d: unknown keyword %s", t->s.curr, pp->key);
                return 0;
            }
            if (rv < 0) {
                TEST_info("Line %d: error processing keyword %s = %s\n",
                          t->s.curr, pp->key, pp->value);
                return 0;
            }
        }
    }

    return 1;
}

static int run_file_tests(int i)
{
    EVP_TEST *t;
    const char *testfile = test_get_argument(i);
    int c;

    if (!TEST_ptr(t = OPENSSL_zalloc(sizeof(*t))))
        return 0;
    if (!test_start_file(&t->s, testfile)) {
        OPENSSL_free(t);
        return 0;
    }

    while (!BIO_eof(t->s.fp)) {
        c = parse(t);
        if (t->skip) {
            t->s.numskip++;
            continue;
        }
        if (c == 0 || !run_test(t)) {
            t->s.errors++;
            break;
        }
    }
    test_end_file(&t->s);
    clear_test(t);

    free_key_list(public_keys);
    free_key_list(private_keys);
    BIO_free(t->s.key);
    c = t->s.errors;
    OPENSSL_free(t);
    return c == 0;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
        { "config", OPT_CONFIG_FILE, '<',
          "The configuration file to use for the libctx" },
        { OPT_HELP_STR, 1, '-', "file\tFile to run tests on.\n" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    size_t n;
    char *config_file = NULL;

    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONFIG_FILE:
            config_file = opt_arg();
            break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    /*
     * Load the provider via configuration into the created library context.
     * Load the 'null' provider into the default library context to ensure that
     * the tests do not fallback to using the default provider.
     */
    if (!test_get_libctx(&libctx, &prov_null, config_file, NULL, NULL))
        return 0;

    n = test_get_argument_count();
    if (n == 0)
        return 0;

    ADD_ALL_TESTS(run_file_tests, n);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(prov_null);
    OSSL_LIB_CTX_free(libctx);
}

#define STR_STARTS_WITH(str, pre) OPENSSL_strncasecmp(pre, str, strlen(pre)) == 0
#define STR_ENDS_WITH(str, pre)                                                \
strlen(str) < strlen(pre) ? 0 : (OPENSSL_strcasecmp(pre, str + strlen(str) - strlen(pre)) == 0)

static int is_digest_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE"))
        return 1;
#endif
#ifdef OPENSSL_NO_MD2
    if (OPENSSL_strcasecmp(name, "MD2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MDC2
    if (OPENSSL_strcasecmp(name, "MDC2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD4
    if (OPENSSL_strcasecmp(name, "MD4") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD5
    if (OPENSSL_strcasecmp(name, "MD5") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_RMD160
    if (OPENSSL_strcasecmp(name, "RIPEMD160") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_SM3
    if (OPENSSL_strcasecmp(name, "SM3") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
    if (OPENSSL_strcasecmp(name, "WHIRLPOOL") == 0)
        return 1;
#endif
    return 0;
}

static int is_pkey_disabled(const char *name)
{
#ifdef OPENSSL_NO_EC
    if (STR_STARTS_WITH(name, "EC"))
        return 1;
#endif
#ifdef OPENSSL_NO_DH
    if (STR_STARTS_WITH(name, "DH"))
        return 1;
#endif
#ifdef OPENSSL_NO_DSA
    if (STR_STARTS_WITH(name, "DSA"))
        return 1;
#endif
    return 0;
}

static int is_mac_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE2BMAC")
        || STR_STARTS_WITH(name, "BLAKE2SMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_CMAC
    if (STR_STARTS_WITH(name, "CMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_STARTS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIPHASH
    if (STR_STARTS_WITH(name, "SipHash"))
        return 1;
#endif
    return 0;
}
static int is_kdf_disabled(const char *name)
{
#ifdef OPENSSL_NO_SCRYPT
    if (STR_ENDS_WITH(name, "SCRYPT"))
        return 1;
#endif
    return 0;
}

static int is_cipher_disabled(const char *name)
{
#ifdef OPENSSL_NO_ARIA
    if (STR_STARTS_WITH(name, "ARIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_BF
    if (STR_STARTS_WITH(name, "BF"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAMELLIA
    if (STR_STARTS_WITH(name, "CAMELLIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAST
    if (STR_STARTS_WITH(name, "CAST"))
        return 1;
#endif
#ifdef OPENSSL_NO_CHACHA
    if (STR_STARTS_WITH(name, "CHACHA"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_ENDS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_DES
    if (STR_STARTS_WITH(name, "DES"))
        return 1;
    if (STR_ENDS_WITH(name, "3DESwrap"))
        return 1;
#endif
#ifdef OPENSSL_NO_OCB
    if (STR_ENDS_WITH(name, "OCB"))
        return 1;
#endif
#ifdef OPENSSL_NO_IDEA
    if (STR_STARTS_WITH(name, "IDEA"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC2
    if (STR_STARTS_WITH(name, "RC2"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC4
    if (STR_STARTS_WITH(name, "RC4"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC5
    if (STR_STARTS_WITH(name, "RC5"))
        return 1;
#endif
#ifdef OPENSSL_NO_SEED
    if (STR_STARTS_WITH(name, "SEED"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIV
    if (STR_ENDS_WITH(name, "SIV"))
        return 1;
#endif
#ifdef OPENSSL_NO_SM4
    if (STR_STARTS_WITH(name, "SM4"))
        return 1;
#endif
    return 0;
}
                                   got, got_len)) {
            t->err = "TEST_MAC_ERR";
            goto err;
        }
    }
    if (reinit--) {
        OSSL_PARAM ivparams[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
        int ret;

        /* If the MAC uses IV, we have to set it again */
        if (expected->iv != NULL) {
            ivparams[0] =
                OSSL_PARAM_construct_octet_string(OSSL_MAC_PARAM_IV,
                                                  expected->iv,
                                                  expected->iv_len);
            ivparams[1] = OSSL_PARAM_construct_end();
        }
        ERR_set_mark();
        ret = EVP_MAC_init(ctx, NULL, 0, ivparams);
        if (expected->no_reinit) {
            if (ret) {
                ERR_clear_last_mark();
                t->err = "MAC_REINIT_SHOULD_FAIL";
                goto err;
            }
{
    PKEY_KDF_DATA *expected = t->data;
    unsigned char *got = NULL;
    size_t got_len = 0;

    /* Find out the KDF output size */
    if (EVP_PKEY_derive(expected->ctx, NULL, &got_len) <= 0) {
        t->err = "INTERNAL_ERROR";
        goto err;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSignFinal(expected->ctx, got, &got_len)) {
        t->err = "DIGESTSIGNFINAL_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "SIGNATURE_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD digestsign_test_method = {
    "DigestSign",
    digestsign_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    digestsign_test_run
};

static int digestverify_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 1, 0);
}

static int digestverify_update_fn(void *ctx, const unsigned char *buf,
                                  size_t buflen)
{
    return EVP_DigestVerifyUpdate(ctx, buf, buflen);
}

static int digestverify_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *mdata = t->data;

    if (!evp_test_buffer_do(mdata->input, digestverify_update_fn, mdata->ctx)) {
        t->err = "DIGESTUPDATE_ERROR";
        return 1;
    }
    if (!TEST_ptr(got = OPENSSL_malloc(got_len))) {
        t->err = "MALLOC_FAILURE";
        goto err;
    }
    if (!EVP_DigestSign(expected->ctx, got, &got_len,
                        expected->osin, expected->osin_len)) {
        t->err = "DIGESTSIGN_ERROR";
        goto err;
    }
    if (!memory_err_compare(t, "SIGNATURE_MISMATCH",
                            expected->output, expected->output_len,
                            got, got_len))
        goto err;

    t->err = NULL;
 err:
    OPENSSL_free(got);
    return 1;
}

static const EVP_TEST_METHOD oneshot_digestsign_test_method = {
    "OneShotDigestSign",
    oneshot_digestsign_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    oneshot_digestsign_test_run
};

static int oneshot_digestverify_test_init(EVP_TEST *t, const char *alg)
{
    return digestsigver_test_init(t, alg, 1, 1);
}

static int oneshot_digestverify_test_run(EVP_TEST *t)
{
    DIGESTSIGN_DATA *mdata = t->data;

    if (EVP_DigestVerify(mdata->ctx, mdata->output, mdata->output_len,
                         mdata->osin, mdata->osin_len) <= 0)
        t->err = "VERIFY_ERROR";
    return 1;
}

static const EVP_TEST_METHOD oneshot_digestverify_test_method = {
    "OneShotDigestVerify",
    oneshot_digestverify_test_init,
    digestsigver_test_cleanup,
    digestsigver_test_parse,
    oneshot_digestverify_test_run
};


/**
 **  PARSING AND DISPATCH
 **/

static const EVP_TEST_METHOD *evp_test_list[] = {
    &rand_test_method,
    &cipher_test_method,
    &digest_test_method,
    &digestsign_test_method,
    &digestverify_test_method,
    &encode_test_method,
    &kdf_test_method,
    &pkey_kdf_test_method,
    &keypair_test_method,
    &keygen_test_method,
    &mac_test_method,
    &oneshot_digestsign_test_method,
    &oneshot_digestverify_test_method,
    &pbe_test_method,
    &pdecrypt_test_method,
    &pderive_test_method,
    &psign_test_method,
    &pverify_recover_test_method,
    &pverify_test_method,
    NULL
};

static const EVP_TEST_METHOD *find_test(const char *name)
{
    const EVP_TEST_METHOD **tt;

    for (tt = evp_test_list; *tt; tt++) {
        if (strcmp(name, (*tt)->name) == 0)
            return *tt;
    }
    return NULL;
}

static void clear_test(EVP_TEST *t)
{
    test_clearstanza(&t->s);
    ERR_clear_error();
    if (t->data != NULL) {
        if (t->meth != NULL)
            t->meth->cleanup(t);
        OPENSSL_free(t->data);
        t->data = NULL;
    }
    OPENSSL_free(t->expected_err);
    t->expected_err = NULL;
    OPENSSL_free(t->reason);
    t->reason = NULL;

    /* Text literal. */
    t->err = NULL;
    t->skip = 0;
    t->meth = NULL;
}

/* Check for errors in the test structure; return 1 if okay, else 0. */
static int check_test_error(EVP_TEST *t)
{
    unsigned long err;
    const char *reason;

    if (t->err == NULL && t->expected_err == NULL)
        return 1;
    if (t->err != NULL && t->expected_err == NULL) {
        if (t->aux_err != NULL) {
            TEST_info("%s:%d: Source of above error (%s); unexpected error %s",
                      t->s.test_file, t->s.start, t->aux_err, t->err);
        } else {
            TEST_info("%s:%d: Source of above error; unexpected error %s",
                      t->s.test_file, t->s.start, t->err);
        }
        return 0;
    }
    if (t->err == NULL && t->expected_err != NULL) {
        TEST_info("%s:%d: Succeeded but was expecting %s",
                  t->s.test_file, t->s.start, t->expected_err);
        return 0;
    }

    if (strcmp(t->err, t->expected_err) != 0) {
        TEST_info("%s:%d: Expected %s got %s",
                  t->s.test_file, t->s.start, t->expected_err, t->err);
        return 0;
    }

    if (t->reason == NULL)
        return 1;

    if (t->reason == NULL) {
        TEST_info("%s:%d: Test is missing function or reason code",
                  t->s.test_file, t->s.start);
        return 0;
    }

    err = ERR_peek_error();
    if (err == 0) {
        TEST_info("%s:%d: Expected error \"%s\" not set",
                  t->s.test_file, t->s.start, t->reason);
        return 0;
    }

    reason = ERR_reason_error_string(err);
    if (reason == NULL) {
        TEST_info("%s:%d: Expected error \"%s\", no strings available."
                  " Assuming ok.",
                  t->s.test_file, t->s.start, t->reason);
        return 1;
    }

    if (strcmp(reason, t->reason) == 0)
        return 1;

    TEST_info("%s:%d: Expected error \"%s\", got \"%s\"",
              t->s.test_file, t->s.start, t->reason, reason);

    return 0;
}

/* Run a parsed test. Log a message and return 0 on error. */
static int run_test(EVP_TEST *t)
{
    if (t->meth == NULL)
        return 1;
    t->s.numtests++;
    if (t->skip) {
        t->s.numskip++;
    } else {
        /* run the test */
        if (t->err == NULL && t->meth->run_test(t) != 1) {
            TEST_info("%s:%d %s error",
                      t->s.test_file, t->s.start, t->meth->name);
            return 0;
        }
        if (!check_test_error(t)) {
            TEST_openssl_errors();
            t->s.errors++;
        }
    }

    /* clean it up */
    return 1;
}

static int find_key(EVP_PKEY **ppk, const char *name, KEY_LIST *lst)
{
    for (; lst != NULL; lst = lst->next) {
        if (strcmp(lst->name, name) == 0) {
            if (ppk != NULL)
                *ppk = lst->key;
            return 1;
        }
    }
    return 0;
}

static void free_key_list(KEY_LIST *lst)
{
    while (lst != NULL) {
        KEY_LIST *next = lst->next;

        EVP_PKEY_free(lst->key);
        OPENSSL_free(lst->name);
        OPENSSL_free(lst);
        lst = next;
    }
}

/*
 * Is the key type an unsupported algorithm?
 */
static int key_unsupported(void)
{
    long err = ERR_peek_last_error();
    int lib = ERR_GET_LIB(err);
    long reason = ERR_GET_REASON(err);

    if ((lib == ERR_LIB_EVP && reason == EVP_R_UNSUPPORTED_ALGORITHM)
        || (lib == ERR_LIB_EVP && reason == EVP_R_DECODE_ERROR)
        || reason == ERR_R_UNSUPPORTED) {
        ERR_clear_error();
        return 1;
    }
#ifndef OPENSSL_NO_EC
    /*
     * If EC support is enabled we should catch also EC_R_UNKNOWN_GROUP as an
     * hint to an unsupported algorithm/curve (e.g. if binary EC support is
     * disabled).
     */
    if (lib == ERR_LIB_EC
        && (reason == EC_R_UNKNOWN_GROUP
            || reason == EC_R_INVALID_CURVE)) {
        ERR_clear_error();
        return 1;
    }
#endif /* OPENSSL_NO_EC */
    return 0;
}

/* NULL out the value from |pp| but return it.  This "steals" a pointer. */
static char *take_value(PAIR *pp)
{
    char *p = pp->value;

    pp->value = NULL;
    return p;
}

#if !defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
static int securitycheck_enabled(void)
{
    static int enabled = -1;

    if (enabled == -1) {
        if (OSSL_PROVIDER_available(libctx, "fips")) {
            OSSL_PARAM params[2];
            OSSL_PROVIDER *prov = NULL;
            int check = 1;

            prov = OSSL_PROVIDER_load(libctx, "fips");
            if (prov != NULL) {
                params[0] =
                    OSSL_PARAM_construct_int(OSSL_PROV_PARAM_SECURITY_CHECKS,
                                             &check);
                params[1] = OSSL_PARAM_construct_end();
                OSSL_PROVIDER_get_params(prov, params);
                OSSL_PROVIDER_unload(prov);
            }
            enabled = check;
            return enabled;
        }
        enabled = 0;
    }
    return enabled;
}
#endif

/*
 * Return 1 if one of the providers named in the string is available.
 * The provider names are separated with whitespace.
 * NOTE: destructive function, it inserts '\0' after each provider name.
 */
static int prov_available(char *providers)
{
    char *p;
    int more = 1;

    while (more) {
        for (; isspace(*providers); providers++)
            continue;
        if (*providers == '\0')
            break;               /* End of the road */
        for (p = providers; *p != '\0' && !isspace(*p); p++)
            continue;
        if (*p == '\0')
            more = 0;
        else
            *p = '\0';
        if (OSSL_PROVIDER_available(libctx, providers))
            return 1;            /* Found one */
    }
    return 0;
}

/* Read and parse one test.  Return 0 if failure, 1 if okay. */
static int parse(EVP_TEST *t)
{
    KEY_LIST *key, **klist;
    EVP_PKEY *pkey;
    PAIR *pp;
    int i, skip_availablein = 0;

top:
    do {
        if (BIO_eof(t->s.fp))
            return EOF;
        clear_test(t);
        if (!test_readstanza(&t->s))
            return 0;
    } while (t->s.numpairs == 0);
    pp = &t->s.pairs[0];

    /* Are we adding a key? */
    klist = NULL;
    pkey = NULL;
start:
    if (strcmp(pp->key, "PrivateKey") == 0) {
        pkey = PEM_read_bio_PrivateKey_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read private key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &private_keys;
    } else if (strcmp(pp->key, "PublicKey") == 0) {
        pkey = PEM_read_bio_PUBKEY_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read public key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &public_keys;
    } else if (strcmp(pp->key, "PrivateKeyRaw") == 0
               || strcmp(pp->key, "PublicKeyRaw") == 0 ) {
        char *strnid = NULL, *keydata = NULL;
        unsigned char *keybin;
        size_t keylen;
        int nid;

        if (strcmp(pp->key, "PrivateKeyRaw") == 0)
            klist = &private_keys;
        else
            klist = &public_keys;

        strnid = strchr(pp->value, ':');
        if (strnid != NULL) {
            *strnid++ = '\0';
            keydata = strchr(strnid, ':');
            if (keydata != NULL)
                *keydata++ = '\0';
        }
        if (keydata == NULL) {
            TEST_info("Failed to parse %s value", pp->key);
            return 0;
        }

        nid = OBJ_txt2nid(strnid);
        if (nid == NID_undef) {
            TEST_info("Unrecognised algorithm NID");
            return 0;
        }
        if (!parse_bin(keydata, &keybin, &keylen)) {
            TEST_info("Failed to create binary key");
            return 0;
        }
        if (klist == &private_keys)
            pkey = EVP_PKEY_new_raw_private_key_ex(libctx, strnid, NULL, keybin,
                                                   keylen);
        else
            pkey = EVP_PKEY_new_raw_public_key_ex(libctx, strnid, NULL, keybin,
                                                  keylen);
        if (pkey == NULL && !key_unsupported()) {
            TEST_info("Can't read %s data", pp->key);
            OPENSSL_free(keybin);
            TEST_openssl_errors();
            return 0;
        }
        OPENSSL_free(keybin);
    } else if (strcmp(pp->key, "Availablein") == 0) {
        if (!prov_available(pp->value)) {
            TEST_info("skipping, '%s' provider not available: %s:%d",
                      pp->value, t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
        }
        skip_availablein++;
        pp++;
        goto start;
    }

    /* If we have a key add to list */
    if (klist != NULL) {
        if (find_key(NULL, pp->value, *klist)) {
            TEST_info("Duplicate key %s", pp->value);
            return 0;
        }
        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            return 0;
        key->name = take_value(pp);
        key->key = pkey;
        key->next = *klist;
        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

    /* Find the test, based on first keyword. */
    if (!TEST_ptr(t->meth = find_test(pp->key)))
        return 0;
    if (!t->meth->init(t, pp->value)) {
        TEST_error("unknown %s: %s\n", pp->key, pp->value);
        return 0;
    }
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        } else if (strcmp(pp->key, "Availablein") == 0) {
            TEST_info("Line %d: 'Availablein' should be the first option",
                      t->s.curr);
            return 0;
        } else if (strcmp(pp->key, "Result") == 0) {
            if (t->expected_err != NULL) {
                TEST_info("Line %d: multiple result lines", t->s.curr);
                return 0;
            }
            t->expected_err = take_value(pp);
        } else if (strcmp(pp->key, "Function") == 0) {
            /* Ignore old line. */
        } else if (strcmp(pp->key, "Reason") == 0) {
            if (t->reason != NULL) {
                TEST_info("Line %d: multiple reason lines", t->s.curr);
                return 0;
            }
            t->reason = take_value(pp);
        } else {
            /* Must be test specific line: try to parse it */
            int rv = t->meth->parse(t, pp->key, pp->value);

            if (rv == 0) {
                TEST_info("Line %d: unknown keyword %s", t->s.curr, pp->key);
                return 0;
            }
            if (rv < 0) {
                TEST_info("Line %d: error processing keyword %s = %s\n",
                          t->s.curr, pp->key, pp->value);
                return 0;
            }
        }
    }

    return 1;
}

static int run_file_tests(int i)
{
    EVP_TEST *t;
    const char *testfile = test_get_argument(i);
    int c;

    if (!TEST_ptr(t = OPENSSL_zalloc(sizeof(*t))))
        return 0;
    if (!test_start_file(&t->s, testfile)) {
        OPENSSL_free(t);
        return 0;
    }

    while (!BIO_eof(t->s.fp)) {
        c = parse(t);
        if (t->skip) {
            t->s.numskip++;
            continue;
        }
        if (c == 0 || !run_test(t)) {
            t->s.errors++;
            break;
        }
    }
    test_end_file(&t->s);
    clear_test(t);

    free_key_list(public_keys);
    free_key_list(private_keys);
    BIO_free(t->s.key);
    c = t->s.errors;
    OPENSSL_free(t);
    return c == 0;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
        { "config", OPT_CONFIG_FILE, '<',
          "The configuration file to use for the libctx" },
        { OPT_HELP_STR, 1, '-', "file\tFile to run tests on.\n" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    size_t n;
    char *config_file = NULL;

    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONFIG_FILE:
            config_file = opt_arg();
            break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    /*
     * Load the provider via configuration into the created library context.
     * Load the 'null' provider into the default library context to ensure that
     * the tests do not fallback to using the default provider.
     */
    if (!test_get_libctx(&libctx, &prov_null, config_file, NULL, NULL))
        return 0;

    n = test_get_argument_count();
    if (n == 0)
        return 0;

    ADD_ALL_TESTS(run_file_tests, n);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(prov_null);
    OSSL_LIB_CTX_free(libctx);
}

#define STR_STARTS_WITH(str, pre) OPENSSL_strncasecmp(pre, str, strlen(pre)) == 0
#define STR_ENDS_WITH(str, pre)                                                \
strlen(str) < strlen(pre) ? 0 : (OPENSSL_strcasecmp(pre, str + strlen(str) - strlen(pre)) == 0)

static int is_digest_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE"))
        return 1;
#endif
#ifdef OPENSSL_NO_MD2
    if (OPENSSL_strcasecmp(name, "MD2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MDC2
    if (OPENSSL_strcasecmp(name, "MDC2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD4
    if (OPENSSL_strcasecmp(name, "MD4") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD5
    if (OPENSSL_strcasecmp(name, "MD5") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_RMD160
    if (OPENSSL_strcasecmp(name, "RIPEMD160") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_SM3
    if (OPENSSL_strcasecmp(name, "SM3") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
    if (OPENSSL_strcasecmp(name, "WHIRLPOOL") == 0)
        return 1;
#endif
    return 0;
}

static int is_pkey_disabled(const char *name)
{
#ifdef OPENSSL_NO_EC
    if (STR_STARTS_WITH(name, "EC"))
        return 1;
#endif
#ifdef OPENSSL_NO_DH
    if (STR_STARTS_WITH(name, "DH"))
        return 1;
#endif
#ifdef OPENSSL_NO_DSA
    if (STR_STARTS_WITH(name, "DSA"))
        return 1;
#endif
    return 0;
}

static int is_mac_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE2BMAC")
        || STR_STARTS_WITH(name, "BLAKE2SMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_CMAC
    if (STR_STARTS_WITH(name, "CMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_STARTS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIPHASH
    if (STR_STARTS_WITH(name, "SipHash"))
        return 1;
#endif
    return 0;
}
static int is_kdf_disabled(const char *name)
{
#ifdef OPENSSL_NO_SCRYPT
    if (STR_ENDS_WITH(name, "SCRYPT"))
        return 1;
#endif
    return 0;
}

static int is_cipher_disabled(const char *name)
{
#ifdef OPENSSL_NO_ARIA
    if (STR_STARTS_WITH(name, "ARIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_BF
    if (STR_STARTS_WITH(name, "BF"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAMELLIA
    if (STR_STARTS_WITH(name, "CAMELLIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAST
    if (STR_STARTS_WITH(name, "CAST"))
        return 1;
#endif
#ifdef OPENSSL_NO_CHACHA
    if (STR_STARTS_WITH(name, "CHACHA"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_ENDS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_DES
    if (STR_STARTS_WITH(name, "DES"))
        return 1;
    if (STR_ENDS_WITH(name, "3DESwrap"))
        return 1;
#endif
#ifdef OPENSSL_NO_OCB
    if (STR_ENDS_WITH(name, "OCB"))
        return 1;
#endif
#ifdef OPENSSL_NO_IDEA
    if (STR_STARTS_WITH(name, "IDEA"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC2
    if (STR_STARTS_WITH(name, "RC2"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC4
    if (STR_STARTS_WITH(name, "RC4"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC5
    if (STR_STARTS_WITH(name, "RC5"))
        return 1;
#endif
#ifdef OPENSSL_NO_SEED
    if (STR_STARTS_WITH(name, "SEED"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIV
    if (STR_ENDS_WITH(name, "SIV"))
        return 1;
#endif
#ifdef OPENSSL_NO_SM4
    if (STR_STARTS_WITH(name, "SM4"))
        return 1;
#endif
    return 0;
}
{
    KEY_LIST *key, **klist;
    EVP_PKEY *pkey;
    PAIR *pp;
    int i, skip_availablein = 0;

top:
    do {
        if (BIO_eof(t->s.fp))
            return EOF;
        clear_test(t);
        if (!test_readstanza(&t->s))
            return 0;
    } while (t->s.numpairs == 0);
    pp = &t->s.pairs[0];

    /* Are we adding a key? */
    klist = NULL;
    pkey = NULL;
start:
    if (strcmp(pp->key, "PrivateKey") == 0) {
        pkey = PEM_read_bio_PrivateKey_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read private key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &private_keys;
    } else if (strcmp(pp->key, "PublicKey") == 0) {
        pkey = PEM_read_bio_PUBKEY_ex(t->s.key, NULL, 0, NULL, libctx, NULL);
        if (pkey == NULL && !key_unsupported()) {
            EVP_PKEY_free(pkey);
            TEST_info("Can't read public key %s", pp->value);
            TEST_openssl_errors();
            return 0;
        }
        klist = &public_keys;
    } else if (strcmp(pp->key, "PrivateKeyRaw") == 0
               || strcmp(pp->key, "PublicKeyRaw") == 0 ) {
        char *strnid = NULL, *keydata = NULL;
        unsigned char *keybin;
        size_t keylen;
        int nid;

        if (strcmp(pp->key, "PrivateKeyRaw") == 0)
            klist = &private_keys;
        else
            klist = &public_keys;

        strnid = strchr(pp->value, ':');
        if (strnid != NULL) {
            *strnid++ = '\0';
            keydata = strchr(strnid, ':');
            if (keydata != NULL)
                *keydata++ = '\0';
        }
        if (keydata == NULL) {
            TEST_info("Failed to parse %s value", pp->key);
            return 0;
        }

        nid = OBJ_txt2nid(strnid);
        if (nid == NID_undef) {
            TEST_info("Unrecognised algorithm NID");
            return 0;
        }
        if (!parse_bin(keydata, &keybin, &keylen)) {
            TEST_info("Failed to create binary key");
            return 0;
        }
        if (klist == &private_keys)
            pkey = EVP_PKEY_new_raw_private_key_ex(libctx, strnid, NULL, keybin,
                                                   keylen);
        else
            pkey = EVP_PKEY_new_raw_public_key_ex(libctx, strnid, NULL, keybin,
                                                  keylen);
        if (pkey == NULL && !key_unsupported()) {
            TEST_info("Can't read %s data", pp->key);
            OPENSSL_free(keybin);
            TEST_openssl_errors();
            return 0;
        }
        OPENSSL_free(keybin);
    } else if (strcmp(pp->key, "Availablein") == 0) {
        if (!prov_available(pp->value)) {
            TEST_info("skipping, '%s' provider not available: %s:%d",
                      pp->value, t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
        }
        skip_availablein++;
        pp++;
        goto start;
    }

    /* If we have a key add to list */
    if (klist != NULL) {
        if (find_key(NULL, pp->value, *klist)) {
            TEST_info("Duplicate key %s", pp->value);
            return 0;
        }
        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            return 0;
        key->name = take_value(pp);
        key->key = pkey;
        key->next = *klist;
        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

    /* Find the test, based on first keyword. */
    if (!TEST_ptr(t->meth = find_test(pp->key)))
        return 0;
    if (!t->meth->init(t, pp->value)) {
        TEST_error("unknown %s: %s\n", pp->key, pp->value);
        return 0;
    }
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        } else if (strcmp(pp->key, "Availablein") == 0) {
            TEST_info("Line %d: 'Availablein' should be the first option",
                      t->s.curr);
            return 0;
        } else if (strcmp(pp->key, "Result") == 0) {
            if (t->expected_err != NULL) {
                TEST_info("Line %d: multiple result lines", t->s.curr);
                return 0;
            }
            t->expected_err = take_value(pp);
        } else if (strcmp(pp->key, "Function") == 0) {
            /* Ignore old line. */
        } else if (strcmp(pp->key, "Reason") == 0) {
            if (t->reason != NULL) {
                TEST_info("Line %d: multiple reason lines", t->s.curr);
                return 0;
            }
            t->reason = take_value(pp);
        } else {
            /* Must be test specific line: try to parse it */
            int rv = t->meth->parse(t, pp->key, pp->value);

            if (rv == 0) {
                TEST_info("Line %d: unknown keyword %s", t->s.curr, pp->key);
                return 0;
            }
            if (rv < 0) {
                TEST_info("Line %d: error processing keyword %s = %s\n",
                          t->s.curr, pp->key, pp->value);
                return 0;
            }
        }
    }

    return 1;
}

static int run_file_tests(int i)
{
    EVP_TEST *t;
    const char *testfile = test_get_argument(i);
    int c;

    if (!TEST_ptr(t = OPENSSL_zalloc(sizeof(*t))))
        return 0;
    if (!test_start_file(&t->s, testfile)) {
        OPENSSL_free(t);
        return 0;
    }

    while (!BIO_eof(t->s.fp)) {
        c = parse(t);
        if (t->skip) {
            t->s.numskip++;
            continue;
        }
        if (c == 0 || !run_test(t)) {
            t->s.errors++;
            break;
        }
    }
    test_end_file(&t->s);
    clear_test(t);

    free_key_list(public_keys);
    free_key_list(private_keys);
    BIO_free(t->s.key);
    c = t->s.errors;
    OPENSSL_free(t);
    return c == 0;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
        { "config", OPT_CONFIG_FILE, '<',
          "The configuration file to use for the libctx" },
        { OPT_HELP_STR, 1, '-', "file\tFile to run tests on.\n" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    size_t n;
    char *config_file = NULL;

    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONFIG_FILE:
            config_file = opt_arg();
            break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    /*
     * Load the provider via configuration into the created library context.
     * Load the 'null' provider into the default library context to ensure that
     * the tests do not fallback to using the default provider.
     */
    if (!test_get_libctx(&libctx, &prov_null, config_file, NULL, NULL))
        return 0;

    n = test_get_argument_count();
    if (n == 0)
        return 0;

    ADD_ALL_TESTS(run_file_tests, n);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(prov_null);
    OSSL_LIB_CTX_free(libctx);
}

#define STR_STARTS_WITH(str, pre) OPENSSL_strncasecmp(pre, str, strlen(pre)) == 0
#define STR_ENDS_WITH(str, pre)                                                \
strlen(str) < strlen(pre) ? 0 : (OPENSSL_strcasecmp(pre, str + strlen(str) - strlen(pre)) == 0)

static int is_digest_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE"))
        return 1;
#endif
#ifdef OPENSSL_NO_MD2
    if (OPENSSL_strcasecmp(name, "MD2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MDC2
    if (OPENSSL_strcasecmp(name, "MDC2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD4
    if (OPENSSL_strcasecmp(name, "MD4") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD5
    if (OPENSSL_strcasecmp(name, "MD5") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_RMD160
    if (OPENSSL_strcasecmp(name, "RIPEMD160") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_SM3
    if (OPENSSL_strcasecmp(name, "SM3") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
    if (OPENSSL_strcasecmp(name, "WHIRLPOOL") == 0)
        return 1;
#endif
    return 0;
}

static int is_pkey_disabled(const char *name)
{
#ifdef OPENSSL_NO_EC
    if (STR_STARTS_WITH(name, "EC"))
        return 1;
#endif
#ifdef OPENSSL_NO_DH
    if (STR_STARTS_WITH(name, "DH"))
        return 1;
#endif
#ifdef OPENSSL_NO_DSA
    if (STR_STARTS_WITH(name, "DSA"))
        return 1;
#endif
    return 0;
}

static int is_mac_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE2BMAC")
        || STR_STARTS_WITH(name, "BLAKE2SMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_CMAC
    if (STR_STARTS_WITH(name, "CMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_STARTS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIPHASH
    if (STR_STARTS_WITH(name, "SipHash"))
        return 1;
#endif
    return 0;
}
static int is_kdf_disabled(const char *name)
{
#ifdef OPENSSL_NO_SCRYPT
    if (STR_ENDS_WITH(name, "SCRYPT"))
        return 1;
#endif
    return 0;
}

static int is_cipher_disabled(const char *name)
{
#ifdef OPENSSL_NO_ARIA
    if (STR_STARTS_WITH(name, "ARIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_BF
    if (STR_STARTS_WITH(name, "BF"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAMELLIA
    if (STR_STARTS_WITH(name, "CAMELLIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAST
    if (STR_STARTS_WITH(name, "CAST"))
        return 1;
#endif
#ifdef OPENSSL_NO_CHACHA
    if (STR_STARTS_WITH(name, "CHACHA"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_ENDS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_DES
    if (STR_STARTS_WITH(name, "DES"))
        return 1;
    if (STR_ENDS_WITH(name, "3DESwrap"))
        return 1;
#endif
#ifdef OPENSSL_NO_OCB
    if (STR_ENDS_WITH(name, "OCB"))
        return 1;
#endif
#ifdef OPENSSL_NO_IDEA
    if (STR_STARTS_WITH(name, "IDEA"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC2
    if (STR_STARTS_WITH(name, "RC2"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC4
    if (STR_STARTS_WITH(name, "RC4"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC5
    if (STR_STARTS_WITH(name, "RC5"))
        return 1;
#endif
#ifdef OPENSSL_NO_SEED
    if (STR_STARTS_WITH(name, "SEED"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIV
    if (STR_ENDS_WITH(name, "SIV"))
        return 1;
#endif
#ifdef OPENSSL_NO_SM4
    if (STR_STARTS_WITH(name, "SM4"))
        return 1;
#endif
    return 0;
}
        if (!prov_available(pp->value)) {
            TEST_info("skipping, '%s' provider not available: %s:%d",
                      pp->value, t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
        }
        skip_availablein++;
        pp++;
        goto start;
    }

    /* If we have a key add to list */
    if (klist != NULL) {
        if (find_key(NULL, pp->value, *klist)) {
            TEST_info("Duplicate key %s", pp->value);
            return 0;
        }
        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            return 0;
        key->name = take_value(pp);
        key->key = pkey;
        key->next = *klist;
        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

    /* Find the test, based on first keyword. */
    if (!TEST_ptr(t->meth = find_test(pp->key)))
        return 0;
    if (!t->meth->init(t, pp->value)) {
        TEST_error("unknown %s: %s\n", pp->key, pp->value);
        return 0;
    }
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        if (find_key(NULL, pp->value, *klist)) {
            TEST_info("Duplicate key %s", pp->value);
            return 0;
        }
        if (!TEST_ptr(key = OPENSSL_malloc(sizeof(*key))))
            return 0;
        key->name = take_value(pp);
        key->key = pkey;
        key->next = *klist;
        *klist = key;

        /* Go back and start a new stanza. */
        if ((t->s.numpairs - skip_availablein) != 1)
            TEST_info("Line %d: missing blank line\n", t->s.curr);
        goto top;
    }

    /* Find the test, based on first keyword. */
    if (!TEST_ptr(t->meth = find_test(pp->key)))
        return 0;
    if (!t->meth->init(t, pp->value)) {
        TEST_error("unknown %s: %s\n", pp->key, pp->value);
        return 0;
    }
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        } else if (strcmp(pp->key, "Availablein") == 0) {
            TEST_info("Line %d: 'Availablein' should be the first option",
                      t->s.curr);
            return 0;
        } else if (strcmp(pp->key, "Result") == 0) {
            if (t->expected_err != NULL) {
                TEST_info("Line %d: multiple result lines", t->s.curr);
                return 0;
            }
            t->expected_err = take_value(pp);
        } else if (strcmp(pp->key, "Function") == 0) {
            /* Ignore old line. */
        } else if (strcmp(pp->key, "Reason") == 0) {
            if (t->reason != NULL) {
                TEST_info("Line %d: multiple reason lines", t->s.curr);
                return 0;
            }
            t->reason = take_value(pp);
        } else {
            /* Must be test specific line: try to parse it */
            int rv = t->meth->parse(t, pp->key, pp->value);

            if (rv == 0) {
                TEST_info("Line %d: unknown keyword %s", t->s.curr, pp->key);
                return 0;
            }
            if (rv < 0) {
                TEST_info("Line %d: error processing keyword %s = %s\n",
                          t->s.curr, pp->key, pp->value);
                return 0;
            }
        }
    }

    return 1;
}

static int run_file_tests(int i)
{
    EVP_TEST *t;
    const char *testfile = test_get_argument(i);
    int c;

    if (!TEST_ptr(t = OPENSSL_zalloc(sizeof(*t))))
        return 0;
    if (!test_start_file(&t->s, testfile)) {
        OPENSSL_free(t);
        return 0;
    }

    while (!BIO_eof(t->s.fp)) {
        c = parse(t);
        if (t->skip) {
            t->s.numskip++;
            continue;
        }
        if (c == 0 || !run_test(t)) {
            t->s.errors++;
            break;
        }
    }
    test_end_file(&t->s);
    clear_test(t);

    free_key_list(public_keys);
    free_key_list(private_keys);
    BIO_free(t->s.key);
    c = t->s.errors;
    OPENSSL_free(t);
    return c == 0;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
        { "config", OPT_CONFIG_FILE, '<',
          "The configuration file to use for the libctx" },
        { OPT_HELP_STR, 1, '-', "file\tFile to run tests on.\n" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    size_t n;
    char *config_file = NULL;

    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONFIG_FILE:
            config_file = opt_arg();
            break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    /*
     * Load the provider via configuration into the created library context.
     * Load the 'null' provider into the default library context to ensure that
     * the tests do not fallback to using the default provider.
     */
    if (!test_get_libctx(&libctx, &prov_null, config_file, NULL, NULL))
        return 0;

    n = test_get_argument_count();
    if (n == 0)
        return 0;

    ADD_ALL_TESTS(run_file_tests, n);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(prov_null);
    OSSL_LIB_CTX_free(libctx);
}

#define STR_STARTS_WITH(str, pre) OPENSSL_strncasecmp(pre, str, strlen(pre)) == 0
#define STR_ENDS_WITH(str, pre)                                                \
strlen(str) < strlen(pre) ? 0 : (OPENSSL_strcasecmp(pre, str + strlen(str) - strlen(pre)) == 0)

static int is_digest_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE"))
        return 1;
#endif
#ifdef OPENSSL_NO_MD2
    if (OPENSSL_strcasecmp(name, "MD2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MDC2
    if (OPENSSL_strcasecmp(name, "MDC2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD4
    if (OPENSSL_strcasecmp(name, "MD4") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD5
    if (OPENSSL_strcasecmp(name, "MD5") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_RMD160
    if (OPENSSL_strcasecmp(name, "RIPEMD160") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_SM3
    if (OPENSSL_strcasecmp(name, "SM3") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
    if (OPENSSL_strcasecmp(name, "WHIRLPOOL") == 0)
        return 1;
#endif
    return 0;
}

static int is_pkey_disabled(const char *name)
{
#ifdef OPENSSL_NO_EC
    if (STR_STARTS_WITH(name, "EC"))
        return 1;
#endif
#ifdef OPENSSL_NO_DH
    if (STR_STARTS_WITH(name, "DH"))
        return 1;
#endif
#ifdef OPENSSL_NO_DSA
    if (STR_STARTS_WITH(name, "DSA"))
        return 1;
#endif
    return 0;
}

static int is_mac_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE2BMAC")
        || STR_STARTS_WITH(name, "BLAKE2SMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_CMAC
    if (STR_STARTS_WITH(name, "CMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_STARTS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIPHASH
    if (STR_STARTS_WITH(name, "SipHash"))
        return 1;
#endif
    return 0;
}
static int is_kdf_disabled(const char *name)
{
#ifdef OPENSSL_NO_SCRYPT
    if (STR_ENDS_WITH(name, "SCRYPT"))
        return 1;
#endif
    return 0;
}

static int is_cipher_disabled(const char *name)
{
#ifdef OPENSSL_NO_ARIA
    if (STR_STARTS_WITH(name, "ARIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_BF
    if (STR_STARTS_WITH(name, "BF"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAMELLIA
    if (STR_STARTS_WITH(name, "CAMELLIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAST
    if (STR_STARTS_WITH(name, "CAST"))
        return 1;
#endif
#ifdef OPENSSL_NO_CHACHA
    if (STR_STARTS_WITH(name, "CHACHA"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_ENDS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_DES
    if (STR_STARTS_WITH(name, "DES"))
        return 1;
    if (STR_ENDS_WITH(name, "3DESwrap"))
        return 1;
#endif
#ifdef OPENSSL_NO_OCB
    if (STR_ENDS_WITH(name, "OCB"))
        return 1;
#endif
#ifdef OPENSSL_NO_IDEA
    if (STR_STARTS_WITH(name, "IDEA"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC2
    if (STR_STARTS_WITH(name, "RC2"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC4
    if (STR_STARTS_WITH(name, "RC4"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC5
    if (STR_STARTS_WITH(name, "RC5"))
        return 1;
#endif
#ifdef OPENSSL_NO_SEED
    if (STR_STARTS_WITH(name, "SEED"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIV
    if (STR_ENDS_WITH(name, "SIV"))
        return 1;
#endif
#ifdef OPENSSL_NO_SM4
    if (STR_STARTS_WITH(name, "SM4"))
        return 1;
#endif
    return 0;
}
    if (t->skip == 1) {
        /* TEST_info("skipping %s %s", pp->key, pp->value); */
        return 0;
    }

    for (pp++, i = 1; i < (t->s.numpairs - skip_availablein); pp++, i++) {
        if (strcmp(pp->key, "Securitycheck") == 0) {
#if defined(OPENSSL_NO_FIPS_SECURITYCHECKS)
#else
            if (!securitycheck_enabled())
#endif
            {
                TEST_info("skipping, Securitycheck is disabled: %s:%d",
                          t->s.test_file, t->s.start);
                t->skip = 1;
                return 0;
            }
        } else if (strcmp(pp->key, "Availablein") == 0) {
            TEST_info("Line %d: 'Availablein' should be the first option",
                      t->s.curr);
            return 0;
        } else if (strcmp(pp->key, "Result") == 0) {
            if (t->expected_err != NULL) {
                TEST_info("Line %d: multiple result lines", t->s.curr);
                return 0;
            }
            t->expected_err = take_value(pp);
        } else if (strcmp(pp->key, "Function") == 0) {
            /* Ignore old line. */
        } else if (strcmp(pp->key, "Reason") == 0) {
            if (t->reason != NULL) {
                TEST_info("Line %d: multiple reason lines", t->s.curr);
                return 0;
            }
            t->reason = take_value(pp);
        } else {
            /* Must be test specific line: try to parse it */
            int rv = t->meth->parse(t, pp->key, pp->value);

            if (rv == 0) {
                TEST_info("Line %d: unknown keyword %s", t->s.curr, pp->key);
                return 0;
            }
            if (rv < 0) {
                TEST_info("Line %d: error processing keyword %s = %s\n",
                          t->s.curr, pp->key, pp->value);
                return 0;
            }
        }
    }

    return 1;
}

static int run_file_tests(int i)
{
    EVP_TEST *t;
    const char *testfile = test_get_argument(i);
    int c;

    if (!TEST_ptr(t = OPENSSL_zalloc(sizeof(*t))))
        return 0;
    if (!test_start_file(&t->s, testfile)) {
        OPENSSL_free(t);
        return 0;
    }

    while (!BIO_eof(t->s.fp)) {
        c = parse(t);
        if (t->skip) {
            t->s.numskip++;
            continue;
        }
        if (c == 0 || !run_test(t)) {
            t->s.errors++;
            break;
        }
    }
    test_end_file(&t->s);
    clear_test(t);

    free_key_list(public_keys);
    free_key_list(private_keys);
    BIO_free(t->s.key);
    c = t->s.errors;
    OPENSSL_free(t);
    return c == 0;
}

const OPTIONS *test_get_options(void)
{
    static const OPTIONS test_options[] = {
        OPT_TEST_OPTIONS_WITH_EXTRA_USAGE("[file...]\n"),
        { "config", OPT_CONFIG_FILE, '<',
          "The configuration file to use for the libctx" },
        { OPT_HELP_STR, 1, '-', "file\tFile to run tests on.\n" },
        { NULL }
    };
    return test_options;
}

int setup_tests(void)
{
    size_t n;
    char *config_file = NULL;

    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONFIG_FILE:
            config_file = opt_arg();
            break;
        case OPT_TEST_CASES:
           break;
        default:
        case OPT_ERR:
            return 0;
        }
    }

    /*
     * Load the provider via configuration into the created library context.
     * Load the 'null' provider into the default library context to ensure that
     * the tests do not fallback to using the default provider.
     */
    if (!test_get_libctx(&libctx, &prov_null, config_file, NULL, NULL))
        return 0;

    n = test_get_argument_count();
    if (n == 0)
        return 0;

    ADD_ALL_TESTS(run_file_tests, n);
    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(prov_null);
    OSSL_LIB_CTX_free(libctx);
}

#define STR_STARTS_WITH(str, pre) OPENSSL_strncasecmp(pre, str, strlen(pre)) == 0
#define STR_ENDS_WITH(str, pre)                                                \
strlen(str) < strlen(pre) ? 0 : (OPENSSL_strcasecmp(pre, str + strlen(str) - strlen(pre)) == 0)

static int is_digest_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE"))
        return 1;
#endif
#ifdef OPENSSL_NO_MD2
    if (OPENSSL_strcasecmp(name, "MD2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MDC2
    if (OPENSSL_strcasecmp(name, "MDC2") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD4
    if (OPENSSL_strcasecmp(name, "MD4") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_MD5
    if (OPENSSL_strcasecmp(name, "MD5") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_RMD160
    if (OPENSSL_strcasecmp(name, "RIPEMD160") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_SM3
    if (OPENSSL_strcasecmp(name, "SM3") == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_WHIRLPOOL
    if (OPENSSL_strcasecmp(name, "WHIRLPOOL") == 0)
        return 1;
#endif
    return 0;
}

static int is_pkey_disabled(const char *name)
{
#ifdef OPENSSL_NO_EC
    if (STR_STARTS_WITH(name, "EC"))
        return 1;
#endif
#ifdef OPENSSL_NO_DH
    if (STR_STARTS_WITH(name, "DH"))
        return 1;
#endif
#ifdef OPENSSL_NO_DSA
    if (STR_STARTS_WITH(name, "DSA"))
        return 1;
#endif
    return 0;
}

static int is_mac_disabled(const char *name)
{
#ifdef OPENSSL_NO_BLAKE2
    if (STR_STARTS_WITH(name, "BLAKE2BMAC")
        || STR_STARTS_WITH(name, "BLAKE2SMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_CMAC
    if (STR_STARTS_WITH(name, "CMAC"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_STARTS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIPHASH
    if (STR_STARTS_WITH(name, "SipHash"))
        return 1;
#endif
    return 0;
}
static int is_kdf_disabled(const char *name)
{
#ifdef OPENSSL_NO_SCRYPT
    if (STR_ENDS_WITH(name, "SCRYPT"))
        return 1;
#endif
    return 0;
}

static int is_cipher_disabled(const char *name)
{
#ifdef OPENSSL_NO_ARIA
    if (STR_STARTS_WITH(name, "ARIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_BF
    if (STR_STARTS_WITH(name, "BF"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAMELLIA
    if (STR_STARTS_WITH(name, "CAMELLIA"))
        return 1;
#endif
#ifdef OPENSSL_NO_CAST
    if (STR_STARTS_WITH(name, "CAST"))
        return 1;
#endif
#ifdef OPENSSL_NO_CHACHA
    if (STR_STARTS_WITH(name, "CHACHA"))
        return 1;
#endif
#ifdef OPENSSL_NO_POLY1305
    if (STR_ENDS_WITH(name, "Poly1305"))
        return 1;
#endif
#ifdef OPENSSL_NO_DES
    if (STR_STARTS_WITH(name, "DES"))
        return 1;
    if (STR_ENDS_WITH(name, "3DESwrap"))
        return 1;
#endif
#ifdef OPENSSL_NO_OCB
    if (STR_ENDS_WITH(name, "OCB"))
        return 1;
#endif
#ifdef OPENSSL_NO_IDEA
    if (STR_STARTS_WITH(name, "IDEA"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC2
    if (STR_STARTS_WITH(name, "RC2"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC4
    if (STR_STARTS_WITH(name, "RC4"))
        return 1;
#endif
#ifdef OPENSSL_NO_RC5
    if (STR_STARTS_WITH(name, "RC5"))
        return 1;
#endif
#ifdef OPENSSL_NO_SEED
    if (STR_STARTS_WITH(name, "SEED"))
        return 1;
#endif
#ifdef OPENSSL_NO_SIV
    if (STR_ENDS_WITH(name, "SIV"))
        return 1;
#endif
#ifdef OPENSSL_NO_SM4
    if (STR_STARTS_WITH(name, "SM4"))
        return 1;
#endif
    return 0;
}