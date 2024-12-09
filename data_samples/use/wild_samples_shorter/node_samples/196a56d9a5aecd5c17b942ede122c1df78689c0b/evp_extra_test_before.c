    EVP_PKEY *params_and_keypair = NULL;
    BIGNUM *priv = NULL;
    int ret = 0;

    /*
     * Setup the parameters for our pkey object. For our purposes they don't
     * have to actually be *valid* parameters. We just need to set something.
        || !TEST_int_gt(EVP_PKEY_eq(params_and_keypair, params_and_priv), 0))
        goto err;

    ret = 1;
 err:
    OSSL_PARAM_free(params);
    OSSL_PARAM_BLD_free(bld);
}
#endif

#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)

static int test_EVP_SM2_verify(void)
{
    const char *pubkey =
    return ret;
}

#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
static int test_decrypt_null_chunks(void)
{
    EVP_CIPHER_CTX* ctx = NULL;
     * library context in this test.
     */
    if (testctx != NULL)
        return 1;

    custom_md_init_called = custom_md_cleanup_called = 0;

    nid = OBJ_create("1.3.6.1.4.1.16604.998866.1", "custom-md", "custom-md");
               /*
                * Initing our custom md and then initing another md should
                * result in the init and cleanup functions of the custom md
                * from being called.
                */
            || !TEST_true(EVP_DigestInit_ex(mdctx, tmp, NULL))
            || !TEST_true(EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
            || !TEST_true(EVP_DigestUpdate(mdctx, mess, strlen(mess)))
    return testresult;
}

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
/* Test we can create a signature keys with an associated ENGINE */
static int test_signatures_with_engine(int tst)
{
#ifndef OPENSSL_NO_EC
    ADD_ALL_TESTS(test_EC_keygen_with_enc, OSSL_NELEM(ec_encodings));
#endif
#if !defined(OPENSSL_NO_SM2) && !defined(FIPS_MODULE)
    ADD_TEST(test_EVP_SM2);
    ADD_TEST(test_EVP_SM2_verify);
#endif
    ADD_ALL_TESTS(test_set_get_raw_keys, OSSL_NELEM(keys));
    ADD_TEST(test_DSA_priv_pub);
#endif
    ADD_TEST(test_RSA_get_set_params);
#if !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    ADD_TEST(test_decrypt_null_chunks);
#endif
#ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_custom_pmeth, 12);
    ADD_TEST(test_evp_md_cipher_meth);
    ADD_TEST(test_custom_md_meth);

# ifndef OPENSSL_NO_DYNAMIC_ENGINE
    /* Tests only support the default libctx */
    if (testctx == NULL) {