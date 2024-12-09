    EVP_PKEY *params_and_keypair = NULL;
    BIGNUM *priv = NULL;
    int ret = 0;
    unsigned char *encoded = NULL;

    /*
     * Setup the parameters for our pkey object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_priv), 0))
        goto err;

    /* Positive and negative testcase for EVP_PKEY_get1_encoded_public_key */
    if (!TEST_int_gt(EVP_PKEY_get1_encoded_public_key(params_and_pub, &encoded), 0))
        goto err;
    OPENSSL_free(encoded);
    encoded = NULL;
    if (!TEST_int_eq(EVP_PKEY_get1_encoded_public_key(just_params, &encoded), 0)) {
        OPENSSL_free(encoded);
        encoded = NULL;
        goto err;
    }

    ret = 1;
 err:
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
}
#endif

#if !defined(OPENSSL_NO_SM2)

static int test_EVP_SM2_verify(void)
{
    const char *pubkey =
    return ret;
}

static int test_RSA_OAEP_set_get_params(void)
{
    int ret = 0;
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *key_ctx = NULL;

    if (nullprov != NULL)
        return TEST_skip("Test does not support a non-default library context");

    if (!TEST_ptr(key = load_example_rsa_key())
        || !TEST_ptr(key_ctx = EVP_PKEY_CTX_new_from_pkey(0, key, 0)))
        goto err;

    {
        int padding = RSA_PKCS1_OAEP_PADDING;
        OSSL_PARAM params[4];

        params[0] = OSSL_PARAM_construct_int(OSSL_SIGNATURE_PARAM_PAD_MODE, &padding);
        params[1] = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST,
                                                     OSSL_DIGEST_NAME_SHA2_256, 0);
        params[2] = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST,
                                                     OSSL_DIGEST_NAME_SHA1, 0);
        params[3] = OSSL_PARAM_construct_end();

        if (!TEST_int_gt(EVP_PKEY_encrypt_init_ex(key_ctx, params),0))
            goto err;
    }
    {
        OSSL_PARAM params[3];
        char oaepmd[30] = { '\0' };
        char mgf1md[30] = { '\0' };

        params[0] = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST,
                                                     oaepmd, sizeof(oaepmd));
        params[1] = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST,
                                                     mgf1md, sizeof(mgf1md));
        params[2] = OSSL_PARAM_construct_end();

        if (!TEST_true(EVP_PKEY_CTX_get_params(key_ctx, params)))
            goto err;

        if (!TEST_str_eq(oaepmd, OSSL_DIGEST_NAME_SHA2_256)
            || !TEST_str_eq(mgf1md, OSSL_DIGEST_NAME_SHA1))
            goto err;
    }

    ret = 1;

 err:
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(key_ctx);

    return ret;
}

#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
static int test_decrypt_null_chunks(void)
{
    EVP_CIPHER_CTX* ctx = NULL;
     * library context in this test.
     */
    if (testctx != NULL)
        return TEST_skip("Non-default libctx");

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
    return testresult;
}

typedef struct {
        int data;
} custom_ciph_ctx;

static int custom_ciph_init_called = 0;
static int custom_ciph_cleanup_called = 0;

static int custom_ciph_init(EVP_CIPHER_CTX *ctx, const unsigned char *key,
                            const unsigned char *iv, int enc)
{
    custom_ciph_ctx *p = EVP_CIPHER_CTX_get_cipher_data(ctx);

    if (p == NULL)
        return 0;

    custom_ciph_init_called++;
    return 1;
}

static int custom_ciph_cleanup(EVP_CIPHER_CTX *ctx)
{
    custom_ciph_ctx *p = EVP_CIPHER_CTX_get_cipher_data(ctx);

    if (p == NULL)
        /* Nothing to do */
        return 1;

    custom_ciph_cleanup_called++;
    return 1;
}

static int test_custom_ciph_meth(void)
{
    EVP_CIPHER_CTX *ciphctx = NULL;
    EVP_CIPHER *tmp = NULL;
    int testresult = 0;
    int nid;

    /*
     * We are testing deprecated functions. We don't support a non-default
     * library context in this test.
     */
    if (testctx != NULL)
        return TEST_skip("Non-default libctx");

    custom_ciph_init_called = custom_ciph_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.2", "custom-ciph", "custom-ciph");
    if (!TEST_int_ne(nid, NID_undef))
        goto err;
    tmp = EVP_CIPHER_meth_new(nid, 16, 16);
    if (!TEST_ptr(tmp))
        goto err;

    if (!TEST_true(EVP_CIPHER_meth_set_init(tmp, custom_ciph_init))
            || !TEST_true(EVP_CIPHER_meth_set_flags(tmp, EVP_CIPH_ALWAYS_CALL_INIT))
            || !TEST_true(EVP_CIPHER_meth_set_cleanup(tmp, custom_ciph_cleanup))
            || !TEST_true(EVP_CIPHER_meth_set_impl_ctx_size(tmp,
                                                            sizeof(custom_ciph_ctx))))
        goto err;

    ciphctx = EVP_CIPHER_CTX_new();
    if (!TEST_ptr(ciphctx)
            /*
             * Initing our custom cipher and then initing another cipher
             * should result in the init and cleanup functions of the custom
             * cipher being called.
             */
            || !TEST_true(EVP_CipherInit_ex(ciphctx, tmp, NULL, NULL, NULL, 1))
            || !TEST_true(EVP_CipherInit_ex(ciphctx, EVP_aes_128_cbc(), NULL,
                                            NULL, NULL, 1))
            || !TEST_int_eq(custom_ciph_init_called, 1)
            || !TEST_int_eq(custom_ciph_cleanup_called, 1))
        goto err;

    testresult = 1;
 err:
    EVP_CIPHER_CTX_free(ciphctx);
    EVP_CIPHER_meth_free(tmp);
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
    ADD_TEST(test_RSA_OAEP_set_get_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);
    ADD_TEST(test_custom_ciph_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {