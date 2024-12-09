{
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;
    EVP_PKEY *just_params = NULL;
    EVP_PKEY *params_and_priv = NULL;
    EVP_PKEY *params_and_pub = NULL;
    EVP_PKEY *params_and_keypair = NULL;
    BIGNUM *priv = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our pkey object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    if (!TEST_ptr(priv = BN_bin2bn(ec_priv, sizeof(ec_priv), NULL)))
        goto err;

    /* Test !priv and !pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(just_params = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    if (!test_selection(just_params, OSSL_KEYMGMT_SELECT_ALL_PARAMETERS)
        || test_selection(just_params, OSSL_KEYMGMT_SELECT_KEYPAIR))
        goto err;

    /* Test priv and !pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY,
                                             priv)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_priv = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    /*
     * We indicate only parameters here, in spite of having built a key that
     * has a private part, because the PEM_write_bio_PrivateKey_ex call is
     * expected to fail because it does not support exporting a private EC
     * key without a corresponding public key
     */
    if (!test_selection(params_and_priv, OSSL_KEYMGMT_SELECT_ALL_PARAMETERS)
        || test_selection(params_and_priv, OSSL_KEYMGMT_SELECT_PUBLIC_KEY))
        goto err;

    /* Test !priv and pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                                                       OSSL_PKEY_PARAM_PUB_KEY,
                                                       ec_pub, sizeof(ec_pub))))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_pub = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    if (!test_selection(params_and_pub, OSSL_KEYMGMT_SELECT_PUBLIC_KEY)
        || test_selection(params_and_pub, OSSL_KEYMGMT_SELECT_PRIVATE_KEY))
        goto err;

    /* Test priv and pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                                                       OSSL_PKEY_PARAM_PUB_KEY,
                                                       ec_pub, sizeof(ec_pub)))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY,
                                             priv)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_keypair = make_key_fromdata("EC", params)))
        goto err;

    if (!test_selection(params_and_keypair, EVP_PKEY_KEYPAIR))
        goto err;

    /* Try key equality */
    if (!TEST_int_gt(EVP_PKEY_parameters_eq(just_params, just_params), 0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_pub),
                        0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_priv),
                        0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_keypair),
                        0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_pub, params_and_pub), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_priv, params_and_priv), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_pub), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_priv), 0))
        goto err;

    ret = 1;
 err:
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    EVP_PKEY_free(just_params);
    EVP_PKEY_free(params_and_priv);
    EVP_PKEY_free(params_and_pub);
    EVP_PKEY_free(params_and_keypair);
    BN_free(priv);

    return ret;
}

/* Test that using a legacy EC key with only a private key in it works */
# ifndef OPENSSL_NO_DEPRECATED_3_0
static int test_EC_priv_only_legacy(void)
{
    BIGNUM *priv = NULL;
    int ret = 0;
    EC_KEY *eckey = NULL;
    EVP_PKEY *pkey = NULL, *dup_pk = NULL;
    EVP_MD_CTX *ctx = NULL;

    /* Create the low level EC_KEY */
    if (!TEST_ptr(priv = BN_bin2bn(ec_priv, sizeof(ec_priv), NULL)))
        goto err;

    eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!TEST_ptr(eckey))
        goto err;

    if (!TEST_true(EC_KEY_set_private_key(eckey, priv)))
        goto err;

    pkey = EVP_PKEY_new();
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_true(EVP_PKEY_assign_EC_KEY(pkey, eckey)))
        goto err;
    eckey = NULL;

    while (dup_pk == NULL) {
        ret = 0;
        ctx = EVP_MD_CTX_new();
        if (!TEST_ptr(ctx))
            goto err;

        /*
         * The EVP_DigestSignInit function should create the key on the
         * provider side which is sufficient for this test.
         */
        if (!TEST_true(EVP_DigestSignInit_ex(ctx, NULL, NULL, testctx,
                                             testpropq, pkey, NULL)))
            goto err;
        EVP_MD_CTX_free(ctx);
        ctx = NULL;

        if (!TEST_ptr(dup_pk = EVP_PKEY_dup(pkey)))
            goto err;
        /* EVP_PKEY_eq() returns -2 with missing public keys */
        ret = TEST_int_eq(EVP_PKEY_eq(pkey, dup_pk), -2);
        EVP_PKEY_free(pkey);
        pkey = dup_pk;
        if (!ret)
            goto err;
    }
{
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;
    EVP_PKEY *just_params = NULL;
    EVP_PKEY *params_and_priv = NULL;
    EVP_PKEY *params_and_pub = NULL;
    EVP_PKEY *params_and_keypair = NULL;
    BIGNUM *priv = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our pkey object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    if (!TEST_ptr(priv = BN_bin2bn(ec_priv, sizeof(ec_priv), NULL)))
        goto err;

    /* Test !priv and !pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(just_params = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    if (!test_selection(just_params, OSSL_KEYMGMT_SELECT_ALL_PARAMETERS)
        || test_selection(just_params, OSSL_KEYMGMT_SELECT_KEYPAIR))
        goto err;

    /* Test priv and !pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY,
                                             priv)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_priv = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    /*
     * We indicate only parameters here, in spite of having built a key that
     * has a private part, because the PEM_write_bio_PrivateKey_ex call is
     * expected to fail because it does not support exporting a private EC
     * key without a corresponding public key
     */
    if (!test_selection(params_and_priv, OSSL_KEYMGMT_SELECT_ALL_PARAMETERS)
        || test_selection(params_and_priv, OSSL_KEYMGMT_SELECT_PUBLIC_KEY))
        goto err;

    /* Test !priv and pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                                                       OSSL_PKEY_PARAM_PUB_KEY,
                                                       ec_pub, sizeof(ec_pub))))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_pub = make_key_fromdata("EC", params)))
        goto err;

    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    params = NULL;
    bld = NULL;

    if (!test_selection(params_and_pub, OSSL_KEYMGMT_SELECT_PUBLIC_KEY)
        || test_selection(params_and_pub, OSSL_KEYMGMT_SELECT_PRIVATE_KEY))
        goto err;

    /* Test priv and pub */
    if (!TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_true(OSSL_PARAM_BLD_push_utf8_string(bld,
                                                      OSSL_PKEY_PARAM_GROUP_NAME,
                                                      "P-256", 0))
        || !TEST_true(OSSL_PARAM_BLD_push_octet_string(bld,
                                                       OSSL_PKEY_PARAM_PUB_KEY,
                                                       ec_pub, sizeof(ec_pub)))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY,
                                             priv)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld))
        || !TEST_ptr(params_and_keypair = make_key_fromdata("EC", params)))
        goto err;

    if (!test_selection(params_and_keypair, EVP_PKEY_KEYPAIR))
        goto err;

    /* Try key equality */
    if (!TEST_int_gt(EVP_PKEY_parameters_eq(just_params, just_params), 0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_pub),
                        0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_priv),
                        0)
        || !TEST_int_gt(EVP_PKEY_parameters_eq(just_params, params_and_keypair),
                        0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_pub, params_and_pub), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_priv, params_and_priv), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_pub), 0)
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_priv), 0))
        goto err;

    ret = 1;
 err:
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    EVP_PKEY_free(just_params);
    EVP_PKEY_free(params_and_priv);
    EVP_PKEY_free(params_and_pub);
    EVP_PKEY_free(params_and_keypair);
    BN_free(priv);

    return ret;
}

/* Test that using a legacy EC key with only a private key in it works */
# ifndef OPENSSL_NO_DEPRECATED_3_0
static int test_EC_priv_only_legacy(void)
{
    BIGNUM *priv = NULL;
    int ret = 0;
    EC_KEY *eckey = NULL;
    EVP_PKEY *pkey = NULL, *dup_pk = NULL;
    EVP_MD_CTX *ctx = NULL;

    /* Create the low level EC_KEY */
    if (!TEST_ptr(priv = BN_bin2bn(ec_priv, sizeof(ec_priv), NULL)))
        goto err;

    eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (!TEST_ptr(eckey))
        goto err;

    if (!TEST_true(EC_KEY_set_private_key(eckey, priv)))
        goto err;

    pkey = EVP_PKEY_new();
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_true(EVP_PKEY_assign_EC_KEY(pkey, eckey)))
        goto err;
    eckey = NULL;

    while (dup_pk == NULL) {
        ret = 0;
        ctx = EVP_MD_CTX_new();
        if (!TEST_ptr(ctx))
            goto err;

        /*
         * The EVP_DigestSignInit function should create the key on the
         * provider side which is sufficient for this test.
         */
        if (!TEST_true(EVP_DigestSignInit_ex(ctx, NULL, NULL, testctx,
                                             testpropq, pkey, NULL)))
            goto err;
        EVP_MD_CTX_free(ctx);
        ctx = NULL;

        if (!TEST_ptr(dup_pk = EVP_PKEY_dup(pkey)))
            goto err;
        /* EVP_PKEY_eq() returns -2 with missing public keys */
        ret = TEST_int_eq(EVP_PKEY_eq(pkey, dup_pk), -2);
        EVP_PKEY_free(pkey);
        pkey = dup_pk;
        if (!ret)
            goto err;
    }
{
    EVP_PKEY *params = NULL, *key = NULL;
    EVP_PKEY_CTX *pctx = NULL, *kctx = NULL;
    int enc;
    int ret = 0;

    enc = ec_encodings[idx].encoding;

    /* Create key parameters */
    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "EC", NULL))
        || !TEST_int_gt(EVP_PKEY_paramgen_init(pctx), 0)
        || !TEST_int_gt(EVP_PKEY_CTX_set_group_name(pctx, "P-256"), 0)
        || !TEST_int_gt(EVP_PKEY_CTX_set_ec_param_enc(pctx, enc), 0)
        || !TEST_true(EVP_PKEY_paramgen(pctx, &params))
        || !TEST_ptr(params))
        goto done;

    /* Create key */
    if (!TEST_ptr(kctx = EVP_PKEY_CTX_new_from_pkey(testctx, params, NULL))
        || !TEST_int_gt(EVP_PKEY_keygen_init(kctx), 0)
        || !TEST_true(EVP_PKEY_keygen(kctx, &key))
        || !TEST_ptr(key))
        goto done;

    /* Check that the encoding got all the way into the key */
    if (!TEST_true(evp_keymgmt_util_export(key, OSSL_KEYMGMT_SELECT_ALL,
                                           ec_export_get_encoding_cb, &enc))
        || !TEST_int_eq(enc, ec_encodings[idx].encoding))
        goto done;

    ret = 1;
 done:
    EVP_PKEY_free(key);
    EVP_PKEY_free(params);
    EVP_PKEY_CTX_free(kctx);
    EVP_PKEY_CTX_free(pctx);
    return ret;
}
#endif

#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)

static int test_EVP_SM2_verify(void)
{
    const char *pubkey =
        "-----BEGIN PUBLIC KEY-----\n"
        "MFkwEwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAEp1KLWq1ZE2jmoAnnBJE1LBGxVr18\n"
        "YvvqECWCpXfAQ9qUJ+UmthnUPf0iM3SaXKHe6PlLIDyNlWMWb9RUh/yU3g==\n"
        "-----END PUBLIC KEY-----\n";

    const char *msg = "message digest";
    const char *id = "ALICE123@YAHOO.COM";

    const uint8_t signature[] = {
        0x30, 0x44, 0x02, 0x20, 0x5b, 0xdb, 0xab, 0x81, 0x4f, 0xbb,
        0x8b, 0x69, 0xb1, 0x05, 0x9c, 0x99, 0x3b, 0xb2, 0x45, 0x06,
        0x4a, 0x30, 0x15, 0x59, 0x84, 0xcd, 0xee, 0x30, 0x60, 0x36,
        0x57, 0x87, 0xef, 0x5c, 0xd0, 0xbe, 0x02, 0x20, 0x43, 0x8d,
        0x1f, 0xc7, 0x77, 0x72, 0x39, 0xbb, 0x72, 0xe1, 0xfd, 0x07,
        0x58, 0xd5, 0x82, 0xc8, 0x2d, 0xba, 0x3b, 0x2c, 0x46, 0x24,
        0xe3, 0x50, 0xff, 0x04, 0xc7, 0xa0, 0x71, 0x9f, 0xa4, 0x70
    };

    int rc = 0;
    BIO *bio = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD_CTX *mctx = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_MD *sm3 = NULL;

    bio = BIO_new_mem_buf(pubkey, strlen(pubkey));
    if (!TEST_true(bio != NULL))
        goto done;

    pkey = PEM_read_bio_PUBKEY_ex(bio, NULL, NULL, NULL, testctx, testpropq);
    if (!TEST_true(pkey != NULL))
        goto done;

    if (!TEST_true(EVP_PKEY_is_a(pkey, "SM2")))
        goto done;

    if (!TEST_ptr(mctx = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_pkey(testctx, pkey, testpropq)))
        goto done;

    EVP_MD_CTX_set_pkey_ctx(mctx, pctx);

    if (!TEST_ptr(sm3 = EVP_MD_fetch(testctx, "sm3", testpropq)))
        goto done;

    if (!TEST_true(EVP_DigestVerifyInit(mctx, NULL, sm3, NULL, pkey)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(pctx, id, strlen(id)), 0))
        goto done;

    if (!TEST_true(EVP_DigestVerifyUpdate(mctx, msg, strlen(msg))))
        goto done;

    if (!TEST_int_gt(EVP_DigestVerifyFinal(mctx, signature, sizeof(signature)), 0))
        goto done;
    rc = 1;

 done:
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    EVP_MD_CTX_free(mctx);
    EVP_MD_free(sm3);
    return rc;
}

static int test_EVP_SM2(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *pkeyparams = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY_CTX *kctx = NULL;
    EVP_PKEY_CTX *sctx = NULL;
    size_t sig_len = 0;
    unsigned char *sig = NULL;
    EVP_MD_CTX *md_ctx = NULL;
    EVP_MD_CTX *md_ctx_verify = NULL;
    EVP_PKEY_CTX *cctx = NULL;
    EVP_MD *check_md = NULL;

    uint8_t ciphertext[128];
    size_t ctext_len = sizeof(ciphertext);

    uint8_t plaintext[8];
    size_t ptext_len = sizeof(plaintext);

    uint8_t sm2_id[] = {1, 2, 3, 4, 'l', 'e', 't', 't', 'e', 'r'};

    OSSL_PARAM sparams[2] = {OSSL_PARAM_END, OSSL_PARAM_END};
    OSSL_PARAM gparams[2] = {OSSL_PARAM_END, OSSL_PARAM_END};
    int i;
    char mdname[OSSL_MAX_NAME_SIZE];

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx,
                                                    "SM2", testpropq)))
        goto done;

    if (!TEST_true(EVP_PKEY_paramgen_init(pctx) == 1))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_sm2), 0))
        goto done;

    if (!TEST_true(EVP_PKEY_paramgen(pctx, &pkeyparams)))
        goto done;

    if (!TEST_ptr(kctx = EVP_PKEY_CTX_new_from_pkey(testctx,
                                                    pkeyparams, testpropq)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_keygen_init(kctx), 0))
        goto done;

    if (!TEST_true(EVP_PKEY_keygen(kctx, &pkey)))
        goto done;

    if (!TEST_ptr(md_ctx = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(md_ctx_verify = EVP_MD_CTX_new()))
        goto done;

    if (!TEST_ptr(sctx = EVP_PKEY_CTX_new_from_pkey(testctx, pkey, testpropq)))
        goto done;

    EVP_MD_CTX_set_pkey_ctx(md_ctx, sctx);
    EVP_MD_CTX_set_pkey_ctx(md_ctx_verify, sctx);

    if (!TEST_ptr(check_md = EVP_MD_fetch(testctx, "sm3", testpropq)))
        goto done;

    if (!TEST_true(EVP_DigestSignInit(md_ctx, NULL, check_md, NULL, pkey)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(sctx, sm2_id, sizeof(sm2_id)), 0))
        goto done;

    if (!TEST_true(EVP_DigestSignUpdate(md_ctx, kMsg, sizeof(kMsg))))
        goto done;

    /* Determine the size of the signature. */
    if (!TEST_true(EVP_DigestSignFinal(md_ctx, NULL, &sig_len)))
        goto done;

    if (!TEST_ptr(sig = OPENSSL_malloc(sig_len)))
        goto done;

    if (!TEST_true(EVP_DigestSignFinal(md_ctx, sig, &sig_len)))
        goto done;

    /* Ensure that the signature round-trips. */

    if (!TEST_true(EVP_DigestVerifyInit(md_ctx_verify, NULL, check_md, NULL,
                                        pkey)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(sctx, sm2_id, sizeof(sm2_id)), 0))
        goto done;

    if (!TEST_true(EVP_DigestVerifyUpdate(md_ctx_verify, kMsg, sizeof(kMsg))))
        goto done;

    if (!TEST_int_gt(EVP_DigestVerifyFinal(md_ctx_verify, sig, sig_len), 0))
        goto done;

    /*
     * Try verify again with non-matching 0 length id but ensure that it can
     * be set on the context and overrides the previous value.
     */

    if (!TEST_true(EVP_DigestVerifyInit(md_ctx_verify, NULL, check_md, NULL,
                                        pkey)))
        goto done;

    if (!TEST_int_gt(EVP_PKEY_CTX_set1_id(sctx, NULL, 0), 0))
        goto done;

    if (!TEST_true(EVP_DigestVerifyUpdate(md_ctx_verify, kMsg, sizeof(kMsg))))
        goto done;

    if (!TEST_int_eq(EVP_DigestVerifyFinal(md_ctx_verify, sig, sig_len), 0))
        goto done;

    /* now check encryption/decryption */

    gparams[0] = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_DIGEST,
                                                  mdname, sizeof(mdname));
    for (i = 0; i < 2; i++) {
        const char *mdnames[] = {
#ifndef OPENSSL_NO_SM3
            "SM3",
#else
            NULL,
#endif
            "SHA2-256" };
        EVP_PKEY_CTX_free(cctx);

        if (mdnames[i] == NULL)
            continue;

        sparams[0] =
            OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_DIGEST,
                                             (char *)mdnames[i], 0);

        if (!TEST_ptr(cctx = EVP_PKEY_CTX_new_from_pkey(testctx,
                                                        pkey, testpropq)))
            goto done;

        if (!TEST_true(EVP_PKEY_encrypt_init(cctx)))
            goto done;

        if (!TEST_true(EVP_PKEY_CTX_set_params(cctx, sparams)))
            goto done;

        if (!TEST_true(EVP_PKEY_encrypt(cctx, ciphertext, &ctext_len, kMsg,
                                        sizeof(kMsg))))
            goto done;

        if (!TEST_true(EVP_PKEY_decrypt_init(cctx)))
            goto done;

        if (!TEST_true(EVP_PKEY_CTX_set_params(cctx, sparams)))
            goto done;

        if (!TEST_int_gt(EVP_PKEY_decrypt(cctx, plaintext, &ptext_len, ciphertext,
                                        ctext_len), 0))
            goto done;

        if (!TEST_true(EVP_PKEY_CTX_get_params(cctx, gparams)))
            goto done;

        /*
         * Test we're still using the digest we think we are.
         * Because of aliases, the easiest is to fetch the digest and
         * check the name with EVP_MD_is_a().
         */
        EVP_MD_free(check_md);
        if (!TEST_ptr(check_md = EVP_MD_fetch(testctx, mdname, testpropq)))
            goto done;
        if (!TEST_true(EVP_MD_is_a(check_md, mdnames[i]))) {
            TEST_info("Fetched md %s isn't %s", mdname, mdnames[i]);
            goto done;
        }

        if (!TEST_true(ptext_len == sizeof(kMsg)))
            goto done;

        if (!TEST_true(memcmp(plaintext, kMsg, sizeof(kMsg)) == 0))
            goto done;
    }

    ret = 1;
done:
    EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_CTX_free(kctx);
    EVP_PKEY_CTX_free(sctx);
    EVP_PKEY_CTX_free(cctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(pkeyparams);
    EVP_MD_CTX_free(md_ctx);
    EVP_MD_CTX_free(md_ctx_verify);
    EVP_MD_free(check_md);
    OPENSSL_free(sig);
    return ret;
}

#endif

static struct keys_st {
    int type;
    char *priv;
    char *pub;
} keys[] = {
    {
        EVP_PKEY_HMAC, "0123456789", NULL
    },
    {
        EVP_PKEY_HMAC, "", NULL
#ifndef OPENSSL_NO_POLY1305
    }, {
        EVP_PKEY_POLY1305, "01234567890123456789012345678901", NULL
#endif
#ifndef OPENSSL_NO_SIPHASH
    }, {
        EVP_PKEY_SIPHASH, "0123456789012345", NULL
#endif
    },
#ifndef OPENSSL_NO_EC
    {
        EVP_PKEY_X25519, "01234567890123456789012345678901",
        "abcdefghijklmnopqrstuvwxyzabcdef"
    }, {
        EVP_PKEY_ED25519, "01234567890123456789012345678901",
        "abcdefghijklmnopqrstuvwxyzabcdef"
    }, {
        EVP_PKEY_X448,
        "01234567890123456789012345678901234567890123456789012345",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcd"
    }, {
        EVP_PKEY_ED448,
        "012345678901234567890123456789012345678901234567890123456",
        "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcde"
    }
#endif
};

static int test_set_get_raw_keys_int(int tst, int pub, int uselibctx)
{
    int ret = 0;
    unsigned char buf[80];
    unsigned char *in;
    size_t inlen, len = 0, shortlen = 1;
    EVP_PKEY *pkey;

    /* Check if this algorithm supports public keys */
    if (pub && keys[tst].pub == NULL)
        return 1;

    memset(buf, 0, sizeof(buf));

    if (pub) {
#ifndef OPENSSL_NO_EC
        inlen = strlen(keys[tst].pub);
        in = (unsigned char *)keys[tst].pub;
        if (uselibctx) {
            pkey = EVP_PKEY_new_raw_public_key_ex(
                        testctx,
                        OBJ_nid2sn(keys[tst].type),
                        NULL,
                        in,
                        inlen);
        } else {
            pkey = EVP_PKEY_new_raw_public_key(keys[tst].type,
                                               NULL,
                                               in,
                                               inlen);
        }
#else
        return 1;
#endif
    } else {
        inlen = strlen(keys[tst].priv);
        in = (unsigned char *)keys[tst].priv;
        if (uselibctx) {
            pkey = EVP_PKEY_new_raw_private_key_ex(
                        testctx, OBJ_nid2sn(keys[tst].type),
                        NULL,
                        in,
                        inlen);
        } else {
            pkey = EVP_PKEY_new_raw_private_key(keys[tst].type,
                                                NULL,
                                                in,
                                                inlen);
        }
    }

    if (!TEST_ptr(pkey)
            || !TEST_int_eq(EVP_PKEY_eq(pkey, pkey), 1)
            || (!pub && !TEST_true(EVP_PKEY_get_raw_private_key(pkey, NULL, &len)))
            || (pub && !TEST_true(EVP_PKEY_get_raw_public_key(pkey, NULL, &len)))
            || !TEST_true(len == inlen))
        goto done;
    if (tst != 1) {
        /*
         * Test that supplying a buffer that is too small fails. Doesn't apply
         * to HMAC with a zero length key
         */
        if ((!pub && !TEST_false(EVP_PKEY_get_raw_private_key(pkey, buf,
                                                                 &shortlen)))
                || (pub && !TEST_false(EVP_PKEY_get_raw_public_key(pkey, buf,
                                                                   &shortlen))))
            goto done;
    }
    if ((!pub && !TEST_true(EVP_PKEY_get_raw_private_key(pkey, buf, &len)))
            || (pub && !TEST_true(EVP_PKEY_get_raw_public_key(pkey, buf, &len)))
            || !TEST_mem_eq(in, inlen, buf, len))
        goto done;

    ret = 1;
 done:
    EVP_PKEY_free(pkey);
    return ret;
}

static int test_set_get_raw_keys(int tst)
{
    return (nullprov != NULL || test_set_get_raw_keys_int(tst, 0, 0))
           && test_set_get_raw_keys_int(tst, 0, 1)
           && (nullprov != NULL || test_set_get_raw_keys_int(tst, 1, 0))
           && test_set_get_raw_keys_int(tst, 1, 1);
}

#ifndef OPENSSL_NO_DEPRECATED_3_0
static int pkey_custom_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static int pkey_custom_pub_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static int pkey_custom_param_check(EVP_PKEY *pkey)
{
    return 0xbeef;
}

static EVP_PKEY_METHOD *custom_pmeth;
#endif

static int test_EVP_PKEY_check(int i)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_CTX *ctx2 = NULL;
#endif
    const APK_DATA *ak = &keycheckdata[i];
    const unsigned char *input = ak->kder;
    size_t input_len = ak->size;
    int expected_id = ak->evptype;
    int expected_check = ak->check;
    int expected_pub_check = ak->pub_check;
    int expected_param_check = ak->param_check;
    int type = ak->type;

    if (!TEST_ptr(pkey = load_example_key(ak->keytype, input, input_len)))
        goto done;
    if (type == 0
        && !TEST_int_eq(EVP_PKEY_get_id(pkey), expected_id))
        goto done;

    if (!TEST_ptr(ctx = EVP_PKEY_CTX_new_from_pkey(testctx, pkey, testpropq)))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_check(ctx), expected_check))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_public_check(ctx), expected_pub_check))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_param_check(ctx), expected_param_check))
        goto done;

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ctx2 = EVP_PKEY_CTX_new_id(0xdefaced, NULL);
    /* assign the pkey directly, as an internal test */
    EVP_PKEY_up_ref(pkey);
    ctx2->pkey = pkey;

    if (!TEST_int_eq(EVP_PKEY_check(ctx2), 0xbeef))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_public_check(ctx2), 0xbeef))
        goto done;

    if (!TEST_int_eq(EVP_PKEY_param_check(ctx2), 0xbeef))
        goto done;
#endif

    ret = 1;

 done:
    EVP_PKEY_CTX_free(ctx);
#ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_CTX_free(ctx2);
#endif
    EVP_PKEY_free(pkey);
    return ret;
}

#ifndef OPENSSL_NO_CMAC
static int get_cmac_val(EVP_PKEY *pkey, unsigned char *mac)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    const char msg[] = "Hello World";
    size_t maclen = AES_BLOCK_SIZE;
    int ret = 1;

    if (!TEST_ptr(mdctx)
            || !TEST_true(EVP_DigestSignInit_ex(mdctx, NULL, NULL, testctx,
                                                testpropq, pkey, NULL))
            || !TEST_true(EVP_DigestSignUpdate(mdctx, msg, sizeof(msg)))
            || !TEST_true(EVP_DigestSignFinal(mdctx, mac, &maclen))
            || !TEST_size_t_eq(maclen, AES_BLOCK_SIZE))
        ret = 0;

    EVP_MD_CTX_free(mdctx);

    return ret;
}
static int test_CMAC_keygen(void)
{
    static unsigned char key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    EVP_PKEY_CTX *kctx = NULL;
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    unsigned char mac[AES_BLOCK_SIZE];
# if !defined(OPENSSL_NO_DEPRECATED_3_0)
    unsigned char mac2[AES_BLOCK_SIZE];
# endif

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    /*
     * This is a legacy method for CMACs, but should still work.
     * This verifies that it works without an ENGINE.
     */
    kctx = EVP_PKEY_CTX_new_id(EVP_PKEY_CMAC, NULL);

    /* Test a CMAC key created using the "generated" method */
    if (!TEST_int_gt(EVP_PKEY_keygen_init(kctx), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN,
                                            EVP_PKEY_CTRL_CIPHER,
                                            0, (void *)EVP_aes_256_ecb()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_ctrl(kctx, -1, EVP_PKEY_OP_KEYGEN,
                                            EVP_PKEY_CTRL_SET_MAC_KEY,
                                            sizeof(key), (void *)key), 0)
            || !TEST_int_gt(EVP_PKEY_keygen(kctx, &pkey), 0)
            || !TEST_ptr(pkey)
            || !TEST_true(get_cmac_val(pkey, mac)))
        goto done;

# if !defined(OPENSSL_NO_DEPRECATED_3_0)
    EVP_PKEY_free(pkey);

    /*
     * Test a CMAC key using the direct method, and compare with the mac
     * created above.
     */
    pkey = EVP_PKEY_new_CMAC_key(NULL, key, sizeof(key), EVP_aes_256_ecb());
    if (!TEST_ptr(pkey)
            || !TEST_true(get_cmac_val(pkey, mac2))
            || !TEST_mem_eq(mac, sizeof(mac), mac2, sizeof(mac2)))
        goto done;
# endif

    ret = 1;

 done:
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(kctx);
    return ret;
}
#endif

static int test_HKDF(void)
{
    EVP_PKEY_CTX *pctx;
    unsigned char out[20];
    size_t outlen;
    int i, ret = 0;
    unsigned char salt[] = "0123456789";
    unsigned char key[] = "012345678901234567890123456789";
    unsigned char info[] = "infostring";
    const unsigned char expected[] = {
        0xe5, 0x07, 0x70, 0x7f, 0xc6, 0x78, 0xd6, 0x54, 0x32, 0x5f, 0x7e, 0xc5,
        0x7b, 0x59, 0x3e, 0xd8, 0x03, 0x6b, 0xed, 0xca
    };
    size_t expectedlen = sizeof(expected);

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "HKDF", testpropq)))
        goto done;

    /* We do this twice to test reuse of the EVP_PKEY_CTX */
    for (i = 0; i < 2; i++) {
        outlen = sizeof(out);
        memset(out, 0, outlen);

        if (!TEST_int_gt(EVP_PKEY_derive_init(pctx), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt,
                                                            sizeof(salt) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_key(pctx, key,
                                                           sizeof(key) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_CTX_add1_hkdf_info(pctx, info,
                                                            sizeof(info) - 1), 0)
                || !TEST_int_gt(EVP_PKEY_derive(pctx, out, &outlen), 0)
                || !TEST_mem_eq(out, outlen, expected, expectedlen))
            goto done;
    }

    ret = 1;

 done:
    EVP_PKEY_CTX_free(pctx);

    return ret;
}

static int test_emptyikm_HKDF(void)
{
    EVP_PKEY_CTX *pctx;
    unsigned char out[20];
    size_t outlen;
    int ret = 0;
    unsigned char salt[] = "9876543210";
    unsigned char key[] = "";
    unsigned char info[] = "stringinfo";
    const unsigned char expected[] = {
        0x68, 0x81, 0xa5, 0x3e, 0x5b, 0x9c, 0x7b, 0x6f, 0x2e, 0xec, 0xc8, 0x47,
        0x7c, 0xfa, 0x47, 0x35, 0x66, 0x82, 0x15, 0x30
    };
    size_t expectedlen = sizeof(expected);

    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "HKDF", testpropq)))
        goto done;

    outlen = sizeof(out);
    memset(out, 0, outlen);

    if (!TEST_int_gt(EVP_PKEY_derive_init(pctx), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_salt(pctx, salt,
                                                        sizeof(salt) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_set1_hkdf_key(pctx, key,
                                                       sizeof(key) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_add1_hkdf_info(pctx, info,
                                                        sizeof(info) - 1), 0)
            || !TEST_int_gt(EVP_PKEY_derive(pctx, out, &outlen), 0)
            || !TEST_mem_eq(out, outlen, expected, expectedlen))
        goto done;

    ret = 1;

 done:
    EVP_PKEY_CTX_free(pctx);

    return ret;
}

#ifndef OPENSSL_NO_EC
static int test_X509_PUBKEY_inplace(void)
{
    int ret = 0;
    X509_PUBKEY *xp = X509_PUBKEY_new_ex(testctx, testpropq);
    const unsigned char *p = kExampleECPubKeyDER;
    size_t input_len = sizeof(kExampleECPubKeyDER);

    if (!TEST_ptr(xp))
        goto done;
    if (!TEST_ptr(d2i_X509_PUBKEY(&xp, &p, input_len)))
        goto done;

    if (!TEST_ptr(X509_PUBKEY_get0(xp)))
        goto done;

    p = kExampleBadECPubKeyDER;
    input_len = sizeof(kExampleBadECPubKeyDER);

    if (!TEST_ptr(xp = d2i_X509_PUBKEY(&xp, &p, input_len)))
        goto done;

    if (!TEST_true(X509_PUBKEY_get0(xp) == NULL))
        goto done;

    ret = 1;

 done:
    X509_PUBKEY_free(xp);
    return ret;
}

static int test_X509_PUBKEY_dup(void)
{
    int ret = 0;
    X509_PUBKEY *xp = NULL, *xq = NULL;
    const unsigned char *p = kExampleECPubKeyDER;
    size_t input_len = sizeof(kExampleECPubKeyDER);

    xp = X509_PUBKEY_new_ex(testctx, testpropq);
    if (!TEST_ptr(xp)
            || !TEST_ptr(d2i_X509_PUBKEY(&xp, &p, input_len))
            || !TEST_ptr(xq = X509_PUBKEY_dup(xp))
            || !TEST_ptr_ne(xp, xq))
        goto done;

    if (!TEST_ptr(X509_PUBKEY_get0(xq))
            || !TEST_ptr(X509_PUBKEY_get0(xp))
            || !TEST_ptr_ne(X509_PUBKEY_get0(xq), X509_PUBKEY_get0(xp)))
        goto done;

    X509_PUBKEY_free(xq);
    xq = NULL;
    p = kExampleBadECPubKeyDER;
    input_len = sizeof(kExampleBadECPubKeyDER);

    if (!TEST_ptr(xp = d2i_X509_PUBKEY(&xp, &p, input_len))
            || !TEST_ptr(xq = X509_PUBKEY_dup(xp)))
        goto done;

    X509_PUBKEY_free(xp);
    xp = NULL;
    if (!TEST_true(X509_PUBKEY_get0(xq) == NULL))
        goto done;

    ret = 1;

 done:
    X509_PUBKEY_free(xp);
    X509_PUBKEY_free(xq);
    return ret;
}
#endif /* OPENSSL_NO_EC */

/* Test getting and setting parameters on an EVP_PKEY_CTX */
static int test_EVP_PKEY_CTX_get_set_params(EVP_PKEY *pkey)
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    const OSSL_PARAM *params;
    OSSL_PARAM ourparams[2], *param = ourparams, *param_md;
    int ret = 0;
    const EVP_MD *md;
    char mdname[OSSL_MAX_NAME_SIZE];
    char ssl3ms[48];

    /* Initialise a sign operation */
    ctx = EVP_PKEY_CTX_new_from_pkey(testctx, pkey, testpropq);
    if (!TEST_ptr(ctx)
            || !TEST_int_gt(EVP_PKEY_sign_init(ctx), 0))
        goto err;

    /*
     * We should be able to query the parameters now.
     */
    params = EVP_PKEY_CTX_settable_params(ctx);
    if (!TEST_ptr(params)
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_DIGEST)))
        goto err;

    params = EVP_PKEY_CTX_gettable_params(ctx);
    if (!TEST_ptr(params)
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_ALGORITHM_ID))
        || !TEST_ptr(OSSL_PARAM_locate_const(params,
                                             OSSL_SIGNATURE_PARAM_DIGEST)))
        goto err;

    /*
     * Test getting and setting params via EVP_PKEY_CTX_set_params() and
     * EVP_PKEY_CTX_get_params()
     */
    strcpy(mdname, "SHA512");
    param_md = param;
    *param++ = OSSL_PARAM_construct_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST,
                                                mdname, 0);
    *param++ = OSSL_PARAM_construct_end();

    if (!TEST_true(EVP_PKEY_CTX_set_params(ctx, ourparams)))
        goto err;

    mdname[0] = '\0';
    *param_md = OSSL_PARAM_construct_utf8_string(OSSL_SIGNATURE_PARAM_DIGEST,
                                                 mdname, sizeof(mdname));
    if (!TEST_true(EVP_PKEY_CTX_get_params(ctx, ourparams))
            || !TEST_str_eq(mdname, "SHA512"))
        goto err;

    /*
     * Test the TEST_PKEY_CTX_set_signature_md() and
     * TEST_PKEY_CTX_get_signature_md() functions
     */
    if (!TEST_int_gt(EVP_PKEY_CTX_set_signature_md(ctx, EVP_sha256()), 0)
            || !TEST_int_gt(EVP_PKEY_CTX_get_signature_md(ctx, &md), 0)
            || !TEST_ptr_eq(md, EVP_sha256()))
        goto err;

    /*
     * Test getting MD parameters via an associated EVP_PKEY_CTX
     */
    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
        || !TEST_true(EVP_DigestSignInit_ex(mdctx, NULL, "SHA1", testctx, testpropq,
                                            pkey, NULL)))
        goto err;

    /*
     * We now have an EVP_MD_CTX with an EVP_PKEY_CTX inside it. We should be
     * able to obtain the digest's settable parameters from the provider.
     */
    params = EVP_MD_CTX_settable_params(mdctx);
    if (!TEST_ptr(params)
            || !TEST_int_eq(strcmp(params[0].key, OSSL_DIGEST_PARAM_SSL3_MS), 0)
               /* The final key should be NULL */
            || !TEST_ptr_null(params[1].key))
        goto err;

    param = ourparams;
    memset(ssl3ms, 0, sizeof(ssl3ms));
    *param++ = OSSL_PARAM_construct_octet_string(OSSL_DIGEST_PARAM_SSL3_MS,
                                                 ssl3ms, sizeof(ssl3ms));
    *param++ = OSSL_PARAM_construct_end();

    if (!TEST_true(EVP_MD_CTX_set_params(mdctx, ourparams)))
        goto err;

    ret = 1;

 err:
    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_CTX_free(ctx);

    return ret;
}

#ifndef OPENSSL_NO_DSA
static int test_DSA_get_set_params(void)
{
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;
    BIGNUM *p = NULL, *q = NULL, *g = NULL, *pub = NULL, *priv = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our DSA object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "DSA", NULL))
        || !TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_ptr(p = BN_new())
        || !TEST_ptr(q = BN_new())
        || !TEST_ptr(g = BN_new())
        || !TEST_ptr(pub = BN_new())
        || !TEST_ptr(priv = BN_new()))
        goto err;
    if (!TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_P, p))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_Q, q))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_FFC_G, g))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PUB_KEY,
                                             pub))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_PRIV_KEY,
                                             priv)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld)))
        goto err;

    if (!TEST_int_gt(EVP_PKEY_fromdata_init(pctx), 0)
        || !TEST_int_gt(EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_KEYPAIR,
                                          params), 0))
        goto err;

    if (!TEST_ptr(pkey))
        goto err;

    ret = test_EVP_PKEY_CTX_get_set_params(pkey);

 err:
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    BN_free(p);
    BN_free(q);
    BN_free(g);
    BN_free(pub);
    BN_free(priv);

    return ret;
}

/*
 * Test combinations of private, public, missing and private + public key
 * params to ensure they are all accepted
 */
static int test_DSA_priv_pub(void)
{
    return test_EVP_PKEY_ffc_priv_pub("DSA");
}

#endif /* !OPENSSL_NO_DSA */

static int test_RSA_get_set_params(void)
{
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;
    BIGNUM *n = NULL, *e = NULL, *d = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our RSA object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "RSA", NULL))
        || !TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_ptr(n = BN_new())
        || !TEST_ptr(e = BN_new())
        || !TEST_ptr(d = BN_new()))
        goto err;
    if (!TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld)))
        goto err;

    if (!TEST_int_gt(EVP_PKEY_fromdata_init(pctx), 0)
        || !TEST_int_gt(EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_KEYPAIR,
                                          params), 0))
        goto err;

    if (!TEST_ptr(pkey))
        goto err;

    ret = test_EVP_PKEY_CTX_get_set_params(pkey);

 err:
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    BN_free(n);
    BN_free(e);
    BN_free(d);

    return ret;
}

#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
static int test_decrypt_null_chunks(void)
{
    EVP_CIPHER_CTX* ctx = NULL;
    EVP_CIPHER *cipher = NULL;
    const unsigned char key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1
    };
    unsigned char iv[12] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
    };
    unsigned char msg[] = "It was the best of times, it was the worst of times";
    unsigned char ciphertext[80];
    unsigned char plaintext[80];
    /* We initialise tmp to a non zero value on purpose */
    int ctlen, ptlen, tmp = 99;
    int ret = 0;
    const int enc_offset = 10, dec_offset = 20;

    if (!TEST_ptr(cipher = EVP_CIPHER_fetch(testctx, "ChaCha20-Poly1305", testpropq))
            || !TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_true(EVP_EncryptInit_ex(ctx, cipher, NULL,
                                             key, iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &ctlen, msg,
                                            enc_offset))
            /* Deliberate add a zero length update */
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp,
                                            msg + enc_offset,
                                            sizeof(msg) - enc_offset))
            || !TEST_int_eq(ctlen += tmp, sizeof(msg))
            || !TEST_true(EVP_EncryptFinal(ctx, ciphertext + ctlen, &tmp))
            || !TEST_int_eq(tmp, 0))
        goto err;

    /* Deliberately initialise tmp to a non zero value */
    tmp = 99;
    if (!TEST_true(EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv))
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext, &ptlen, ciphertext,
                                            dec_offset))
            /*
             * Deliberately add a zero length update. We also deliberately do
             * this at a different offset than for encryption.
             */
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp,
                                            ciphertext + dec_offset,
                                            ctlen - dec_offset))
            || !TEST_int_eq(ptlen += tmp, sizeof(msg))
            || !TEST_true(EVP_DecryptFinal(ctx, plaintext + ptlen, &tmp))
            || !TEST_int_eq(tmp, 0)
            || !TEST_mem_eq(msg, sizeof(msg), plaintext, ptlen))
        goto err;

    ret = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(cipher);
    return ret;
}
#endif /* !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305) */

#ifndef OPENSSL_NO_DH
/*
 * Test combinations of private, public, missing and private + public key
 * params to ensure they are all accepted
 */
static int test_DH_priv_pub(void)
{
    return test_EVP_PKEY_ffc_priv_pub("DH");
}

# ifndef OPENSSL_NO_DEPRECATED_3_0
static int test_EVP_PKEY_set1_DH(void)
{
    DH *x942dh = NULL, *noqdh = NULL;
    EVP_PKEY *pkey1 = NULL, *pkey2 = NULL;
    int ret = 0;
    BIGNUM *p, *g = NULL;
    BIGNUM *pubkey = NULL;
    unsigned char pub[2048 / 8];
    size_t len = 0;

    if (!TEST_ptr(p = BN_new())
            || !TEST_ptr(g = BN_new())
            || !TEST_ptr(pubkey = BN_new())
            || !TEST_true(BN_set_word(p, 9999))
            || !TEST_true(BN_set_word(g, 2))
            || !TEST_true(BN_set_word(pubkey, 4321))
            || !TEST_ptr(noqdh = DH_new())
            || !TEST_true(DH_set0_pqg(noqdh, p, NULL, g))
            || !TEST_true(DH_set0_key(noqdh, pubkey, NULL))
            || !TEST_ptr(pubkey = BN_new())
            || !TEST_true(BN_set_word(pubkey, 4321)))
        goto err;
    p = g = NULL;

    x942dh = DH_get_2048_256();
    pkey1 = EVP_PKEY_new();
    pkey2 = EVP_PKEY_new();
    if (!TEST_ptr(x942dh)
            || !TEST_ptr(noqdh)
            || !TEST_ptr(pkey1)
            || !TEST_ptr(pkey2)
            || !TEST_true(DH_set0_key(x942dh, pubkey, NULL)))
        goto err;
    pubkey = NULL;

    if (!TEST_true(EVP_PKEY_set1_DH(pkey1, x942dh))
            || !TEST_int_eq(EVP_PKEY_get_id(pkey1), EVP_PKEY_DHX))
        goto err;

    if (!TEST_true(EVP_PKEY_get_bn_param(pkey1, OSSL_PKEY_PARAM_PUB_KEY,
                                         &pubkey))
            || !TEST_ptr(pubkey))
        goto err;

    if (!TEST_true(EVP_PKEY_set1_DH(pkey2, noqdh))
            || !TEST_int_eq(EVP_PKEY_get_id(pkey2), EVP_PKEY_DH))
        goto err;

    if (!TEST_true(EVP_PKEY_get_octet_string_param(pkey2,
                                                   OSSL_PKEY_PARAM_PUB_KEY,
                                                   pub, sizeof(pub), &len))
            || !TEST_size_t_ne(len, 0))
        goto err;

    ret = 1;
 err:
    BN_free(p);
    BN_free(g);
    BN_free(pubkey);
    EVP_PKEY_free(pkey1);
    EVP_PKEY_free(pkey2);
    DH_free(x942dh);
    DH_free(noqdh);

    return ret;
}
# endif /* !OPENSSL_NO_DEPRECATED_3_0 */
#endif /* !OPENSSL_NO_DH */

/*
 * We test what happens with an empty template.  For the sake of this test,
 * the template must be ignored, and we know that's the case for RSA keys
 * (this might arguably be a misfeature, but that's what we currently do,
 * even in provider code, since that's how the legacy RSA implementation
 * does things)
 */
static int test_keygen_with_empty_template(int n)
{
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *tkey = NULL;
    int ret = 0;

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    switch (n) {
    case 0:
        /* We do test with no template at all as well */
        if (!TEST_ptr(ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL)))
            goto err;
        break;
    case 1:
        /* Here we create an empty RSA key that serves as our template */
        if (!TEST_ptr(tkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_set_type(tkey, EVP_PKEY_RSA))
            || !TEST_ptr(ctx = EVP_PKEY_CTX_new(tkey, NULL)))
            goto err;
        break;
    }

    if (!TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        || !TEST_int_gt(EVP_PKEY_keygen(ctx, &pkey), 0))
        goto err;

    ret = 1;
 err:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(tkey);
    return ret;
}

/*
 * Test that we fail if we attempt to use an algorithm that is not available
 * in the current library context (unless we are using an algorithm that
 * should be made available via legacy codepaths).
 *
 * 0:   RSA
 * 1:   SM2
 */
static int test_pkey_ctx_fail_without_provider(int tst)
{
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_PROVIDER *tmpnullprov = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    const char *keytype = NULL;
    int expect_null = 0;
    int ret = 0;

    if (!TEST_ptr(tmpctx))
        goto err;

    tmpnullprov = OSSL_PROVIDER_load(tmpctx, "null");
    if (!TEST_ptr(tmpnullprov))
        goto err;

    /*
     * We check for certain algos in the null provider.
     * If an algo is expected to have a provider keymgmt, contructing an
     * EVP_PKEY_CTX is expected to fail (return NULL).
     * Otherwise, if it's expected to have legacy support, contructing an
     * EVP_PKEY_CTX is expected to succeed (return non-NULL).
     */
    switch (tst) {
    case 0:
        keytype = "RSA";
        expect_null = 1;
        break;
    case 1:
        keytype = "SM2";
        expect_null = 1;
#ifdef OPENSSL_NO_EC
        TEST_info("EC disable, skipping SM2 check...");
        goto end;
#endif
#ifdef OPENSSL_NO_SM2
        TEST_info("SM2 disable, skipping SM2 check...");
        goto end;
#endif
        break;
    default:
        TEST_error("No test for case %d", tst);
        goto err;
    }

    pctx = EVP_PKEY_CTX_new_from_name(tmpctx, keytype, "");
    if (expect_null ? !TEST_ptr_null(pctx) : !TEST_ptr(pctx))
        goto err;

#if defined(OPENSSL_NO_EC) || defined(OPENSSL_NO_SM2)
 end:
#endif
    ret = 1;

 err:
    EVP_PKEY_CTX_free(pctx);
    OSSL_PROVIDER_unload(tmpnullprov);
    OSSL_LIB_CTX_free(tmpctx);
    return ret;
}

static int test_rand_agglomeration(void)
{
    EVP_RAND *rand;
    EVP_RAND_CTX *ctx;
    OSSL_PARAM params[3], *p = params;
    int res;
    unsigned int step = 7;
    static unsigned char seed[] = "It does not matter how slowly you go "
                                  "as long as you do not stop.";
    unsigned char out[sizeof(seed)];

    if (!TEST_int_ne(sizeof(seed) % step, 0)
            || !TEST_ptr(rand = EVP_RAND_fetch(testctx, "TEST-RAND", testpropq)))
        return 0;
    ctx = EVP_RAND_CTX_new(rand, NULL);
    EVP_RAND_free(rand);
    if (!TEST_ptr(ctx))
        return 0;

    memset(out, 0, sizeof(out));
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY,
                                             seed, sizeof(seed));
    *p++ = OSSL_PARAM_construct_uint(OSSL_RAND_PARAM_MAX_REQUEST, &step);
    *p = OSSL_PARAM_construct_end();
    res = TEST_true(EVP_RAND_CTX_set_params(ctx, params))
          && TEST_true(EVP_RAND_generate(ctx, out, sizeof(out), 0, 1, NULL, 0))
          && TEST_mem_eq(seed, sizeof(seed), out, sizeof(out));
    EVP_RAND_CTX_free(ctx);
    return res;
}

/*
 * Test that we correctly return the original or "running" IV after
 * an encryption operation.
 * Run multiple times for some different relevant algorithms/modes.
 */
static int test_evp_iv_aes(int idx)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char key[16] = {0x4c, 0x43, 0xdb, 0xdd, 0x42, 0x73, 0x47, 0xd1,
                             0xe5, 0x62, 0x7d, 0xcd, 0x4d, 0x76, 0x4d, 0x57};
    unsigned char init_iv[EVP_MAX_IV_LENGTH] =
        {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98, 0x82,
         0x5a, 0x55, 0x91, 0x81, 0x42, 0xa8, 0x89, 0x34};
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                         9, 10, 11, 12, 13, 14, 15, 16 };
    unsigned char ciphertext[32], oiv[16], iv[16];
    unsigned char *ref_iv;
    unsigned char cbc_state[16] = {0x10, 0x2f, 0x05, 0xcc, 0xc2, 0x55, 0x72, 0xb9,
                                   0x88, 0xe6, 0x4a, 0x17, 0x10, 0x74, 0x22, 0x5e};

    unsigned char ofb_state[16] = {0x76, 0xe6, 0x66, 0x61, 0xd0, 0x8a, 0xe4, 0x64,
                                   0xdd, 0x66, 0xbf, 0x00, 0xf0, 0xe3, 0x6f, 0xfd};
    unsigned char cfb_state[16] = {0x77, 0xe4, 0x65, 0x65, 0xd5, 0x8c, 0xe3, 0x6c,
                                   0xd4, 0x6c, 0xb4, 0x0c, 0xfd, 0xed, 0x60, 0xed};
    unsigned char gcm_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
    unsigned char ccm_state[7] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98};
#ifndef OPENSSL_NO_OCB
    unsigned char ocb_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
#endif
    int len = sizeof(ciphertext);
    size_t ivlen, ref_len;
    const EVP_CIPHER *type = NULL;
    int iv_reset = 0;

    if (nullprov != NULL && idx < 6)
        return TEST_skip("Test does not support a non-default library context");

    switch(idx) {
    case 0:
        type = EVP_aes_128_cbc();
        /* FALLTHROUGH */
    case 6:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-cbc", testpropq);
        ref_iv = cbc_state;
        ref_len = sizeof(cbc_state);
        iv_reset = 1;
        break;
    case 1:
        type = EVP_aes_128_ofb();
        /* FALLTHROUGH */
    case 7:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ofb", testpropq);
        ref_iv = ofb_state;
        ref_len = sizeof(ofb_state);
        iv_reset = 1;
        break;
    case 2:
        type = EVP_aes_128_cfb();
        /* FALLTHROUGH */
    case 8:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-cfb", testpropq);
        ref_iv = cfb_state;
        ref_len = sizeof(cfb_state);
        iv_reset = 1;
        break;
    case 3:
        type = EVP_aes_128_gcm();
        /* FALLTHROUGH */
    case 9:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-gcm", testpropq);
        ref_iv = gcm_state;
        ref_len = sizeof(gcm_state);
        break;
    case 4:
        type = EVP_aes_128_ccm();
        /* FALLTHROUGH */
    case 10:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ccm", testpropq);
        ref_iv = ccm_state;
        ref_len = sizeof(ccm_state);
        break;
#ifdef OPENSSL_NO_OCB
    case 5:
    case 11:
        return 1;
#else
    case 5:
        type = EVP_aes_128_ocb();
        /* FALLTHROUGH */
    case 11:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ocb", testpropq);
        ref_iv = ocb_state;
        ref_len = sizeof(ocb_state);
        break;
#endif
    default:
        return 0;
    }

    if (!TEST_ptr(type)
            || !TEST_ptr((ctx = EVP_CIPHER_CTX_new()))
            || !TEST_true(EVP_EncryptInit_ex(ctx, type, NULL, key, init_iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &len, msg,
                          (int)sizeof(msg)))
            || !TEST_true(EVP_CIPHER_CTX_get_original_iv(ctx, oiv, sizeof(oiv)))
            || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx, ciphertext, &len)))
        goto err;
    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
    if (!TEST_mem_eq(init_iv, ivlen, oiv, ivlen)
            || !TEST_mem_eq(ref_iv, ref_len, iv, ivlen))
        goto err;

    /* CBC, OFB, and CFB modes: the updated iv must be reset after reinit */
    if (!TEST_true(EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL))
        || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv))))
        goto err;
    if (iv_reset) {
        if (!TEST_mem_eq(init_iv, ivlen, iv, ivlen))
            goto err;
    } else {
        if (!TEST_mem_eq(ref_iv, ivlen, iv, ivlen))
            goto err;
    }

    ret = 1;
err:
    EVP_CIPHER_CTX_free(ctx);
    if (idx >= 6)
        EVP_CIPHER_free((EVP_CIPHER *)type);
    return ret;
}

#ifndef OPENSSL_NO_DES
static int test_evp_iv_des(int idx)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    static const unsigned char key[24] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xf1, 0xe0, 0xd3, 0xc2, 0xb5, 0xa4, 0x97, 0x86,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    static const unsigned char init_iv[8] = {
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                         9, 10, 11, 12, 13, 14, 15, 16 };
    unsigned char ciphertext[32], oiv[8], iv[8];
    unsigned const char *ref_iv;
    static const unsigned char cbc_state_des[8] = {
        0x4f, 0xa3, 0x85, 0xcd, 0x8b, 0xf3, 0x06, 0x2a
    };
    static const unsigned char cbc_state_3des[8] = {
        0x35, 0x27, 0x7d, 0x65, 0x6c, 0xfb, 0x50, 0xd9
    };
    static const unsigned char ofb_state_des[8] = {
        0xa7, 0x0d, 0x1d, 0x45, 0xf9, 0x96, 0x3f, 0x2c
    };
    static const unsigned char ofb_state_3des[8] = {
        0xab, 0x16, 0x24, 0xbb, 0x5b, 0xac, 0xed, 0x5e
    };
    static const unsigned char cfb_state_des[8] = {
        0x91, 0xeb, 0x6d, 0x29, 0x4b, 0x08, 0xbd, 0x73
    };
    static const unsigned char cfb_state_3des[8] = {
        0x34, 0xdd, 0xfb, 0x47, 0x33, 0x1c, 0x61, 0xf7
    };
    int len = sizeof(ciphertext);
    size_t ivlen, ref_len;
    EVP_CIPHER *type = NULL;

    if (lgcyprov == NULL && idx < 3)
        return TEST_skip("Test requires legacy provider to be loaded");

    switch(idx) {
    case 0:
        type = EVP_CIPHER_fetch(testctx, "des-cbc", testpropq);
        ref_iv = cbc_state_des;
        ref_len = sizeof(cbc_state_des);
        break;
    case 1:
        type = EVP_CIPHER_fetch(testctx, "des-ofb", testpropq);
        ref_iv = ofb_state_des;
        ref_len = sizeof(ofb_state_des);
        break;
    case 2:
        type = EVP_CIPHER_fetch(testctx, "des-cfb", testpropq);
        ref_iv = cfb_state_des;
        ref_len = sizeof(cfb_state_des);
        break;
    case 3:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-cbc", testpropq);
        ref_iv = cbc_state_3des;
        ref_len = sizeof(cbc_state_3des);
        break;
    case 4:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-ofb", testpropq);
        ref_iv = ofb_state_3des;
        ref_len = sizeof(ofb_state_3des);
        break;
    case 5:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-cfb", testpropq);
        ref_iv = cfb_state_3des;
        ref_len = sizeof(cfb_state_3des);
        break;
    default:
        return 0;
    }

    if (!TEST_ptr(type)
            || !TEST_ptr((ctx = EVP_CIPHER_CTX_new()))
            || !TEST_true(EVP_EncryptInit_ex(ctx, type, NULL, key, init_iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &len, msg,
                          (int)sizeof(msg)))
            || !TEST_true(EVP_CIPHER_CTX_get_original_iv(ctx, oiv, sizeof(oiv)))
            || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx, ciphertext, &len)))
        goto err;
    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
    if (!TEST_mem_eq(init_iv, ivlen, oiv, ivlen)
            || !TEST_mem_eq(ref_iv, ref_len, iv, ivlen))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL))
        || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv))))
        goto err;
    if (!TEST_mem_eq(init_iv, ivlen, iv, ivlen))
        goto err;

    ret = 1;
err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return ret;
}
#endif

#ifndef OPENSSL_NO_BF
static int test_evp_bf_default_keylen(int idx)
{
    int ret = 0;
    static const char *algos[4] = {
        "bf-ecb", "bf-cbc", "bf-cfb", "bf-ofb"
    };
    int ivlen[4] = { 0, 8, 8, 8 };
    EVP_CIPHER *cipher = NULL;

    if (lgcyprov == NULL)
        return TEST_skip("Test requires legacy provider to be loaded");

    if (!TEST_ptr(cipher = EVP_CIPHER_fetch(testctx, algos[idx], testpropq))
            || !TEST_int_eq(EVP_CIPHER_get_key_length(cipher), 16)
            || !TEST_int_eq(EVP_CIPHER_get_iv_length(cipher), ivlen[idx]))
        goto err;

    ret = 1;
err:
    EVP_CIPHER_free(cipher);
    return ret;
}
#endif

#ifndef OPENSSL_NO_EC
static int ecpub_nids[] = {
    NID_brainpoolP256r1, NID_X9_62_prime256v1,
    NID_secp384r1, NID_secp521r1,
# ifndef OPENSSL_NO_EC2M
    NID_sect233k1, NID_sect233r1, NID_sect283r1,
    NID_sect409k1, NID_sect409r1, NID_sect571k1, NID_sect571r1,
# endif
    NID_brainpoolP384r1, NID_brainpoolP512r1
};

static int test_ecpub(int idx)
{
    int ret = 0, len, savelen;
    int nid;
    unsigned char buf[1024];
    unsigned char *p;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
# ifndef OPENSSL_NO_DEPRECATED_3_0
    const unsigned char *q;
    EVP_PKEY *pkey2 = NULL;
    EC_KEY *ec = NULL;
# endif

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    nid = ecpub_nids[idx];

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (!TEST_ptr(ctx)
        || !TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        || !TEST_int_gt(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid), 0)
        || !TEST_true(EVP_PKEY_keygen(ctx, &pkey)))
        goto done;
    len = i2d_PublicKey(pkey, NULL);
    savelen = len;
    if (!TEST_int_ge(len, 1)
        || !TEST_int_lt(len, 1024))
        goto done;
    p = buf;
    len = i2d_PublicKey(pkey, &p);
    if (!TEST_int_ge(len, 1)
            || !TEST_int_eq(len, savelen))
        goto done;

# ifndef OPENSSL_NO_DEPRECATED_3_0
    /* Now try to decode the just-created DER. */
    q = buf;
    if (!TEST_ptr((pkey2 = EVP_PKEY_new()))
            || !TEST_ptr((ec = EC_KEY_new_by_curve_name(nid)))
            || !TEST_true(EVP_PKEY_assign_EC_KEY(pkey2, ec)))
        goto done;
    /* EC_KEY ownership transferred */
    ec = NULL;
    if (!TEST_ptr(d2i_PublicKey(EVP_PKEY_EC, &pkey2, &q, savelen)))
        goto done;
    /* The keys should match. */
    if (!TEST_int_eq(EVP_PKEY_eq(pkey, pkey2), 1))
        goto done;
# endif

    ret = 1;

 done:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_free(pkey2);
    EC_KEY_free(ec);
# endif
    return ret;
}
#endif

static int test_EVP_rsa_pss_with_keygen_bits(void)
{
    int ret = 0;
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD *md;

    md = EVP_MD_fetch(testctx, "sha256", testpropq);
    ret = TEST_ptr(md)
        && TEST_ptr((ctx = EVP_PKEY_CTX_new_from_name(testctx, "RSA-PSS", testpropq)))
        && TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 512), 0)
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_pss_keygen_md(ctx, md), 0)
        && TEST_true(EVP_PKEY_keygen(ctx, &pkey));

    EVP_MD_free(md);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int test_EVP_rsa_pss_set_saltlen(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pkey_ctx = NULL;
    EVP_MD *sha256 = NULL;
    EVP_MD_CTX *sha256_ctx = NULL;
    int saltlen = 9999; /* buggy EVP_PKEY_CTX_get_rsa_pss_saltlen() didn't update this */
    const int test_value = 32;

    ret = TEST_ptr(pkey = load_example_rsa_key())
        && TEST_ptr(sha256 = EVP_MD_fetch(testctx, "sha256", NULL))
        && TEST_ptr(sha256_ctx = EVP_MD_CTX_new())
        && TEST_true(EVP_DigestSignInit(sha256_ctx, &pkey_ctx, sha256, NULL, pkey))
        && TEST_true(EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING))
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, test_value), 0)
        && TEST_int_gt(EVP_PKEY_CTX_get_rsa_pss_saltlen(pkey_ctx, &saltlen), 0)
        && TEST_int_eq(saltlen, test_value);

    EVP_MD_CTX_free(sha256_ctx);
    EVP_PKEY_free(pkey);
    EVP_MD_free(sha256);

    return ret;
}

static int success = 1;
static void md_names(const char *name, void *vctx)
{
    OSSL_LIB_CTX *ctx = (OSSL_LIB_CTX *)vctx;
    /* Force a namemap update */
    EVP_CIPHER *aes128 = EVP_CIPHER_fetch(ctx, "AES-128-CBC", NULL);

    if (!TEST_ptr(aes128))
        success = 0;

    EVP_CIPHER_free(aes128);
}

/*
 * Test that changing the namemap in a user callback works in a names_do_all
 * function.
 */
static int test_names_do_all(void)
{
    /* We use a custom libctx so that we know the state of the namemap */
    OSSL_LIB_CTX *ctx = OSSL_LIB_CTX_new();
    EVP_MD *sha256 = NULL;
    int testresult = 0;

    if (!TEST_ptr(ctx))
        goto err;

    sha256 = EVP_MD_fetch(ctx, "SHA2-256", NULL);
    if (!TEST_ptr(sha256))
        goto err;

    /*
     * We loop through all the names for a given digest. This should still work
     * even if the namemap changes part way through.
     */
    if (!TEST_true(EVP_MD_names_do_all(sha256, md_names, ctx)))
        goto err;

    if (!TEST_true(success))
        goto err;

    testresult = 1;
 err:
    EVP_MD_free(sha256);
    OSSL_LIB_CTX_free(ctx);
    return testresult;
}

typedef struct {
    const char *cipher;
    const unsigned char *key;
    const unsigned char *iv;
    const unsigned char *input;
    const unsigned char *expected;
    const unsigned char *tag;
    size_t ivlen; /* 0 if we do not need to set a specific IV len */
    size_t inlen;
    size_t expectedlen;
    size_t taglen;
    int keyfirst;
    int initenc;
    int finalenc;
} EVP_INIT_TEST_st;

static const EVP_INIT_TEST_st evp_init_tests[] = {
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbPlaintext,
        cfbCiphertext, NULL, 0, sizeof(cfbPlaintext), sizeof(cfbCiphertext),
        0, 1, 0, 1
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultPlaintext,
        gcmDefaultCiphertext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultPlaintext), sizeof(gcmDefaultCiphertext),
        sizeof(gcmDefaultTag), 1, 0, 1
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbPlaintext,
        cfbCiphertext, NULL, 0, sizeof(cfbPlaintext), sizeof(cfbCiphertext),
        0, 0, 0, 1
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultPlaintext,
        gcmDefaultCiphertext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultPlaintext), sizeof(gcmDefaultCiphertext),
        sizeof(gcmDefaultTag), 0, 0, 1
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbCiphertext,
        cfbPlaintext, NULL, 0, sizeof(cfbCiphertext), sizeof(cfbPlaintext),
        0, 1, 1, 0
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultCiphertext,
        gcmDefaultPlaintext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultCiphertext), sizeof(gcmDefaultPlaintext),
        sizeof(gcmDefaultTag), 1, 1, 0
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbCiphertext,
        cfbPlaintext, NULL, 0, sizeof(cfbCiphertext), sizeof(cfbPlaintext),
        0, 0, 1, 0
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultCiphertext,
        gcmDefaultPlaintext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultCiphertext), sizeof(gcmDefaultPlaintext),
        sizeof(gcmDefaultTag), 0, 1, 0
    }
};

static int evp_init_seq_set_iv(EVP_CIPHER_CTX *ctx, const EVP_INIT_TEST_st *t)
{
    int res = 0;
    
    if (t->ivlen != 0) {
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen, NULL), 0))
            goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, t->iv, -1)))
        goto err;
    res = 1;
 err:
    return res;
}

/*
 * Test step-wise cipher initialization via EVP_CipherInit_ex where the
 * arguments are given one at a time and a final adjustment to the enc
 * parameter sets the correct operation.
 */
static int test_evp_init_seq(int idx)
{
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    unsigned char tag[16];
    const EVP_INIT_TEST_st *t = &evp_init_tests[idx];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    size_t taglen = sizeof(tag);
    char *errmsg = NULL;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, t->cipher, testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, t->initenc))) {
        errmsg = "EMPTY_ENC_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (t->keyfirst && !TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, t->key, NULL, -1))) {
        errmsg = "KEY_INIT (before iv)";
        goto err;
    }
    if (!evp_init_seq_set_iv(ctx, t)) {
        errmsg = "IV_INIT";
        goto err;
    }
    if (t->keyfirst == 0 &&  !TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, t->key, NULL, -1))) {
        errmsg = "KEY_INIT (after iv)";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, NULL, t->finalenc))) {
        errmsg = "FINAL_ENC_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (t->finalenc == 0 && t->tag != NULL) {
        /* Set expected tag */
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG,
                                           t->taglen, (void *)t->tag), 0)) {
            errmsg = "SET_TAG";
            goto err;
        }
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT";
        goto err;
    }
    if (t->finalenc != 0 && t->tag != NULL) {
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
            errmsg = "GET_TAG";
            goto err;
        }
        if (!TEST_mem_eq(t->tag, t->taglen, tag, taglen)) {
            errmsg = "TAG_ERROR";
            goto err;
        }
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("evp_init_test %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

typedef struct {
    const unsigned char *input;
    const unsigned char *expected;
    size_t inlen;
    size_t expectedlen;
    int enc;
} EVP_RESET_TEST_st;

static const EVP_RESET_TEST_st evp_reset_tests[] = {
    {
        cfbPlaintext, cfbCiphertext,
        sizeof(cfbPlaintext), sizeof(cfbCiphertext), 1
    },
    {
        cfbCiphertext, cfbPlaintext,
        sizeof(cfbCiphertext), sizeof(cfbPlaintext), 0
    }
};

/*
 * Test a reset of a cipher via EVP_CipherInit_ex after the cipher has already
 * been used.
 */
static int test_evp_reset(int idx)
{
    const EVP_RESET_TEST_st *t = &evp_reset_tests[idx];
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, "aes-128-cfb", testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, kCFBDefaultKey, iCFBIV, t->enc))) {
        errmsg = "CIPHER_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, NULL, -1))) {
        errmsg = "CIPHER_REINIT";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE (reinit)";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL (reinit)";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT (reinit)";
        goto err;
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("test_evp_reset %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;    
}

typedef struct {
    const char *cipher;
    int enc;
} EVP_UPDATED_IV_TEST_st;

static const EVP_UPDATED_IV_TEST_st evp_updated_iv_tests[] = {
    {
        "aes-128-cfb", 1
    },
    {
        "aes-128-cfb", 0
    },
    {
        "aes-128-cfb1", 1
    },
    {
        "aes-128-cfb1", 0
    },
    {
        "aes-128-cfb8", 1
    },
    {
        "aes-128-cfb8", 0
    },
    {
        "aes-128-ofb", 1
    },
    {
        "aes-128-ofb", 0
    },
    {
        "aes-128-ctr", 1
    },
    {
        "aes-128-ctr", 0
    },
    {
        "aes-128-cbc", 1
    },
    {
        "aes-128-cbc", 0
    }
};

/*
 * Test that the IV in the context is updated during a crypto operation for CFB
 * and OFB.
 */
static int test_evp_updated_iv(int idx)
{
    const EVP_UPDATED_IV_TEST_st *t = &evp_updated_iv_tests[idx];
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    unsigned char updated_iv[EVP_MAX_IV_LENGTH];
    int iv_len;
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if ((type = EVP_CIPHER_fetch(testctx, t->cipher, testpropq)) == NULL) {
        TEST_info("cipher %s not supported, skipping", t->cipher);
        goto ok;
    }

    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, kCFBDefaultKey, iCFBIV, t->enc))) {
        errmsg = "CIPHER_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, cfbPlaintext, sizeof(cfbPlaintext)))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, updated_iv, sizeof(updated_iv)))) {
        errmsg = "CIPHER_CTX_GET_UPDATED_IV";
        goto err;
    }
    if (!TEST_true(iv_len = EVP_CIPHER_CTX_get_iv_length(ctx))) {
        errmsg = "CIPHER_CTX_GET_IV_LEN";
        goto err;
    }
    if (!TEST_mem_ne(iCFBIV, sizeof(iCFBIV), updated_iv, iv_len)) {
        errmsg = "IV_NOT_UPDATED";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
 ok:
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("test_evp_updated_iv %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

typedef struct {
    const unsigned char *iv1;
    const unsigned char *iv2;
    const unsigned char *expected1;
    const unsigned char *expected2;
    const unsigned char *tag1;
    const unsigned char *tag2;
    size_t ivlen1;
    size_t ivlen2;
    size_t expectedlen1;
    size_t expectedlen2;
} TEST_GCM_IV_REINIT_st;

static const TEST_GCM_IV_REINIT_st gcm_reinit_tests[] = {
    {
        iGCMResetIV1, iGCMResetIV2, gcmResetCiphertext1, gcmResetCiphertext2,
        gcmResetTag1, gcmResetTag2, sizeof(iGCMResetIV1), sizeof(iGCMResetIV2),
        sizeof(gcmResetCiphertext1), sizeof(gcmResetCiphertext2)
    },
    {
        iGCMResetIV2, iGCMResetIV1, gcmResetCiphertext2, gcmResetCiphertext1,
        gcmResetTag2, gcmResetTag1, sizeof(iGCMResetIV2), sizeof(iGCMResetIV1),
        sizeof(gcmResetCiphertext2), sizeof(gcmResetCiphertext1)
    }
};

static int test_gcm_reinit(int idx)
{
    int outlen1, outlen2, outlen3;
    int testresult = 0;
    unsigned char outbuf[1024];
    unsigned char tag[16];
    const TEST_GCM_IV_REINIT_st *t = &gcm_reinit_tests[idx];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    size_t taglen = sizeof(tag);
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, "aes-256-gcm", testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, 1))) {
        errmsg = "ENC_INIT";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen1, NULL), 0)) {
        errmsg = "SET_IVLEN1";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, kGCMResetKey, t->iv1, 1))) {
        errmsg = "SET_IV1";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, NULL, &outlen3, gcmAAD, sizeof(gcmAAD)))) {
        errmsg = "AAD1";
        goto err;
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, gcmResetPlaintext,
                                    sizeof(gcmResetPlaintext)))) {
        errmsg = "CIPHER_UPDATE1";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL1";
        goto err;
    }
    if (!TEST_mem_eq(t->expected1, t->expectedlen1, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT1";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
        errmsg = "GET_TAG1";
        goto err;
    }
    if (!TEST_mem_eq(t->tag1, taglen, tag, taglen)) {
        errmsg = "TAG_ERROR1";
        goto err;
    }
    /* Now reinit */
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen2, NULL), 0)) {
        errmsg = "SET_IVLEN2";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, t->iv2, -1))) {
        errmsg = "SET_IV2";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, NULL, &outlen3, gcmAAD, sizeof(gcmAAD)))) {
        errmsg = "AAD2";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, gcmResetPlaintext,
                                    sizeof(gcmResetPlaintext)))) {
        errmsg = "CIPHER_UPDATE2";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL2";
        goto err;
    }
    if (!TEST_mem_eq(t->expected2, t->expectedlen2, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT2";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
        errmsg = "GET_TAG2";
        goto err;
    }
    if (!TEST_mem_eq(t->tag2, taglen, tag, taglen)) {
        errmsg = "TAG_ERROR2";
        goto err;
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("evp_init_test %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

#ifndef OPENSSL_NO_DEPRECATED_3_0
static EVP_PKEY_METHOD *custom_pmeth =  NULL;
static const EVP_PKEY_METHOD *orig_pmeth = NULL;

# define EVP_PKEY_CTRL_MY_COMMAND 9999

static int custom_pmeth_init(EVP_PKEY_CTX *ctx)
{
    int (*pinit)(EVP_PKEY_CTX *ctx);

    EVP_PKEY_meth_get_init(orig_pmeth, &pinit);
    return pinit(ctx);
}

static void custom_pmeth_cleanup(EVP_PKEY_CTX *ctx)
{
    void (*pcleanup)(EVP_PKEY_CTX *ctx);

    EVP_PKEY_meth_get_cleanup(orig_pmeth, &pcleanup);
    pcleanup(ctx);
}

static int custom_pmeth_sign(EVP_PKEY_CTX *ctx, unsigned char *out,
                             size_t *outlen, const unsigned char *in,
                             size_t inlen)
{
    int (*psign)(EVP_PKEY_CTX *ctx, unsigned char *sig, size_t *siglen,
                 const unsigned char *tbs, size_t tbslen);

    EVP_PKEY_meth_get_sign(orig_pmeth, NULL, &psign);
    return psign(ctx, out, outlen, in, inlen);
}

static int custom_pmeth_digestsign(EVP_MD_CTX *ctx, unsigned char *sig,
                                   size_t *siglen, const unsigned char *tbs,
                                   size_t tbslen)
{
    int (*pdigestsign)(EVP_MD_CTX *ctx, unsigned char *sig, size_t *siglen,
                       const unsigned char *tbs, size_t tbslen);

    EVP_PKEY_meth_get_digestsign(orig_pmeth, &pdigestsign);
    return pdigestsign(ctx, sig, siglen, tbs, tbslen);
}

static int custom_pmeth_derive(EVP_PKEY_CTX *ctx, unsigned char *key,
                               size_t *keylen)
{
    int (*pderive)(EVP_PKEY_CTX *ctx, unsigned char *key, size_t *keylen);

    EVP_PKEY_meth_get_derive(orig_pmeth, NULL, &pderive);
    return pderive(ctx, key, keylen);
}

static int custom_pmeth_copy(EVP_PKEY_CTX *dst, const EVP_PKEY_CTX *src)
{
    int (*pcopy)(EVP_PKEY_CTX *dst, const EVP_PKEY_CTX *src);

    EVP_PKEY_meth_get_copy(orig_pmeth, &pcopy);
    return pcopy(dst, src);
}

static int ctrl_called;

static int custom_pmeth_ctrl(EVP_PKEY_CTX *ctx, int type, int p1, void *p2)
{
    int (*pctrl)(EVP_PKEY_CTX *ctx, int type, int p1, void *p2);

    EVP_PKEY_meth_get_ctrl(orig_pmeth, &pctrl, NULL);

    if (type == EVP_PKEY_CTRL_MY_COMMAND) {
        ctrl_called = 1;
        return 1;
    }

    return pctrl(ctx, type, p1, p2);
}

static int test_custom_pmeth(int idx)
{
    EVP_PKEY_CTX *pctx = NULL;
    EVP_MD_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    int id, orig_id, orig_flags;
    int testresult = 0;
    size_t reslen;
    unsigned char *res = NULL;
    unsigned char msg[] = { 'H', 'e', 'l', 'l', 'o' };
    const EVP_MD *md = EVP_sha256();
    int doderive = 0;

    ctrl_called = 0;

    /* We call deprecated APIs so this test doesn't support a custom libctx */
    if (testctx != NULL)
        return 1;

    switch(idx) {
    case 0:
    case 6:
        id = EVP_PKEY_RSA;
        pkey = load_example_rsa_key();
        break;
    case 1:
    case 7:
# ifndef OPENSSL_NO_DSA
        id = EVP_PKEY_DSA;
        pkey = load_example_dsa_key();
        break;
# else
        return 1;
# endif
    case 2:
    case 8:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_EC;
        pkey = load_example_ec_key();
        break;
# else
        return 1;
# endif
    case 3:
    case 9:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_ED25519;
        md = NULL;
        pkey = load_example_ed25519_key();
        break;
# else
        return 1;
# endif
    case 4:
    case 10:
# ifndef OPENSSL_NO_DH
        id = EVP_PKEY_DH;
        doderive = 1;
        pkey = load_example_dh_key();
        break;
# else
        return 1;
# endif
    case 5:
    case 11:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_X25519;
        doderive = 1;
        pkey = load_example_x25519_key();
        break;
# else
        return 1;
# endif
    default:
        TEST_error("Should not happen");
        goto err;
    }

    if (!TEST_ptr(pkey))
        goto err;

    if (idx < 6) {
        if (!TEST_true(evp_pkey_is_provided(pkey)))
            goto err;
    } else {
        EVP_PKEY *tmp = pkey;

        /* Convert to a legacy key */
        pkey = EVP_PKEY_new();
        if (!TEST_ptr(pkey)) {
            pkey = tmp;
            goto err;
        }
        if (!TEST_true(evp_pkey_copy_downgraded(&pkey, tmp))) {
            EVP_PKEY_free(tmp);
            goto err;
        }
        EVP_PKEY_free(tmp);
        if (!TEST_true(evp_pkey_is_legacy(pkey)))
            goto err;
    }

    if (!TEST_ptr(orig_pmeth = EVP_PKEY_meth_find(id))
            || !TEST_ptr(pkey))
        goto err;

    EVP_PKEY_meth_get0_info(&orig_id, &orig_flags, orig_pmeth);
    if (!TEST_int_eq(orig_id, id)
            || !TEST_ptr(custom_pmeth = EVP_PKEY_meth_new(id, orig_flags)))
        goto err;

    if (id == EVP_PKEY_ED25519) {
        EVP_PKEY_meth_set_digestsign(custom_pmeth, custom_pmeth_digestsign);
    } if (id == EVP_PKEY_DH || id == EVP_PKEY_X25519) {
        EVP_PKEY_meth_set_derive(custom_pmeth, NULL, custom_pmeth_derive);
    } else {
        EVP_PKEY_meth_set_sign(custom_pmeth, NULL, custom_pmeth_sign);
    }
    if (id != EVP_PKEY_ED25519 && id != EVP_PKEY_X25519) {
        EVP_PKEY_meth_set_init(custom_pmeth, custom_pmeth_init);
        EVP_PKEY_meth_set_cleanup(custom_pmeth, custom_pmeth_cleanup);
        EVP_PKEY_meth_set_copy(custom_pmeth, custom_pmeth_copy);
    }
    EVP_PKEY_meth_set_ctrl(custom_pmeth, custom_pmeth_ctrl, NULL);
    if (!TEST_true(EVP_PKEY_meth_add0(custom_pmeth)))
        goto err;

    if (doderive) {
        pctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_derive_init(pctx), 1)
                || !TEST_int_ge(EVP_PKEY_CTX_ctrl(pctx, -1, -1,
                                                EVP_PKEY_CTRL_MY_COMMAND, 0, NULL),
                                1)
                || !TEST_int_eq(ctrl_called, 1)
                || !TEST_int_ge(EVP_PKEY_derive_set_peer(pctx, pkey), 1)
                || !TEST_int_ge(EVP_PKEY_derive(pctx, NULL, &reslen), 1)
                || !TEST_ptr(res = OPENSSL_malloc(reslen))
                || !TEST_int_ge(EVP_PKEY_derive(pctx, res, &reslen), 1))
            goto err;
    } else {
        ctx = EVP_MD_CTX_new();
        reslen = EVP_PKEY_size(pkey);
        res = OPENSSL_malloc(reslen);
        if (!TEST_ptr(ctx)
                || !TEST_ptr(res)
                || !TEST_true(EVP_DigestSignInit(ctx, &pctx, md, NULL, pkey))
                || !TEST_int_ge(EVP_PKEY_CTX_ctrl(pctx, -1, -1,
                                                EVP_PKEY_CTRL_MY_COMMAND, 0, NULL),
                                1)
                || !TEST_int_eq(ctrl_called, 1))
            goto err;

        if (id == EVP_PKEY_ED25519) {
            if (!TEST_true(EVP_DigestSign(ctx, res, &reslen, msg, sizeof(msg))))
                goto err;
        } else {
            if (!TEST_true(EVP_DigestUpdate(ctx, msg, sizeof(msg)))
                    || !TEST_true(EVP_DigestSignFinal(ctx, res, &reslen)))
                goto err;
        }
    }

    testresult = 1;
 err:
    OPENSSL_free(res);
    EVP_MD_CTX_free(ctx);
    if (doderive)
        EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_meth_remove(custom_pmeth);
    EVP_PKEY_meth_free(custom_pmeth);
    custom_pmeth = NULL;
    return testresult;
}

static int test_evp_md_cipher_meth(void)
{
    EVP_MD *md = EVP_MD_meth_dup(EVP_sha256());
    EVP_CIPHER *ciph = EVP_CIPHER_meth_dup(EVP_aes_128_cbc());
    int testresult = 0;

    if (!TEST_ptr(md) || !TEST_ptr(ciph))
        goto err;

    testresult = 1;

 err:
    EVP_MD_meth_free(md);
    EVP_CIPHER_meth_free(ciph);

    return testresult;
}

typedef struct {
        int data;
} custom_dgst_ctx;

static int custom_md_init_called = 0;
static int custom_md_cleanup_called = 0;

static int custom_md_init(EVP_MD_CTX *ctx)
{
    custom_dgst_ctx *p = EVP_MD_CTX_md_data(ctx);

    if (p == NULL)
        return 0;

    custom_md_init_called++;
    return 1;
}

static int custom_md_cleanup(EVP_MD_CTX *ctx)
{
    custom_dgst_ctx *p = EVP_MD_CTX_md_data(ctx);

    if (p == NULL)
        /* Nothing to do */
        return 1;

    custom_md_cleanup_called++;
    return 1;
}

static int test_custom_md_meth(void)
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_MD *tmp = NULL;
    char mess[] = "Test Message\n";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_MD_meth_new(nid, NID_undef);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_MD_meth_set_init(tmp, custom_md_init))
            || !TEST_true(EVP_MD_meth_set_cleanup(tmp, custom_md_cleanup))
            || !TEST_true(EVP_MD_meth_set_app_datasize(tmp,
                                                       sizeof(custom_dgst_ctx))))
        goto err;

    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
            || !TEST_true(EVP_DigestFinal_ex(mdctx, md_value, &md_len))
            || !TEST_int_eq(custom_md_init_called, 1)
            || !TEST_int_eq(custom_md_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_MD_CTX_free(mdctx);
    EVP_MD_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    EVP_PKEY *pkey = NULL;
    const unsigned char badcmackey[] = { 0x00, 0x01 };
    const unsigned char cmackey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char ed25519key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_MD_CTX *ctx = NULL;
    unsigned char *mac = NULL;
    size_t maclen = 0;
    int ret;

#  ifdef OPENSSL_NO_CMAC
    /* Skip CMAC tests in a no-cmac build */
    if (tst <= 1)
        return 1;
#  endif

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    switch (tst) {
    case 0:
        pkey = EVP_PKEY_new_CMAC_key(e, cmackey, sizeof(cmackey),
                                     EVP_aes_128_cbc());
        break;
    case 1:
        pkey = EVP_PKEY_new_CMAC_key(e, badcmackey, sizeof(badcmackey),
                                     EVP_aes_128_cbc());
        break;
    case 2:
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, e, ed25519key,
                                            sizeof(ed25519key));
        break;
    default:
        TEST_error("Invalid test case");
        goto err;
    }
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_ptr(ctx = EVP_MD_CTX_new()))
        goto err;

    ret = EVP_DigestSignInit(ctx, NULL, tst == 2 ? NULL : EVP_sha256(), NULL,
                             pkey);
    if (tst == 0) {
        if (!TEST_true(ret))
            goto err;

        if (!TEST_true(EVP_DigestSignUpdate(ctx, msg, sizeof(msg)))
                || !TEST_true(EVP_DigestSignFinal(ctx, NULL, &maclen)))
            goto err;

        if (!TEST_ptr(mac = OPENSSL_malloc(maclen)))
            goto err;

        if (!TEST_true(EVP_DigestSignFinal(ctx, mac, &maclen)))
            goto err;
    } else {
        /* We used a bad key. We expect a failure here */
        if (!TEST_false(ret))
            goto err;
    }

    testresult = 1;
 err:
    EVP_MD_CTX_free(ctx);
    OPENSSL_free(mac);
    EVP_PKEY_free(pkey);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}

static int test_cipher_with_engine(void)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    const unsigned char keyiv[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_CIPHER_CTX *ctx = NULL, *ctx2 = NULL;
    unsigned char buf[AES_BLOCK_SIZE];
    int len = 0;

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_ptr(ctx2 = EVP_CIPHER_CTX_new()))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), e, keyiv, keyiv)))
        goto err;

    /* Copy the ctx, and complete the operation with the new ctx */
    if (!TEST_true(EVP_CIPHER_CTX_copy(ctx2, ctx)))
        goto err;

    if (!TEST_true(EVP_EncryptUpdate(ctx2, buf, &len, msg, sizeof(msg)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx2, buf + len, &len)))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_CTX_free(ctx2);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}
# endif /* OPENSSL_NO_DYNAMIC_ENGINE */
#endif /* OPENSSL_NO_DEPRECATED_3_0 */

static int ecxnids[] = {
    NID_X25519,
    NID_X448,
    NID_ED25519,
    NID_ED448
};

/* Test that creating ECX keys with a short private key fails as expected */
static int test_ecx_short_keys(int tst)
{
    unsigned char ecxkeydata = 1;
    EVP_PKEY *pkey;


    pkey = EVP_PKEY_new_raw_private_key(ecxnids[tst], NULL, &ecxkeydata, 1);
    if (!TEST_ptr_null(pkey)) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    return 1;
}

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
# endif
#endif

    ADD_ALL_TESTS(test_ecx_short_keys, OSSL_NELEM(ecxnids));

    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
{
    OSSL_PARAM_BLD *bld = NULL;
    OSSL_PARAM *params = NULL;
    BIGNUM *n = NULL, *e = NULL, *d = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    EVP_PKEY *pkey = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our RSA object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
     */
    if (!TEST_ptr(pctx = EVP_PKEY_CTX_new_from_name(testctx, "RSA", NULL))
        || !TEST_ptr(bld = OSSL_PARAM_BLD_new())
        || !TEST_ptr(n = BN_new())
        || !TEST_ptr(e = BN_new())
        || !TEST_ptr(d = BN_new()))
        goto err;
    if (!TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_N, n))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_E, e))
        || !TEST_true(OSSL_PARAM_BLD_push_BN(bld, OSSL_PKEY_PARAM_RSA_D, d)))
        goto err;
    if (!TEST_ptr(params = OSSL_PARAM_BLD_to_param(bld)))
        goto err;

    if (!TEST_int_gt(EVP_PKEY_fromdata_init(pctx), 0)
        || !TEST_int_gt(EVP_PKEY_fromdata(pctx, &pkey, EVP_PKEY_KEYPAIR,
                                          params), 0))
        goto err;

    if (!TEST_ptr(pkey))
        goto err;

    ret = test_EVP_PKEY_CTX_get_set_params(pkey);

 err:
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(pctx);
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
    BN_free(n);
    BN_free(e);
    BN_free(d);

    return ret;
}

#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
static int test_decrypt_null_chunks(void)
{
    EVP_CIPHER_CTX* ctx = NULL;
    EVP_CIPHER *cipher = NULL;
    const unsigned char key[32] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1
    };
    unsigned char iv[12] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b
    };
    unsigned char msg[] = "It was the best of times, it was the worst of times";
    unsigned char ciphertext[80];
    unsigned char plaintext[80];
    /* We initialise tmp to a non zero value on purpose */
    int ctlen, ptlen, tmp = 99;
    int ret = 0;
    const int enc_offset = 10, dec_offset = 20;

    if (!TEST_ptr(cipher = EVP_CIPHER_fetch(testctx, "ChaCha20-Poly1305", testpropq))
            || !TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_true(EVP_EncryptInit_ex(ctx, cipher, NULL,
                                             key, iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &ctlen, msg,
                                            enc_offset))
            /* Deliberate add a zero length update */
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext + ctlen, &tmp,
                                            msg + enc_offset,
                                            sizeof(msg) - enc_offset))
            || !TEST_int_eq(ctlen += tmp, sizeof(msg))
            || !TEST_true(EVP_EncryptFinal(ctx, ciphertext + ctlen, &tmp))
            || !TEST_int_eq(tmp, 0))
        goto err;

    /* Deliberately initialise tmp to a non zero value */
    tmp = 99;
    if (!TEST_true(EVP_DecryptInit_ex(ctx, cipher, NULL, key, iv))
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext, &ptlen, ciphertext,
                                            dec_offset))
            /*
             * Deliberately add a zero length update. We also deliberately do
             * this at a different offset than for encryption.
             */
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp, NULL,
                                            0))
            || !TEST_int_eq(tmp, 0)
            || !TEST_true(EVP_DecryptUpdate(ctx, plaintext + ptlen, &tmp,
                                            ciphertext + dec_offset,
                                            ctlen - dec_offset))
            || !TEST_int_eq(ptlen += tmp, sizeof(msg))
            || !TEST_true(EVP_DecryptFinal(ctx, plaintext + ptlen, &tmp))
            || !TEST_int_eq(tmp, 0)
            || !TEST_mem_eq(msg, sizeof(msg), plaintext, ptlen))
        goto err;

    ret = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(cipher);
    return ret;
}
#endif /* !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305) */

#ifndef OPENSSL_NO_DH
/*
 * Test combinations of private, public, missing and private + public key
 * params to ensure they are all accepted
 */
static int test_DH_priv_pub(void)
{
    return test_EVP_PKEY_ffc_priv_pub("DH");
}

# ifndef OPENSSL_NO_DEPRECATED_3_0
static int test_EVP_PKEY_set1_DH(void)
{
    DH *x942dh = NULL, *noqdh = NULL;
    EVP_PKEY *pkey1 = NULL, *pkey2 = NULL;
    int ret = 0;
    BIGNUM *p, *g = NULL;
    BIGNUM *pubkey = NULL;
    unsigned char pub[2048 / 8];
    size_t len = 0;

    if (!TEST_ptr(p = BN_new())
            || !TEST_ptr(g = BN_new())
            || !TEST_ptr(pubkey = BN_new())
            || !TEST_true(BN_set_word(p, 9999))
            || !TEST_true(BN_set_word(g, 2))
            || !TEST_true(BN_set_word(pubkey, 4321))
            || !TEST_ptr(noqdh = DH_new())
            || !TEST_true(DH_set0_pqg(noqdh, p, NULL, g))
            || !TEST_true(DH_set0_key(noqdh, pubkey, NULL))
            || !TEST_ptr(pubkey = BN_new())
            || !TEST_true(BN_set_word(pubkey, 4321)))
        goto err;
    p = g = NULL;

    x942dh = DH_get_2048_256();
    pkey1 = EVP_PKEY_new();
    pkey2 = EVP_PKEY_new();
    if (!TEST_ptr(x942dh)
            || !TEST_ptr(noqdh)
            || !TEST_ptr(pkey1)
            || !TEST_ptr(pkey2)
            || !TEST_true(DH_set0_key(x942dh, pubkey, NULL)))
        goto err;
    pubkey = NULL;

    if (!TEST_true(EVP_PKEY_set1_DH(pkey1, x942dh))
            || !TEST_int_eq(EVP_PKEY_get_id(pkey1), EVP_PKEY_DHX))
        goto err;

    if (!TEST_true(EVP_PKEY_get_bn_param(pkey1, OSSL_PKEY_PARAM_PUB_KEY,
                                         &pubkey))
            || !TEST_ptr(pubkey))
        goto err;

    if (!TEST_true(EVP_PKEY_set1_DH(pkey2, noqdh))
            || !TEST_int_eq(EVP_PKEY_get_id(pkey2), EVP_PKEY_DH))
        goto err;

    if (!TEST_true(EVP_PKEY_get_octet_string_param(pkey2,
                                                   OSSL_PKEY_PARAM_PUB_KEY,
                                                   pub, sizeof(pub), &len))
            || !TEST_size_t_ne(len, 0))
        goto err;

    ret = 1;
 err:
    BN_free(p);
    BN_free(g);
    BN_free(pubkey);
    EVP_PKEY_free(pkey1);
    EVP_PKEY_free(pkey2);
    DH_free(x942dh);
    DH_free(noqdh);

    return ret;
}
# endif /* !OPENSSL_NO_DEPRECATED_3_0 */
#endif /* !OPENSSL_NO_DH */

/*
 * We test what happens with an empty template.  For the sake of this test,
 * the template must be ignored, and we know that's the case for RSA keys
 * (this might arguably be a misfeature, but that's what we currently do,
 * even in provider code, since that's how the legacy RSA implementation
 * does things)
 */
static int test_keygen_with_empty_template(int n)
{
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY *tkey = NULL;
    int ret = 0;

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    switch (n) {
    case 0:
        /* We do test with no template at all as well */
        if (!TEST_ptr(ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL)))
            goto err;
        break;
    case 1:
        /* Here we create an empty RSA key that serves as our template */
        if (!TEST_ptr(tkey = EVP_PKEY_new())
            || !TEST_true(EVP_PKEY_set_type(tkey, EVP_PKEY_RSA))
            || !TEST_ptr(ctx = EVP_PKEY_CTX_new(tkey, NULL)))
            goto err;
        break;
    }

    if (!TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        || !TEST_int_gt(EVP_PKEY_keygen(ctx, &pkey), 0))
        goto err;

    ret = 1;
 err:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_free(tkey);
    return ret;
}

/*
 * Test that we fail if we attempt to use an algorithm that is not available
 * in the current library context (unless we are using an algorithm that
 * should be made available via legacy codepaths).
 *
 * 0:   RSA
 * 1:   SM2
 */
static int test_pkey_ctx_fail_without_provider(int tst)
{
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_PROVIDER *tmpnullprov = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    const char *keytype = NULL;
    int expect_null = 0;
    int ret = 0;

    if (!TEST_ptr(tmpctx))
        goto err;

    tmpnullprov = OSSL_PROVIDER_load(tmpctx, "null");
    if (!TEST_ptr(tmpnullprov))
        goto err;

    /*
     * We check for certain algos in the null provider.
     * If an algo is expected to have a provider keymgmt, contructing an
     * EVP_PKEY_CTX is expected to fail (return NULL).
     * Otherwise, if it's expected to have legacy support, contructing an
     * EVP_PKEY_CTX is expected to succeed (return non-NULL).
     */
    switch (tst) {
    case 0:
        keytype = "RSA";
        expect_null = 1;
        break;
    case 1:
        keytype = "SM2";
        expect_null = 1;
#ifdef OPENSSL_NO_EC
        TEST_info("EC disable, skipping SM2 check...");
        goto end;
#endif
#ifdef OPENSSL_NO_SM2
        TEST_info("SM2 disable, skipping SM2 check...");
        goto end;
#endif
        break;
    default:
        TEST_error("No test for case %d", tst);
        goto err;
    }

    pctx = EVP_PKEY_CTX_new_from_name(tmpctx, keytype, "");
    if (expect_null ? !TEST_ptr_null(pctx) : !TEST_ptr(pctx))
        goto err;

#if defined(OPENSSL_NO_EC) || defined(OPENSSL_NO_SM2)
 end:
#endif
    ret = 1;

 err:
    EVP_PKEY_CTX_free(pctx);
    OSSL_PROVIDER_unload(tmpnullprov);
    OSSL_LIB_CTX_free(tmpctx);
    return ret;
}

static int test_rand_agglomeration(void)
{
    EVP_RAND *rand;
    EVP_RAND_CTX *ctx;
    OSSL_PARAM params[3], *p = params;
    int res;
    unsigned int step = 7;
    static unsigned char seed[] = "It does not matter how slowly you go "
                                  "as long as you do not stop.";
    unsigned char out[sizeof(seed)];

    if (!TEST_int_ne(sizeof(seed) % step, 0)
            || !TEST_ptr(rand = EVP_RAND_fetch(testctx, "TEST-RAND", testpropq)))
        return 0;
    ctx = EVP_RAND_CTX_new(rand, NULL);
    EVP_RAND_free(rand);
    if (!TEST_ptr(ctx))
        return 0;

    memset(out, 0, sizeof(out));
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_RAND_PARAM_TEST_ENTROPY,
                                             seed, sizeof(seed));
    *p++ = OSSL_PARAM_construct_uint(OSSL_RAND_PARAM_MAX_REQUEST, &step);
    *p = OSSL_PARAM_construct_end();
    res = TEST_true(EVP_RAND_CTX_set_params(ctx, params))
          && TEST_true(EVP_RAND_generate(ctx, out, sizeof(out), 0, 1, NULL, 0))
          && TEST_mem_eq(seed, sizeof(seed), out, sizeof(out));
    EVP_RAND_CTX_free(ctx);
    return res;
}

/*
 * Test that we correctly return the original or "running" IV after
 * an encryption operation.
 * Run multiple times for some different relevant algorithms/modes.
 */
static int test_evp_iv_aes(int idx)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char key[16] = {0x4c, 0x43, 0xdb, 0xdd, 0x42, 0x73, 0x47, 0xd1,
                             0xe5, 0x62, 0x7d, 0xcd, 0x4d, 0x76, 0x4d, 0x57};
    unsigned char init_iv[EVP_MAX_IV_LENGTH] =
        {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98, 0x82,
         0x5a, 0x55, 0x91, 0x81, 0x42, 0xa8, 0x89, 0x34};
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                         9, 10, 11, 12, 13, 14, 15, 16 };
    unsigned char ciphertext[32], oiv[16], iv[16];
    unsigned char *ref_iv;
    unsigned char cbc_state[16] = {0x10, 0x2f, 0x05, 0xcc, 0xc2, 0x55, 0x72, 0xb9,
                                   0x88, 0xe6, 0x4a, 0x17, 0x10, 0x74, 0x22, 0x5e};

    unsigned char ofb_state[16] = {0x76, 0xe6, 0x66, 0x61, 0xd0, 0x8a, 0xe4, 0x64,
                                   0xdd, 0x66, 0xbf, 0x00, 0xf0, 0xe3, 0x6f, 0xfd};
    unsigned char cfb_state[16] = {0x77, 0xe4, 0x65, 0x65, 0xd5, 0x8c, 0xe3, 0x6c,
                                   0xd4, 0x6c, 0xb4, 0x0c, 0xfd, 0xed, 0x60, 0xed};
    unsigned char gcm_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
    unsigned char ccm_state[7] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b, 0x98};
#ifndef OPENSSL_NO_OCB
    unsigned char ocb_state[12] = {0x57, 0x71, 0x7d, 0xad, 0xdb, 0x9b,
                                   0x98, 0x82, 0x5a, 0x55, 0x91, 0x81};
#endif
    int len = sizeof(ciphertext);
    size_t ivlen, ref_len;
    const EVP_CIPHER *type = NULL;
    int iv_reset = 0;

    if (nullprov != NULL && idx < 6)
        return TEST_skip("Test does not support a non-default library context");

    switch(idx) {
    case 0:
        type = EVP_aes_128_cbc();
        /* FALLTHROUGH */
    case 6:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-cbc", testpropq);
        ref_iv = cbc_state;
        ref_len = sizeof(cbc_state);
        iv_reset = 1;
        break;
    case 1:
        type = EVP_aes_128_ofb();
        /* FALLTHROUGH */
    case 7:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ofb", testpropq);
        ref_iv = ofb_state;
        ref_len = sizeof(ofb_state);
        iv_reset = 1;
        break;
    case 2:
        type = EVP_aes_128_cfb();
        /* FALLTHROUGH */
    case 8:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-cfb", testpropq);
        ref_iv = cfb_state;
        ref_len = sizeof(cfb_state);
        iv_reset = 1;
        break;
    case 3:
        type = EVP_aes_128_gcm();
        /* FALLTHROUGH */
    case 9:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-gcm", testpropq);
        ref_iv = gcm_state;
        ref_len = sizeof(gcm_state);
        break;
    case 4:
        type = EVP_aes_128_ccm();
        /* FALLTHROUGH */
    case 10:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ccm", testpropq);
        ref_iv = ccm_state;
        ref_len = sizeof(ccm_state);
        break;
#ifdef OPENSSL_NO_OCB
    case 5:
    case 11:
        return 1;
#else
    case 5:
        type = EVP_aes_128_ocb();
        /* FALLTHROUGH */
    case 11:
        type = (type != NULL) ? type :
                                EVP_CIPHER_fetch(testctx, "aes-128-ocb", testpropq);
        ref_iv = ocb_state;
        ref_len = sizeof(ocb_state);
        break;
#endif
    default:
        return 0;
    }

    if (!TEST_ptr(type)
            || !TEST_ptr((ctx = EVP_CIPHER_CTX_new()))
            || !TEST_true(EVP_EncryptInit_ex(ctx, type, NULL, key, init_iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &len, msg,
                          (int)sizeof(msg)))
            || !TEST_true(EVP_CIPHER_CTX_get_original_iv(ctx, oiv, sizeof(oiv)))
            || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx, ciphertext, &len)))
        goto err;
    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
    if (!TEST_mem_eq(init_iv, ivlen, oiv, ivlen)
            || !TEST_mem_eq(ref_iv, ref_len, iv, ivlen))
        goto err;

    /* CBC, OFB, and CFB modes: the updated iv must be reset after reinit */
    if (!TEST_true(EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL))
        || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv))))
        goto err;
    if (iv_reset) {
        if (!TEST_mem_eq(init_iv, ivlen, iv, ivlen))
            goto err;
    } else {
        if (!TEST_mem_eq(ref_iv, ivlen, iv, ivlen))
            goto err;
    }

    ret = 1;
err:
    EVP_CIPHER_CTX_free(ctx);
    if (idx >= 6)
        EVP_CIPHER_free((EVP_CIPHER *)type);
    return ret;
}

#ifndef OPENSSL_NO_DES
static int test_evp_iv_des(int idx)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx = NULL;
    static const unsigned char key[24] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xf1, 0xe0, 0xd3, 0xc2, 0xb5, 0xa4, 0x97, 0x86,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    static const unsigned char init_iv[8] = {
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    static const unsigned char msg[] = { 1, 2, 3, 4, 5, 6, 7, 8,
                                         9, 10, 11, 12, 13, 14, 15, 16 };
    unsigned char ciphertext[32], oiv[8], iv[8];
    unsigned const char *ref_iv;
    static const unsigned char cbc_state_des[8] = {
        0x4f, 0xa3, 0x85, 0xcd, 0x8b, 0xf3, 0x06, 0x2a
    };
    static const unsigned char cbc_state_3des[8] = {
        0x35, 0x27, 0x7d, 0x65, 0x6c, 0xfb, 0x50, 0xd9
    };
    static const unsigned char ofb_state_des[8] = {
        0xa7, 0x0d, 0x1d, 0x45, 0xf9, 0x96, 0x3f, 0x2c
    };
    static const unsigned char ofb_state_3des[8] = {
        0xab, 0x16, 0x24, 0xbb, 0x5b, 0xac, 0xed, 0x5e
    };
    static const unsigned char cfb_state_des[8] = {
        0x91, 0xeb, 0x6d, 0x29, 0x4b, 0x08, 0xbd, 0x73
    };
    static const unsigned char cfb_state_3des[8] = {
        0x34, 0xdd, 0xfb, 0x47, 0x33, 0x1c, 0x61, 0xf7
    };
    int len = sizeof(ciphertext);
    size_t ivlen, ref_len;
    EVP_CIPHER *type = NULL;

    if (lgcyprov == NULL && idx < 3)
        return TEST_skip("Test requires legacy provider to be loaded");

    switch(idx) {
    case 0:
        type = EVP_CIPHER_fetch(testctx, "des-cbc", testpropq);
        ref_iv = cbc_state_des;
        ref_len = sizeof(cbc_state_des);
        break;
    case 1:
        type = EVP_CIPHER_fetch(testctx, "des-ofb", testpropq);
        ref_iv = ofb_state_des;
        ref_len = sizeof(ofb_state_des);
        break;
    case 2:
        type = EVP_CIPHER_fetch(testctx, "des-cfb", testpropq);
        ref_iv = cfb_state_des;
        ref_len = sizeof(cfb_state_des);
        break;
    case 3:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-cbc", testpropq);
        ref_iv = cbc_state_3des;
        ref_len = sizeof(cbc_state_3des);
        break;
    case 4:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-ofb", testpropq);
        ref_iv = ofb_state_3des;
        ref_len = sizeof(ofb_state_3des);
        break;
    case 5:
        type = EVP_CIPHER_fetch(testctx, "des-ede3-cfb", testpropq);
        ref_iv = cfb_state_3des;
        ref_len = sizeof(cfb_state_3des);
        break;
    default:
        return 0;
    }

    if (!TEST_ptr(type)
            || !TEST_ptr((ctx = EVP_CIPHER_CTX_new()))
            || !TEST_true(EVP_EncryptInit_ex(ctx, type, NULL, key, init_iv))
            || !TEST_true(EVP_EncryptUpdate(ctx, ciphertext, &len, msg,
                          (int)sizeof(msg)))
            || !TEST_true(EVP_CIPHER_CTX_get_original_iv(ctx, oiv, sizeof(oiv)))
            || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx, ciphertext, &len)))
        goto err;
    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
    if (!TEST_mem_eq(init_iv, ivlen, oiv, ivlen)
            || !TEST_mem_eq(ref_iv, ref_len, iv, ivlen))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, NULL, NULL, NULL, NULL))
        || !TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, iv, sizeof(iv))))
        goto err;
    if (!TEST_mem_eq(init_iv, ivlen, iv, ivlen))
        goto err;

    ret = 1;
err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return ret;
}
#endif

#ifndef OPENSSL_NO_BF
static int test_evp_bf_default_keylen(int idx)
{
    int ret = 0;
    static const char *algos[4] = {
        "bf-ecb", "bf-cbc", "bf-cfb", "bf-ofb"
    };
    int ivlen[4] = { 0, 8, 8, 8 };
    EVP_CIPHER *cipher = NULL;

    if (lgcyprov == NULL)
        return TEST_skip("Test requires legacy provider to be loaded");

    if (!TEST_ptr(cipher = EVP_CIPHER_fetch(testctx, algos[idx], testpropq))
            || !TEST_int_eq(EVP_CIPHER_get_key_length(cipher), 16)
            || !TEST_int_eq(EVP_CIPHER_get_iv_length(cipher), ivlen[idx]))
        goto err;

    ret = 1;
err:
    EVP_CIPHER_free(cipher);
    return ret;
}
#endif

#ifndef OPENSSL_NO_EC
static int ecpub_nids[] = {
    NID_brainpoolP256r1, NID_X9_62_prime256v1,
    NID_secp384r1, NID_secp521r1,
# ifndef OPENSSL_NO_EC2M
    NID_sect233k1, NID_sect233r1, NID_sect283r1,
    NID_sect409k1, NID_sect409r1, NID_sect571k1, NID_sect571r1,
# endif
    NID_brainpoolP384r1, NID_brainpoolP512r1
};

static int test_ecpub(int idx)
{
    int ret = 0, len, savelen;
    int nid;
    unsigned char buf[1024];
    unsigned char *p;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;
# ifndef OPENSSL_NO_DEPRECATED_3_0
    const unsigned char *q;
    EVP_PKEY *pkey2 = NULL;
    EC_KEY *ec = NULL;
# endif

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    nid = ecpub_nids[idx];

    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL);
    if (!TEST_ptr(ctx)
        || !TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        || !TEST_int_gt(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, nid), 0)
        || !TEST_true(EVP_PKEY_keygen(ctx, &pkey)))
        goto done;
    len = i2d_PublicKey(pkey, NULL);
    savelen = len;
    if (!TEST_int_ge(len, 1)
        || !TEST_int_lt(len, 1024))
        goto done;
    p = buf;
    len = i2d_PublicKey(pkey, &p);
    if (!TEST_int_ge(len, 1)
            || !TEST_int_eq(len, savelen))
        goto done;

# ifndef OPENSSL_NO_DEPRECATED_3_0
    /* Now try to decode the just-created DER. */
    q = buf;
    if (!TEST_ptr((pkey2 = EVP_PKEY_new()))
            || !TEST_ptr((ec = EC_KEY_new_by_curve_name(nid)))
            || !TEST_true(EVP_PKEY_assign_EC_KEY(pkey2, ec)))
        goto done;
    /* EC_KEY ownership transferred */
    ec = NULL;
    if (!TEST_ptr(d2i_PublicKey(EVP_PKEY_EC, &pkey2, &q, savelen)))
        goto done;
    /* The keys should match. */
    if (!TEST_int_eq(EVP_PKEY_eq(pkey, pkey2), 1))
        goto done;
# endif

    ret = 1;

 done:
    EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    EVP_PKEY_free(pkey2);
    EC_KEY_free(ec);
# endif
    return ret;
}
#endif

static int test_EVP_rsa_pss_with_keygen_bits(void)
{
    int ret = 0;
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    EVP_MD *md;

    md = EVP_MD_fetch(testctx, "sha256", testpropq);
    ret = TEST_ptr(md)
        && TEST_ptr((ctx = EVP_PKEY_CTX_new_from_name(testctx, "RSA-PSS", testpropq)))
        && TEST_int_gt(EVP_PKEY_keygen_init(ctx), 0)
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 512), 0)
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_pss_keygen_md(ctx, md), 0)
        && TEST_true(EVP_PKEY_keygen(ctx, &pkey));

    EVP_MD_free(md);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    return ret;
}

static int test_EVP_rsa_pss_set_saltlen(void)
{
    int ret = 0;
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pkey_ctx = NULL;
    EVP_MD *sha256 = NULL;
    EVP_MD_CTX *sha256_ctx = NULL;
    int saltlen = 9999; /* buggy EVP_PKEY_CTX_get_rsa_pss_saltlen() didn't update this */
    const int test_value = 32;

    ret = TEST_ptr(pkey = load_example_rsa_key())
        && TEST_ptr(sha256 = EVP_MD_fetch(testctx, "sha256", NULL))
        && TEST_ptr(sha256_ctx = EVP_MD_CTX_new())
        && TEST_true(EVP_DigestSignInit(sha256_ctx, &pkey_ctx, sha256, NULL, pkey))
        && TEST_true(EVP_PKEY_CTX_set_rsa_padding(pkey_ctx, RSA_PKCS1_PSS_PADDING))
        && TEST_int_gt(EVP_PKEY_CTX_set_rsa_pss_saltlen(pkey_ctx, test_value), 0)
        && TEST_int_gt(EVP_PKEY_CTX_get_rsa_pss_saltlen(pkey_ctx, &saltlen), 0)
        && TEST_int_eq(saltlen, test_value);

    EVP_MD_CTX_free(sha256_ctx);
    EVP_PKEY_free(pkey);
    EVP_MD_free(sha256);

    return ret;
}

static int success = 1;
static void md_names(const char *name, void *vctx)
{
    OSSL_LIB_CTX *ctx = (OSSL_LIB_CTX *)vctx;
    /* Force a namemap update */
    EVP_CIPHER *aes128 = EVP_CIPHER_fetch(ctx, "AES-128-CBC", NULL);

    if (!TEST_ptr(aes128))
        success = 0;

    EVP_CIPHER_free(aes128);
}

/*
 * Test that changing the namemap in a user callback works in a names_do_all
 * function.
 */
static int test_names_do_all(void)
{
    /* We use a custom libctx so that we know the state of the namemap */
    OSSL_LIB_CTX *ctx = OSSL_LIB_CTX_new();
    EVP_MD *sha256 = NULL;
    int testresult = 0;

    if (!TEST_ptr(ctx))
        goto err;

    sha256 = EVP_MD_fetch(ctx, "SHA2-256", NULL);
    if (!TEST_ptr(sha256))
        goto err;

    /*
     * We loop through all the names for a given digest. This should still work
     * even if the namemap changes part way through.
     */
    if (!TEST_true(EVP_MD_names_do_all(sha256, md_names, ctx)))
        goto err;

    if (!TEST_true(success))
        goto err;

    testresult = 1;
 err:
    EVP_MD_free(sha256);
    OSSL_LIB_CTX_free(ctx);
    return testresult;
}

typedef struct {
    const char *cipher;
    const unsigned char *key;
    const unsigned char *iv;
    const unsigned char *input;
    const unsigned char *expected;
    const unsigned char *tag;
    size_t ivlen; /* 0 if we do not need to set a specific IV len */
    size_t inlen;
    size_t expectedlen;
    size_t taglen;
    int keyfirst;
    int initenc;
    int finalenc;
} EVP_INIT_TEST_st;

static const EVP_INIT_TEST_st evp_init_tests[] = {
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbPlaintext,
        cfbCiphertext, NULL, 0, sizeof(cfbPlaintext), sizeof(cfbCiphertext),
        0, 1, 0, 1
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultPlaintext,
        gcmDefaultCiphertext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultPlaintext), sizeof(gcmDefaultCiphertext),
        sizeof(gcmDefaultTag), 1, 0, 1
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbPlaintext,
        cfbCiphertext, NULL, 0, sizeof(cfbPlaintext), sizeof(cfbCiphertext),
        0, 0, 0, 1
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultPlaintext,
        gcmDefaultCiphertext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultPlaintext), sizeof(gcmDefaultCiphertext),
        sizeof(gcmDefaultTag), 0, 0, 1
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbCiphertext,
        cfbPlaintext, NULL, 0, sizeof(cfbCiphertext), sizeof(cfbPlaintext),
        0, 1, 1, 0
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultCiphertext,
        gcmDefaultPlaintext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultCiphertext), sizeof(gcmDefaultPlaintext),
        sizeof(gcmDefaultTag), 1, 1, 0
    },
    {
        "aes-128-cfb", kCFBDefaultKey, iCFBIV, cfbCiphertext,
        cfbPlaintext, NULL, 0, sizeof(cfbCiphertext), sizeof(cfbPlaintext),
        0, 0, 1, 0
    },
    {
        "aes-256-gcm", kGCMDefaultKey, iGCMDefaultIV, gcmDefaultCiphertext,
        gcmDefaultPlaintext, gcmDefaultTag, sizeof(iGCMDefaultIV),
        sizeof(gcmDefaultCiphertext), sizeof(gcmDefaultPlaintext),
        sizeof(gcmDefaultTag), 0, 1, 0
    }
};

static int evp_init_seq_set_iv(EVP_CIPHER_CTX *ctx, const EVP_INIT_TEST_st *t)
{
    int res = 0;
    
    if (t->ivlen != 0) {
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen, NULL), 0))
            goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, t->iv, -1)))
        goto err;
    res = 1;
 err:
    return res;
}

/*
 * Test step-wise cipher initialization via EVP_CipherInit_ex where the
 * arguments are given one at a time and a final adjustment to the enc
 * parameter sets the correct operation.
 */
static int test_evp_init_seq(int idx)
{
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    unsigned char tag[16];
    const EVP_INIT_TEST_st *t = &evp_init_tests[idx];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    size_t taglen = sizeof(tag);
    char *errmsg = NULL;

    ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, t->cipher, testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, t->initenc))) {
        errmsg = "EMPTY_ENC_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (t->keyfirst && !TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, t->key, NULL, -1))) {
        errmsg = "KEY_INIT (before iv)";
        goto err;
    }
    if (!evp_init_seq_set_iv(ctx, t)) {
        errmsg = "IV_INIT";
        goto err;
    }
    if (t->keyfirst == 0 &&  !TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, t->key, NULL, -1))) {
        errmsg = "KEY_INIT (after iv)";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, NULL, t->finalenc))) {
        errmsg = "FINAL_ENC_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (t->finalenc == 0 && t->tag != NULL) {
        /* Set expected tag */
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG,
                                           t->taglen, (void *)t->tag), 0)) {
            errmsg = "SET_TAG";
            goto err;
        }
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT";
        goto err;
    }
    if (t->finalenc != 0 && t->tag != NULL) {
        if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
            errmsg = "GET_TAG";
            goto err;
        }
        if (!TEST_mem_eq(t->tag, t->taglen, tag, taglen)) {
            errmsg = "TAG_ERROR";
            goto err;
        }
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("evp_init_test %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

typedef struct {
    const unsigned char *input;
    const unsigned char *expected;
    size_t inlen;
    size_t expectedlen;
    int enc;
} EVP_RESET_TEST_st;

static const EVP_RESET_TEST_st evp_reset_tests[] = {
    {
        cfbPlaintext, cfbCiphertext,
        sizeof(cfbPlaintext), sizeof(cfbCiphertext), 1
    },
    {
        cfbCiphertext, cfbPlaintext,
        sizeof(cfbCiphertext), sizeof(cfbPlaintext), 0
    }
};

/*
 * Test a reset of a cipher via EVP_CipherInit_ex after the cipher has already
 * been used.
 */
static int test_evp_reset(int idx)
{
    const EVP_RESET_TEST_st *t = &evp_reset_tests[idx];
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, "aes-128-cfb", testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, kCFBDefaultKey, iCFBIV, t->enc))) {
        errmsg = "CIPHER_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, NULL, -1))) {
        errmsg = "CIPHER_REINIT";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, t->input, t->inlen))) {
        errmsg = "CIPHER_UPDATE (reinit)";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL (reinit)";
        goto err;
    }
    if (!TEST_mem_eq(t->expected, t->expectedlen, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT (reinit)";
        goto err;
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("test_evp_reset %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;    
}

typedef struct {
    const char *cipher;
    int enc;
} EVP_UPDATED_IV_TEST_st;

static const EVP_UPDATED_IV_TEST_st evp_updated_iv_tests[] = {
    {
        "aes-128-cfb", 1
    },
    {
        "aes-128-cfb", 0
    },
    {
        "aes-128-cfb1", 1
    },
    {
        "aes-128-cfb1", 0
    },
    {
        "aes-128-cfb8", 1
    },
    {
        "aes-128-cfb8", 0
    },
    {
        "aes-128-ofb", 1
    },
    {
        "aes-128-ofb", 0
    },
    {
        "aes-128-ctr", 1
    },
    {
        "aes-128-ctr", 0
    },
    {
        "aes-128-cbc", 1
    },
    {
        "aes-128-cbc", 0
    }
};

/*
 * Test that the IV in the context is updated during a crypto operation for CFB
 * and OFB.
 */
static int test_evp_updated_iv(int idx)
{
    const EVP_UPDATED_IV_TEST_st *t = &evp_updated_iv_tests[idx];
    int outlen1, outlen2;
    int testresult = 0;
    unsigned char outbuf[1024];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    unsigned char updated_iv[EVP_MAX_IV_LENGTH];
    int iv_len;
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if ((type = EVP_CIPHER_fetch(testctx, t->cipher, testpropq)) == NULL) {
        TEST_info("cipher %s not supported, skipping", t->cipher);
        goto ok;
    }

    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, kCFBDefaultKey, iCFBIV, t->enc))) {
        errmsg = "CIPHER_INIT";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_set_padding(ctx, 0))) {
        errmsg = "PADDING";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, cfbPlaintext, sizeof(cfbPlaintext)))) {
        errmsg = "CIPHER_UPDATE";
        goto err;
    }
    if (!TEST_true(EVP_CIPHER_CTX_get_updated_iv(ctx, updated_iv, sizeof(updated_iv)))) {
        errmsg = "CIPHER_CTX_GET_UPDATED_IV";
        goto err;
    }
    if (!TEST_true(iv_len = EVP_CIPHER_CTX_get_iv_length(ctx))) {
        errmsg = "CIPHER_CTX_GET_IV_LEN";
        goto err;
    }
    if (!TEST_mem_ne(iCFBIV, sizeof(iCFBIV), updated_iv, iv_len)) {
        errmsg = "IV_NOT_UPDATED";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL";
        goto err;
    }
 ok:
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("test_evp_updated_iv %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

typedef struct {
    const unsigned char *iv1;
    const unsigned char *iv2;
    const unsigned char *expected1;
    const unsigned char *expected2;
    const unsigned char *tag1;
    const unsigned char *tag2;
    size_t ivlen1;
    size_t ivlen2;
    size_t expectedlen1;
    size_t expectedlen2;
} TEST_GCM_IV_REINIT_st;

static const TEST_GCM_IV_REINIT_st gcm_reinit_tests[] = {
    {
        iGCMResetIV1, iGCMResetIV2, gcmResetCiphertext1, gcmResetCiphertext2,
        gcmResetTag1, gcmResetTag2, sizeof(iGCMResetIV1), sizeof(iGCMResetIV2),
        sizeof(gcmResetCiphertext1), sizeof(gcmResetCiphertext2)
    },
    {
        iGCMResetIV2, iGCMResetIV1, gcmResetCiphertext2, gcmResetCiphertext1,
        gcmResetTag2, gcmResetTag1, sizeof(iGCMResetIV2), sizeof(iGCMResetIV1),
        sizeof(gcmResetCiphertext2), sizeof(gcmResetCiphertext1)
    }
};

static int test_gcm_reinit(int idx)
{
    int outlen1, outlen2, outlen3;
    int testresult = 0;
    unsigned char outbuf[1024];
    unsigned char tag[16];
    const TEST_GCM_IV_REINIT_st *t = &gcm_reinit_tests[idx];
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *type = NULL;
    size_t taglen = sizeof(tag);
    char *errmsg = NULL;

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())) {
        errmsg = "CTX_ALLOC";
        goto err;
    }
    if (!TEST_ptr(type = EVP_CIPHER_fetch(testctx, "aes-256-gcm", testpropq))) {
        errmsg = "CIPHER_FETCH";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, type, NULL, NULL, NULL, 1))) {
        errmsg = "ENC_INIT";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen1, NULL), 0)) {
        errmsg = "SET_IVLEN1";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, kGCMResetKey, t->iv1, 1))) {
        errmsg = "SET_IV1";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, NULL, &outlen3, gcmAAD, sizeof(gcmAAD)))) {
        errmsg = "AAD1";
        goto err;
    }
    EVP_CIPHER_CTX_set_padding(ctx, 0);
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, gcmResetPlaintext,
                                    sizeof(gcmResetPlaintext)))) {
        errmsg = "CIPHER_UPDATE1";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL1";
        goto err;
    }
    if (!TEST_mem_eq(t->expected1, t->expectedlen1, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT1";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
        errmsg = "GET_TAG1";
        goto err;
    }
    if (!TEST_mem_eq(t->tag1, taglen, tag, taglen)) {
        errmsg = "TAG_ERROR1";
        goto err;
    }
    /* Now reinit */
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, t->ivlen2, NULL), 0)) {
        errmsg = "SET_IVLEN2";
        goto err;
    }
    if (!TEST_true(EVP_CipherInit_ex(ctx, NULL, NULL, NULL, t->iv2, -1))) {
        errmsg = "SET_IV2";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, NULL, &outlen3, gcmAAD, sizeof(gcmAAD)))) {
        errmsg = "AAD2";
        goto err;
    }
    if (!TEST_true(EVP_CipherUpdate(ctx, outbuf, &outlen1, gcmResetPlaintext,
                                    sizeof(gcmResetPlaintext)))) {
        errmsg = "CIPHER_UPDATE2";
        goto err;
    }
    if (!TEST_true(EVP_CipherFinal_ex(ctx, outbuf + outlen1, &outlen2))) {
        errmsg = "CIPHER_FINAL2";
        goto err;
    }
    if (!TEST_mem_eq(t->expected2, t->expectedlen2, outbuf, outlen1 + outlen2)) {
        errmsg = "WRONG_RESULT2";
        goto err;
    }
    if (!TEST_int_gt(EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, taglen, tag), 0)) {
        errmsg = "GET_TAG2";
        goto err;
    }
    if (!TEST_mem_eq(t->tag2, taglen, tag, taglen)) {
        errmsg = "TAG_ERROR2";
        goto err;
    }
    testresult = 1;
 err:
    if (errmsg != NULL)
        TEST_info("evp_init_test %d: %s", idx, errmsg);
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_free(type);
    return testresult;
}

#ifndef OPENSSL_NO_DEPRECATED_3_0
static EVP_PKEY_METHOD *custom_pmeth =  NULL;
static const EVP_PKEY_METHOD *orig_pmeth = NULL;

# define EVP_PKEY_CTRL_MY_COMMAND 9999

static int custom_pmeth_init(EVP_PKEY_CTX *ctx)
{
    int (*pinit)(EVP_PKEY_CTX *ctx);

    EVP_PKEY_meth_get_init(orig_pmeth, &pinit);
    return pinit(ctx);
}

static void custom_pmeth_cleanup(EVP_PKEY_CTX *ctx)
{
    void (*pcleanup)(EVP_PKEY_CTX *ctx);

    EVP_PKEY_meth_get_cleanup(orig_pmeth, &pcleanup);
    pcleanup(ctx);
}

static int custom_pmeth_sign(EVP_PKEY_CTX *ctx, unsigned char *out,
                             size_t *outlen, const unsigned char *in,
                             size_t inlen)
{
    int (*psign)(EVP_PKEY_CTX *ctx, unsigned char *sig, size_t *siglen,
                 const unsigned char *tbs, size_t tbslen);

    EVP_PKEY_meth_get_sign(orig_pmeth, NULL, &psign);
    return psign(ctx, out, outlen, in, inlen);
}

static int custom_pmeth_digestsign(EVP_MD_CTX *ctx, unsigned char *sig,
                                   size_t *siglen, const unsigned char *tbs,
                                   size_t tbslen)
{
    int (*pdigestsign)(EVP_MD_CTX *ctx, unsigned char *sig, size_t *siglen,
                       const unsigned char *tbs, size_t tbslen);

    EVP_PKEY_meth_get_digestsign(orig_pmeth, &pdigestsign);
    return pdigestsign(ctx, sig, siglen, tbs, tbslen);
}

static int custom_pmeth_derive(EVP_PKEY_CTX *ctx, unsigned char *key,
                               size_t *keylen)
{
    int (*pderive)(EVP_PKEY_CTX *ctx, unsigned char *key, size_t *keylen);

    EVP_PKEY_meth_get_derive(orig_pmeth, NULL, &pderive);
    return pderive(ctx, key, keylen);
}

static int custom_pmeth_copy(EVP_PKEY_CTX *dst, const EVP_PKEY_CTX *src)
{
    int (*pcopy)(EVP_PKEY_CTX *dst, const EVP_PKEY_CTX *src);

    EVP_PKEY_meth_get_copy(orig_pmeth, &pcopy);
    return pcopy(dst, src);
}

static int ctrl_called;

static int custom_pmeth_ctrl(EVP_PKEY_CTX *ctx, int type, int p1, void *p2)
{
    int (*pctrl)(EVP_PKEY_CTX *ctx, int type, int p1, void *p2);

    EVP_PKEY_meth_get_ctrl(orig_pmeth, &pctrl, NULL);

    if (type == EVP_PKEY_CTRL_MY_COMMAND) {
        ctrl_called = 1;
        return 1;
    }

    return pctrl(ctx, type, p1, p2);
}

static int test_custom_pmeth(int idx)
{
    EVP_PKEY_CTX *pctx = NULL;
    EVP_MD_CTX *ctx = NULL;
    EVP_PKEY *pkey = NULL;
    int id, orig_id, orig_flags;
    int testresult = 0;
    size_t reslen;
    unsigned char *res = NULL;
    unsigned char msg[] = { 'H', 'e', 'l', 'l', 'o' };
    const EVP_MD *md = EVP_sha256();
    int doderive = 0;

    ctrl_called = 0;

    /* We call deprecated APIs so this test doesn't support a custom libctx */
    if (testctx != NULL)
        return 1;

    switch(idx) {
    case 0:
    case 6:
        id = EVP_PKEY_RSA;
        pkey = load_example_rsa_key();
        break;
    case 1:
    case 7:
# ifndef OPENSSL_NO_DSA
        id = EVP_PKEY_DSA;
        pkey = load_example_dsa_key();
        break;
# else
        return 1;
# endif
    case 2:
    case 8:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_EC;
        pkey = load_example_ec_key();
        break;
# else
        return 1;
# endif
    case 3:
    case 9:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_ED25519;
        md = NULL;
        pkey = load_example_ed25519_key();
        break;
# else
        return 1;
# endif
    case 4:
    case 10:
# ifndef OPENSSL_NO_DH
        id = EVP_PKEY_DH;
        doderive = 1;
        pkey = load_example_dh_key();
        break;
# else
        return 1;
# endif
    case 5:
    case 11:
# ifndef OPENSSL_NO_EC
        id = EVP_PKEY_X25519;
        doderive = 1;
        pkey = load_example_x25519_key();
        break;
# else
        return 1;
# endif
    default:
        TEST_error("Should not happen");
        goto err;
    }

    if (!TEST_ptr(pkey))
        goto err;

    if (idx < 6) {
        if (!TEST_true(evp_pkey_is_provided(pkey)))
            goto err;
    } else {
        EVP_PKEY *tmp = pkey;

        /* Convert to a legacy key */
        pkey = EVP_PKEY_new();
        if (!TEST_ptr(pkey)) {
            pkey = tmp;
            goto err;
        }
        if (!TEST_true(evp_pkey_copy_downgraded(&pkey, tmp))) {
            EVP_PKEY_free(tmp);
            goto err;
        }
        EVP_PKEY_free(tmp);
        if (!TEST_true(evp_pkey_is_legacy(pkey)))
            goto err;
    }

    if (!TEST_ptr(orig_pmeth = EVP_PKEY_meth_find(id))
            || !TEST_ptr(pkey))
        goto err;

    EVP_PKEY_meth_get0_info(&orig_id, &orig_flags, orig_pmeth);
    if (!TEST_int_eq(orig_id, id)
            || !TEST_ptr(custom_pmeth = EVP_PKEY_meth_new(id, orig_flags)))
        goto err;

    if (id == EVP_PKEY_ED25519) {
        EVP_PKEY_meth_set_digestsign(custom_pmeth, custom_pmeth_digestsign);
    } if (id == EVP_PKEY_DH || id == EVP_PKEY_X25519) {
        EVP_PKEY_meth_set_derive(custom_pmeth, NULL, custom_pmeth_derive);
    } else {
        EVP_PKEY_meth_set_sign(custom_pmeth, NULL, custom_pmeth_sign);
    }
    if (id != EVP_PKEY_ED25519 && id != EVP_PKEY_X25519) {
        EVP_PKEY_meth_set_init(custom_pmeth, custom_pmeth_init);
        EVP_PKEY_meth_set_cleanup(custom_pmeth, custom_pmeth_cleanup);
        EVP_PKEY_meth_set_copy(custom_pmeth, custom_pmeth_copy);
    }
    EVP_PKEY_meth_set_ctrl(custom_pmeth, custom_pmeth_ctrl, NULL);
    if (!TEST_true(EVP_PKEY_meth_add0(custom_pmeth)))
        goto err;

    if (doderive) {
        pctx = EVP_PKEY_CTX_new(pkey, NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_derive_init(pctx), 1)
                || !TEST_int_ge(EVP_PKEY_CTX_ctrl(pctx, -1, -1,
                                                EVP_PKEY_CTRL_MY_COMMAND, 0, NULL),
                                1)
                || !TEST_int_eq(ctrl_called, 1)
                || !TEST_int_ge(EVP_PKEY_derive_set_peer(pctx, pkey), 1)
                || !TEST_int_ge(EVP_PKEY_derive(pctx, NULL, &reslen), 1)
                || !TEST_ptr(res = OPENSSL_malloc(reslen))
                || !TEST_int_ge(EVP_PKEY_derive(pctx, res, &reslen), 1))
            goto err;
    } else {
        ctx = EVP_MD_CTX_new();
        reslen = EVP_PKEY_size(pkey);
        res = OPENSSL_malloc(reslen);
        if (!TEST_ptr(ctx)
                || !TEST_ptr(res)
                || !TEST_true(EVP_DigestSignInit(ctx, &pctx, md, NULL, pkey))
                || !TEST_int_ge(EVP_PKEY_CTX_ctrl(pctx, -1, -1,
                                                EVP_PKEY_CTRL_MY_COMMAND, 0, NULL),
                                1)
                || !TEST_int_eq(ctrl_called, 1))
            goto err;

        if (id == EVP_PKEY_ED25519) {
            if (!TEST_true(EVP_DigestSign(ctx, res, &reslen, msg, sizeof(msg))))
                goto err;
        } else {
            if (!TEST_true(EVP_DigestUpdate(ctx, msg, sizeof(msg)))
                    || !TEST_true(EVP_DigestSignFinal(ctx, res, &reslen)))
                goto err;
        }
    }

    testresult = 1;
 err:
    OPENSSL_free(res);
    EVP_MD_CTX_free(ctx);
    if (doderive)
        EVP_PKEY_CTX_free(pctx);
    EVP_PKEY_free(pkey);
    EVP_PKEY_meth_remove(custom_pmeth);
    EVP_PKEY_meth_free(custom_pmeth);
    custom_pmeth = NULL;
    return testresult;
}

static int test_evp_md_cipher_meth(void)
{
    EVP_MD *md = EVP_MD_meth_dup(EVP_sha256());
    EVP_CIPHER *ciph = EVP_CIPHER_meth_dup(EVP_aes_128_cbc());
    int testresult = 0;

    if (!TEST_ptr(md) || !TEST_ptr(ciph))
        goto err;

    testresult = 1;

 err:
    EVP_MD_meth_free(md);
    EVP_CIPHER_meth_free(ciph);

    return testresult;
}

typedef struct {
        int data;
} custom_dgst_ctx;

static int custom_md_init_called = 0;
static int custom_md_cleanup_called = 0;

static int custom_md_init(EVP_MD_CTX *ctx)
{
    custom_dgst_ctx *p = EVP_MD_CTX_md_data(ctx);

    if (p == NULL)
        return 0;

    custom_md_init_called++;
    return 1;
}

static int custom_md_cleanup(EVP_MD_CTX *ctx)
{
    custom_dgst_ctx *p = EVP_MD_CTX_md_data(ctx);

    if (p == NULL)
        /* Nothing to do */
        return 1;

    custom_md_cleanup_called++;
    return 1;
}

static int test_custom_md_meth(void)
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_MD *tmp = NULL;
    char mess[] = "Test Message\n";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_MD_meth_new(nid, NID_undef);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_MD_meth_set_init(tmp, custom_md_init))
            || !TEST_true(EVP_MD_meth_set_cleanup(tmp, custom_md_cleanup))
            || !TEST_true(EVP_MD_meth_set_app_datasize(tmp,
                                                       sizeof(custom_dgst_ctx))))
        goto err;

    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
            || !TEST_true(EVP_DigestFinal_ex(mdctx, md_value, &md_len))
            || !TEST_int_eq(custom_md_init_called, 1)
            || !TEST_int_eq(custom_md_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_MD_CTX_free(mdctx);
    EVP_MD_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    EVP_PKEY *pkey = NULL;
    const unsigned char badcmackey[] = { 0x00, 0x01 };
    const unsigned char cmackey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char ed25519key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_MD_CTX *ctx = NULL;
    unsigned char *mac = NULL;
    size_t maclen = 0;
    int ret;

#  ifdef OPENSSL_NO_CMAC
    /* Skip CMAC tests in a no-cmac build */
    if (tst <= 1)
        return 1;
#  endif

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    switch (tst) {
    case 0:
        pkey = EVP_PKEY_new_CMAC_key(e, cmackey, sizeof(cmackey),
                                     EVP_aes_128_cbc());
        break;
    case 1:
        pkey = EVP_PKEY_new_CMAC_key(e, badcmackey, sizeof(badcmackey),
                                     EVP_aes_128_cbc());
        break;
    case 2:
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, e, ed25519key,
                                            sizeof(ed25519key));
        break;
    default:
        TEST_error("Invalid test case");
        goto err;
    }
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_ptr(ctx = EVP_MD_CTX_new()))
        goto err;

    ret = EVP_DigestSignInit(ctx, NULL, tst == 2 ? NULL : EVP_sha256(), NULL,
                             pkey);
    if (tst == 0) {
        if (!TEST_true(ret))
            goto err;

        if (!TEST_true(EVP_DigestSignUpdate(ctx, msg, sizeof(msg)))
                || !TEST_true(EVP_DigestSignFinal(ctx, NULL, &maclen)))
            goto err;

        if (!TEST_ptr(mac = OPENSSL_malloc(maclen)))
            goto err;

        if (!TEST_true(EVP_DigestSignFinal(ctx, mac, &maclen)))
            goto err;
    } else {
        /* We used a bad key. We expect a failure here */
        if (!TEST_false(ret))
            goto err;
    }

    testresult = 1;
 err:
    EVP_MD_CTX_free(ctx);
    OPENSSL_free(mac);
    EVP_PKEY_free(pkey);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}

static int test_cipher_with_engine(void)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    const unsigned char keyiv[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_CIPHER_CTX *ctx = NULL, *ctx2 = NULL;
    unsigned char buf[AES_BLOCK_SIZE];
    int len = 0;

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_ptr(ctx2 = EVP_CIPHER_CTX_new()))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), e, keyiv, keyiv)))
        goto err;

    /* Copy the ctx, and complete the operation with the new ctx */
    if (!TEST_true(EVP_CIPHER_CTX_copy(ctx2, ctx)))
        goto err;

    if (!TEST_true(EVP_EncryptUpdate(ctx2, buf, &len, msg, sizeof(msg)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx2, buf + len, &len)))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_CTX_free(ctx2);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}
# endif /* OPENSSL_NO_DYNAMIC_ENGINE */
#endif /* OPENSSL_NO_DEPRECATED_3_0 */

static int ecxnids[] = {
    NID_X25519,
    NID_X448,
    NID_ED25519,
    NID_ED448
};

/* Test that creating ECX keys with a short private key fails as expected */
static int test_ecx_short_keys(int tst)
{
    unsigned char ecxkeydata = 1;
    EVP_PKEY *pkey;


    pkey = EVP_PKEY_new_raw_private_key(ecxnids[tst], NULL, &ecxkeydata, 1);
    if (!TEST_ptr_null(pkey)) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    return 1;
}

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
# endif
#endif

    ADD_ALL_TESTS(test_ecx_short_keys, OSSL_NELEM(ecxnids));

    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_MD *tmp = NULL;
    char mess[] = "Test Message\n";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_MD_meth_new(nid, NID_undef);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_MD_meth_set_init(tmp, custom_md_init))
            || !TEST_true(EVP_MD_meth_set_cleanup(tmp, custom_md_cleanup))
            || !TEST_true(EVP_MD_meth_set_app_datasize(tmp,
                                                       sizeof(custom_dgst_ctx))))
        goto err;

    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
            || !TEST_true(EVP_DigestFinal_ex(mdctx, md_value, &md_len))
            || !TEST_int_eq(custom_md_init_called, 1)
            || !TEST_int_eq(custom_md_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_MD_CTX_free(mdctx);
    EVP_MD_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    EVP_PKEY *pkey = NULL;
    const unsigned char badcmackey[] = { 0x00, 0x01 };
    const unsigned char cmackey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char ed25519key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_MD_CTX *ctx = NULL;
    unsigned char *mac = NULL;
    size_t maclen = 0;
    int ret;

#  ifdef OPENSSL_NO_CMAC
    /* Skip CMAC tests in a no-cmac build */
    if (tst <= 1)
        return 1;
#  endif

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    switch (tst) {
    case 0:
        pkey = EVP_PKEY_new_CMAC_key(e, cmackey, sizeof(cmackey),
                                     EVP_aes_128_cbc());
        break;
    case 1:
        pkey = EVP_PKEY_new_CMAC_key(e, badcmackey, sizeof(badcmackey),
                                     EVP_aes_128_cbc());
        break;
    case 2:
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, e, ed25519key,
                                            sizeof(ed25519key));
        break;
    default:
        TEST_error("Invalid test case");
        goto err;
    }
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_ptr(ctx = EVP_MD_CTX_new()))
        goto err;

    ret = EVP_DigestSignInit(ctx, NULL, tst == 2 ? NULL : EVP_sha256(), NULL,
                             pkey);
    if (tst == 0) {
        if (!TEST_true(ret))
            goto err;

        if (!TEST_true(EVP_DigestSignUpdate(ctx, msg, sizeof(msg)))
                || !TEST_true(EVP_DigestSignFinal(ctx, NULL, &maclen)))
            goto err;

        if (!TEST_ptr(mac = OPENSSL_malloc(maclen)))
            goto err;

        if (!TEST_true(EVP_DigestSignFinal(ctx, mac, &maclen)))
            goto err;
    } else {
        /* We used a bad key. We expect a failure here */
        if (!TEST_false(ret))
            goto err;
    }

    testresult = 1;
 err:
    EVP_MD_CTX_free(ctx);
    OPENSSL_free(mac);
    EVP_PKEY_free(pkey);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}

static int test_cipher_with_engine(void)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    const unsigned char keyiv[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_CIPHER_CTX *ctx = NULL, *ctx2 = NULL;
    unsigned char buf[AES_BLOCK_SIZE];
    int len = 0;

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_ptr(ctx2 = EVP_CIPHER_CTX_new()))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), e, keyiv, keyiv)))
        goto err;

    /* Copy the ctx, and complete the operation with the new ctx */
    if (!TEST_true(EVP_CIPHER_CTX_copy(ctx2, ctx)))
        goto err;

    if (!TEST_true(EVP_EncryptUpdate(ctx2, buf, &len, msg, sizeof(msg)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx2, buf + len, &len)))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_CTX_free(ctx2);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}
# endif /* OPENSSL_NO_DYNAMIC_ENGINE */
#endif /* OPENSSL_NO_DEPRECATED_3_0 */

static int ecxnids[] = {
    NID_X25519,
    NID_X448,
    NID_ED25519,
    NID_ED448
};

/* Test that creating ECX keys with a short private key fails as expected */
static int test_ecx_short_keys(int tst)
{
    unsigned char ecxkeydata = 1;
    EVP_PKEY *pkey;


    pkey = EVP_PKEY_new_raw_private_key(ecxnids[tst], NULL, &ecxkeydata, 1);
    if (!TEST_ptr_null(pkey)) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    return 1;
}

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
# endif
#endif

    ADD_ALL_TESTS(test_ecx_short_keys, OSSL_NELEM(ecxnids));

    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_MD *tmp = NULL;
    char mess[] = "Test Message\n";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_MD_meth_new(nid, NID_undef);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_MD_meth_set_init(tmp, custom_md_init))
            || !TEST_true(EVP_MD_meth_set_cleanup(tmp, custom_md_cleanup))
            || !TEST_true(EVP_MD_meth_set_app_datasize(tmp,
                                                       sizeof(custom_dgst_ctx))))
        goto err;

    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
            || !TEST_true(EVP_DigestFinal_ex(mdctx, md_value, &md_len))
            || !TEST_int_eq(custom_md_init_called, 1)
            || !TEST_int_eq(custom_md_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_MD_CTX_free(mdctx);
    EVP_MD_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    EVP_PKEY *pkey = NULL;
    const unsigned char badcmackey[] = { 0x00, 0x01 };
    const unsigned char cmackey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char ed25519key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_MD_CTX *ctx = NULL;
    unsigned char *mac = NULL;
    size_t maclen = 0;
    int ret;

#  ifdef OPENSSL_NO_CMAC
    /* Skip CMAC tests in a no-cmac build */
    if (tst <= 1)
        return 1;
#  endif

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    switch (tst) {
    case 0:
        pkey = EVP_PKEY_new_CMAC_key(e, cmackey, sizeof(cmackey),
                                     EVP_aes_128_cbc());
        break;
    case 1:
        pkey = EVP_PKEY_new_CMAC_key(e, badcmackey, sizeof(badcmackey),
                                     EVP_aes_128_cbc());
        break;
    case 2:
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, e, ed25519key,
                                            sizeof(ed25519key));
        break;
    default:
        TEST_error("Invalid test case");
        goto err;
    }
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_ptr(ctx = EVP_MD_CTX_new()))
        goto err;

    ret = EVP_DigestSignInit(ctx, NULL, tst == 2 ? NULL : EVP_sha256(), NULL,
                             pkey);
    if (tst == 0) {
        if (!TEST_true(ret))
            goto err;

        if (!TEST_true(EVP_DigestSignUpdate(ctx, msg, sizeof(msg)))
                || !TEST_true(EVP_DigestSignFinal(ctx, NULL, &maclen)))
            goto err;

        if (!TEST_ptr(mac = OPENSSL_malloc(maclen)))
            goto err;

        if (!TEST_true(EVP_DigestSignFinal(ctx, mac, &maclen)))
            goto err;
    } else {
        /* We used a bad key. We expect a failure here */
        if (!TEST_false(ret))
            goto err;
    }

    testresult = 1;
 err:
    EVP_MD_CTX_free(ctx);
    OPENSSL_free(mac);
    EVP_PKEY_free(pkey);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}

static int test_cipher_with_engine(void)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    const unsigned char keyiv[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_CIPHER_CTX *ctx = NULL, *ctx2 = NULL;
    unsigned char buf[AES_BLOCK_SIZE];
    int len = 0;

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_ptr(ctx2 = EVP_CIPHER_CTX_new()))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), e, keyiv, keyiv)))
        goto err;

    /* Copy the ctx, and complete the operation with the new ctx */
    if (!TEST_true(EVP_CIPHER_CTX_copy(ctx2, ctx)))
        goto err;

    if (!TEST_true(EVP_EncryptUpdate(ctx2, buf, &len, msg, sizeof(msg)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx2, buf + len, &len)))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_CTX_free(ctx2);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}
# endif /* OPENSSL_NO_DYNAMIC_ENGINE */
#endif /* OPENSSL_NO_DEPRECATED_3_0 */

static int ecxnids[] = {
    NID_X25519,
    NID_X448,
    NID_ED25519,
    NID_ED448
};

/* Test that creating ECX keys with a short private key fails as expected */
static int test_ecx_short_keys(int tst)
{
    unsigned char ecxkeydata = 1;
    EVP_PKEY *pkey;


    pkey = EVP_PKEY_new_raw_private_key(ecxnids[tst], NULL, &ecxkeydata, 1);
    if (!TEST_ptr_null(pkey)) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    return 1;
}

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
# endif
#endif

    ADD_ALL_TESTS(test_ecx_short_keys, OSSL_NELEM(ecxnids));

    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
{
    EVP_MD_CTX *mdctx = NULL;
    EVP_MD *tmp = NULL;
    char mess[] = "Test Message\n";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_MD_meth_new(nid, NID_undef);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_MD_meth_set_init(tmp, custom_md_init))
            || !TEST_true(EVP_MD_meth_set_cleanup(tmp, custom_md_cleanup))
            || !TEST_true(EVP_MD_meth_set_app_datasize(tmp,
                                                       sizeof(custom_dgst_ctx))))
        goto err;

    mdctx = EVP_MD_CTX_new();
    if (!TEST_ptr(mdctx)
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
            || !TEST_true(EVP_DigestFinal_ex(mdctx, md_value, &md_len))
            || !TEST_int_eq(custom_md_init_called, 1)
            || !TEST_int_eq(custom_md_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_MD_CTX_free(mdctx);
    EVP_MD_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    EVP_PKEY *pkey = NULL;
    const unsigned char badcmackey[] = { 0x00, 0x01 };
    const unsigned char cmackey[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char ed25519key[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_MD_CTX *ctx = NULL;
    unsigned char *mac = NULL;
    size_t maclen = 0;
    int ret;

#  ifdef OPENSSL_NO_CMAC
    /* Skip CMAC tests in a no-cmac build */
    if (tst <= 1)
        return 1;
#  endif

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    switch (tst) {
    case 0:
        pkey = EVP_PKEY_new_CMAC_key(e, cmackey, sizeof(cmackey),
                                     EVP_aes_128_cbc());
        break;
    case 1:
        pkey = EVP_PKEY_new_CMAC_key(e, badcmackey, sizeof(badcmackey),
                                     EVP_aes_128_cbc());
        break;
    case 2:
        pkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_ED25519, e, ed25519key,
                                            sizeof(ed25519key));
        break;
    default:
        TEST_error("Invalid test case");
        goto err;
    }
    if (!TEST_ptr(pkey))
        goto err;

    if (!TEST_ptr(ctx = EVP_MD_CTX_new()))
        goto err;

    ret = EVP_DigestSignInit(ctx, NULL, tst == 2 ? NULL : EVP_sha256(), NULL,
                             pkey);
    if (tst == 0) {
        if (!TEST_true(ret))
            goto err;

        if (!TEST_true(EVP_DigestSignUpdate(ctx, msg, sizeof(msg)))
                || !TEST_true(EVP_DigestSignFinal(ctx, NULL, &maclen)))
            goto err;

        if (!TEST_ptr(mac = OPENSSL_malloc(maclen)))
            goto err;

        if (!TEST_true(EVP_DigestSignFinal(ctx, mac, &maclen)))
            goto err;
    } else {
        /* We used a bad key. We expect a failure here */
        if (!TEST_false(ret))
            goto err;
    }

    testresult = 1;
 err:
    EVP_MD_CTX_free(ctx);
    OPENSSL_free(mac);
    EVP_PKEY_free(pkey);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}

static int test_cipher_with_engine(void)
{
    ENGINE *e;
    const char *engine_id = "dasync";
    const unsigned char keyiv[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f
    };
    const unsigned char msg[] = { 0x00, 0x01, 0x02, 0x03 };
    int testresult = 0;
    EVP_CIPHER_CTX *ctx = NULL, *ctx2 = NULL;
    unsigned char buf[AES_BLOCK_SIZE];
    int len = 0;

    if (!TEST_ptr(e = ENGINE_by_id(engine_id)))
        return 0;

    if (!TEST_true(ENGINE_init(e))) {
        ENGINE_free(e);
        return 0;
    }

    if (!TEST_ptr(ctx = EVP_CIPHER_CTX_new())
            || !TEST_ptr(ctx2 = EVP_CIPHER_CTX_new()))
        goto err;

    if (!TEST_true(EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), e, keyiv, keyiv)))
        goto err;

    /* Copy the ctx, and complete the operation with the new ctx */
    if (!TEST_true(EVP_CIPHER_CTX_copy(ctx2, ctx)))
        goto err;

    if (!TEST_true(EVP_EncryptUpdate(ctx2, buf, &len, msg, sizeof(msg)))
            || !TEST_true(EVP_EncryptFinal_ex(ctx2, buf + len, &len)))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ctx);
    EVP_CIPHER_CTX_free(ctx2);
    ENGINE_finish(e);
    ENGINE_free(e);

    return testresult;
}
# endif /* OPENSSL_NO_DYNAMIC_ENGINE */
#endif /* OPENSSL_NO_DEPRECATED_3_0 */

static int ecxnids[] = {
    NID_X25519,
    NID_X448,
    NID_ED25519,
    NID_ED448
};

/* Test that creating ECX keys with a short private key fails as expected */
static int test_ecx_short_keys(int tst)
{
    unsigned char ecxkeydata = 1;
    EVP_PKEY *pkey;


    pkey = EVP_PKEY_new_raw_private_key(ecxnids[tst], NULL, &ecxkeydata, 1);
    if (!TEST_ptr_null(pkey)) {
        EVP_PKEY_free(pkey);
        return 0;
    }
    return 1;
}

typedef enum OPTION_choice {
    OPT_ERR = -1,
    OPT_EOF = 0,
    OPT_CONTEXT,
    OPT_TEST_ENUM
} OPTION_CHOICE;

const OPTIONS *test_get_options(void)
{
    static const OPTIONS options[] = {
        OPT_TEST_OPTIONS_DEFAULT_USAGE,
        { "context", OPT_CONTEXT, '-', "Explicitly use a non-default library context" },
        { NULL }
    };
    return options;
}

int setup_tests(void)
{
    OPTION_CHOICE o;

    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
# endif
#endif

    ADD_ALL_TESTS(test_ecx_short_keys, OSSL_NELEM(ecxnids));

    return 1;
}

void cleanup_tests(void)
{
    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(lgcyprov);
    OSSL_LIB_CTX_free(testctx);
}
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }
        switch (o) {
        case OPT_CONTEXT:
            /* Set up an alternate library context */
            testctx = OSSL_LIB_CTX_new();
            if (!TEST_ptr(testctx))
                return 0;
            /* Swap the libctx to test non-default context only */
            nullprov = OSSL_PROVIDER_load(NULL, "null");
            deflprov = OSSL_PROVIDER_load(testctx, "default");
            lgcyprov = OSSL_PROVIDER_load(testctx, "legacy");
            break;
        case OPT_TEST_CASES:
            break;
        default:
            return 0;
        }
    }

    ADD_TEST(test_EVP_set_default_properties);
    ADD_ALL_TESTS(test_EVP_DigestSignInit, 30);
    ADD_TEST(test_EVP_DigestVerifyInit);
#ifndef OPENSSL_NO_SIPHASH
    ADD_TEST(test_siphash_digestsign);
#endif
    ADD_TEST(test_EVP_Digest);
    ADD_TEST(test_EVP_md_null);
    ADD_ALL_TESTS(test_EVP_PKEY_sign, 3);
    ADD_ALL_TESTS(test_EVP_Enveloped, 2);
    ADD_ALL_TESTS(test_d2i_AutoPrivateKey, OSSL_NELEM(keydata));
    ADD_TEST(test_privatekey_to_pkcs8);
    ADD_TEST(test_EVP_PKCS82PKEY_wrong_tag);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EVP_PKCS82PKEY);
#endif
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
#ifndef OPENSSL_NO_DEPRECATED_3_0
    custom_pmeth = EVP_PKEY_meth_new(0xdefaced, 0);
    if (!TEST_ptr(custom_pmeth))
        return 0;
    EVP_PKEY_meth_set_check(custom_pmeth, pkey_custom_check);
    EVP_PKEY_meth_set_public_check(custom_pmeth, pkey_custom_pub_check);
    EVP_PKEY_meth_set_param_check(custom_pmeth, pkey_custom_param_check);
    if (!TEST_int_eq(EVP_PKEY_meth_add0(custom_pmeth), 1))
        return 0;
#endif
    ADD_ALL_TESTS(test_EVP_PKEY_check, OSSL_NELEM(keycheckdata));
#ifndef OPENSSL_NO_CMAC
    ADD_TEST(test_CMAC_keygen);
#endif
    ADD_TEST(test_HKDF);
    ADD_TEST(test_emptyikm_HKDF);
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_X509_PUBKEY_inplace);
    ADD_TEST(test_X509_PUBKEY_dup);
    ADD_ALL_TESTS(test_invalide_ec_char2_pub_range_decode,
                  OSSL_NELEM(ec_der_pub_keys));
#endif
#ifndef OPENSSL_NO_DSA
    ADD_TEST(test_DSA_get_set_params);
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_TEST(test_DH_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EVP_PKEY_set1_DH);
# endif
#endif
#ifndef OPENSSL_NO_EC
    ADD_TEST(test_EC_priv_pub);
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_TEST(test_EC_priv_only_legacy);
# endif
#endif
    ADD_ALL_TESTS(test_keygen_with_empty_template, 2);
    ADD_ALL_TESTS(test_pkey_ctx_fail_without_provider, 2);

    ADD_TEST(test_rand_agglomeration);
    ADD_ALL_TESTS(test_evp_iv_aes, 12);
#ifndef OPENSSL_NO_DES
    ADD_ALL_TESTS(test_evp_iv_des, 6);
#endif
#ifndef OPENSSL_NO_BF
    ADD_ALL_TESTS(test_evp_bf_default_keylen, 4);
#endif
    ADD_TEST(test_EVP_rsa_pss_with_keygen_bits);
    ADD_TEST(test_EVP_rsa_pss_set_saltlen);
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_ecpub, OSSL_NELEM(ecpub_nids));
#endif

    ADD_TEST(test_names_do_all);

    ADD_ALL_TESTS(test_evp_init_seq, OSSL_NELEM(evp_init_tests));
    ADD_ALL_TESTS(test_evp_reset, OSSL_NELEM(evp_reset_tests));
    ADD_ALL_TESTS(test_gcm_reinit, OSSL_NELEM(gcm_reinit_tests));
    ADD_ALL_TESTS(test_evp_updated_iv, OSSL_NELEM(evp_updated_iv_tests));

#ifndef OPENSSL_NO_DEPRECATED_3_0
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {
#  ifndef OPENSSL_NO_EC
        ADD_ALL_TESTS(test_signatures_with_engine, 3);
#  else
        ADD_ALL_TESTS(test_signatures_with_engine, 2);
#  endif
        ADD_TEST(test_cipher_with_engine);
    }