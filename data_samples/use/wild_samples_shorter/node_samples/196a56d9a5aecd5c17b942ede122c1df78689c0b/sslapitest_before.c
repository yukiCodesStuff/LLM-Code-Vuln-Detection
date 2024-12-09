};


static unsigned char serverinfov1[] = {
    0xff, 0xff, /* Dummy extension type */
    0x00, 0x01, /* Extension length is 1 byte */
    0xff        /* Dummy extension data */
};

static unsigned char serverinfov2[] = {
    0x00, 0x00, 0x00,
    (unsigned char)(SSL_EXT_CLIENT_HELLO & 0xff), /* Dummy context - 4 bytes */
    0xff, 0xff, /* Dummy extension type */
    0x00, 0x01, /* Extension length is 1 byte */
    0xff        /* Dummy extension data */
};

static int hostname_cb(SSL *s, int *al, void *arg)
{
    const char *hostname = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);

                goto end;
        }
    }

    testresult = 1;

 end:
    return testresult;
}

/*
 * Test loading of serverinfo data in various formats. test_sslmessages actually
 * tests to make sure the extensions appear in the handshake
 */
static int test_serverinfo(int tst)
{
    unsigned int version;
    unsigned char *sibuf;
    size_t sibuflen;
    int ret, expected, testresult = 0;
    SSL_CTX *ctx;

    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_method());
    if (!TEST_ptr(ctx))
        goto end;

    if ((tst & 0x01) == 0x01)
        version = SSL_SERVERINFOV2;
    else
        version = SSL_SERVERINFOV1;

    if ((tst & 0x02) == 0x02) {
        sibuf = serverinfov2;
        sibuflen = sizeof(serverinfov2);
        expected = (version == SSL_SERVERINFOV2);
    } else {
        sibuf = serverinfov1;
        sibuflen = sizeof(serverinfov1);
        expected = (version == SSL_SERVERINFOV1);
    }

    if ((tst & 0x04) == 0x04) {
        ret = SSL_CTX_use_serverinfo_ex(ctx, version, sibuf, sibuflen);
    } else {
        ret = SSL_CTX_use_serverinfo(ctx, sibuf, sibuflen);

        /*
         * The version variable is irrelevant in this case - it's what is in the
         * buffer that matters
         */
        if ((tst & 0x02) == 0x02)
            expected = 0;
        else
            expected = 1;
    }

    if (!TEST_true(ret == expected))
        goto end;

    testresult = 1;

 end:
    SSL_CTX_free(ctx);

    return testresult;
}

/*
 * Test that SSL_export_keying_material() produces expected results. There are
 * no test vectors so all we do is test that both sides of the communication
{
    const unsigned char tick_aes_key[16] = "0123456789abcdef";
    const unsigned char tick_hmac_key[16] = "0123456789abcdef";
    EVP_CIPHER *aes128cbc = EVP_CIPHER_fetch(libctx, "AES-128-CBC", NULL);
    EVP_MD *sha256 = EVP_MD_fetch(libctx, "SHA-256", NULL);
    int ret;

    tick_key_cb_called = 1;
    memset(iv, 0, AES_BLOCK_SIZE);
    memset(key_name, 0, 16);
    if (aes128cbc == NULL
            || sha256 == NULL
    const unsigned char tick_aes_key[16] = "0123456789abcdef";
    unsigned char tick_hmac_key[16] = "0123456789abcdef";
    OSSL_PARAM params[2];
    EVP_CIPHER *aes128cbc = EVP_CIPHER_fetch(libctx, "AES-128-CBC", NULL);
    int ret;

    tick_key_cb_called = 1;
    memset(iv, 0, AES_BLOCK_SIZE);
    memset(key_name, 0, 16);
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
                                                 "SHA256", 0);
 * Test 9: TLSv1.3, old ticket key callback, ticket, no renewal
 * Test 10: TLSv1.2, old ticket key callback, ticket, renewal
 * Test 11: TLSv1.3, old ticket key callback, ticket, renewal
 * Test 12: TLSv1.2, ticket key callback, ticket, no renewal
 * Test 13: TLSv1.3, ticket key callback, ticket, no renewal
 * Test 14: TLSv1.2, ticket key callback, ticket, renewal
 * Test 15: TLSv1.3, ticket key callback, ticket, renewal
 */
static int test_ticket_callbacks(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
        return 1;
#endif
#ifdef OPENSSL_NO_DEPRECATED_3_0
    if (tst >= 8 && tst <= 11)
        return 1;
#endif

    gen_tick_called = dec_tick_called = tick_key_cb_called = 0;

    /* Which tests the ticket key callback should request renewal for */
    if (tst == 10 || tst == 11 || tst == 14 || tst == 15)
        tick_key_renew = 1;
    else
        tick_key_renew = 0;

    /* Which tests the decrypt ticket callback should request renewal for */
                                                 NULL)))
        goto end;

    if (tst >= 12) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_evp_cb(sctx, tick_key_evp_cb)))
            goto end;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    } else if (tst >= 8) {
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
            || !TEST_int_eq(dec_tick_called, 1))
        goto end;

    testresult = 1;

     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(NULL, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_serverinfo, 8);
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 16);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_TEST(test_set_verify_cert_store_ssl);
    ADD_ALL_TESTS(test_session_timeout, 1);
    ADD_TEST(test_load_dhfile);
#ifndef OPENSSL_NO_QUIC
    ADD_ALL_TESTS(test_quic_api, 9);
# ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_quic_early_data, 3);