{
    int ret;
    EVP_CIPHER_CTX *ctx = NULL;
    const EVP_CIPHER *cipher;

    ret = TEST_ptr(cipher = EVP_aes_192_gcm())
          && TEST_ptr(ctx = EVP_CIPHER_CTX_new())
          && TEST_true(EVP_EncryptInit_ex(ctx, cipher, NULL, NULL, NULL))
          && TEST_int_le(EVP_CIPHER_CTX_set_key_length(ctx, 2), 0);
    EVP_CIPHER_CTX_free(ctx);
    return ret;
}

static int ivgen_test(void)
{
    unsigned char iv_gen[16];
    unsigned char tag[32];
    unsigned char ct[32];
    int ctlen = 0, taglen = 0;

    return do_encrypt(iv_gen, ct, &ctlen, tag, &taglen)
           && do_decrypt(iv_gen, ct, ctlen, tag, taglen);
}

int setup_tests(void)
{
    ADD_TEST(kat_test);
    ADD_TEST(badkeylen_test);
    ADD_TEST(ivgen_test);
    return 1;
}
{
    unsigned char iv_gen[16];
    unsigned char tag[32];
    unsigned char ct[32];
    int ctlen = 0, taglen = 0;

    return do_encrypt(iv_gen, ct, &ctlen, tag, &taglen)
           && do_decrypt(iv_gen, ct, ctlen, tag, taglen);
}

int setup_tests(void)
{
    ADD_TEST(kat_test);
    ADD_TEST(badkeylen_test);
    ADD_TEST(ivgen_test);
    return 1;
}