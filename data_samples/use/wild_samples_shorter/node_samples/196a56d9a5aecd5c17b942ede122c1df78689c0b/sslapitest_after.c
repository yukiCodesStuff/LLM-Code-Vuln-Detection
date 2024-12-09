};


static int hostname_cb(SSL *s, int *al, void *arg)
{
    const char *hostname = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);

                goto end;
        }
    }
    /*
     * Make a small cache, force out all other sessions but
     * sess2, try to add sess1, which should succeed. Then
     * make sure it's there by checking the owners. Despite
     * the timeouts, sess1 should have kicked out sess2
     */

    /* Make sess1 expire before sess2 */
    if (!TEST_long_gt(SSL_SESSION_set_time(sess1, 1000), 0)
            || !TEST_long_gt(SSL_SESSION_set_timeout(sess1, 1000), 0)
            || !TEST_long_gt(SSL_SESSION_set_time(sess2, 2000), 0)
            || !TEST_long_gt(SSL_SESSION_set_timeout(sess2, 2000), 0))
        goto end;

    if (!TEST_long_ne(SSL_CTX_sess_set_cache_size(sctx, 1), 0))
        goto end;

    /* Don't care about results - cache should only be sess2 at end */
    SSL_CTX_add_session(sctx, sess1);
    SSL_CTX_add_session(sctx, sess2);

    /* Now add sess1, and make sure it remains, despite timeout */
    if (!TEST_true(SSL_CTX_add_session(sctx, sess1))
            || !TEST_ptr(sess1->owner)
            || !TEST_ptr_null(sess2->owner))
        goto end;

    testresult = 1;

 end:
    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) && !defined(OSSL_NO_USABLE_TLS1_3)

#define  SYNTHV1CONTEXT     (SSL_EXT_TLS1_2_AND_BELOW_ONLY \
                             | SSL_EXT_CLIENT_HELLO \
                             | SSL_EXT_TLS1_2_SERVER_HELLO \
                             | SSL_EXT_IGNORE_ON_RESUMPTION)

#define TLS13CONTEXT (SSL_EXT_TLS1_3_CERTIFICATE \
                      | SSL_EXT_TLS1_2_SERVER_HELLO \
                      | SSL_EXT_CLIENT_HELLO)

#define SERVERINFO_CUSTOM                                 \
    0x00, (char)TLSEXT_TYPE_signed_certificate_timestamp, \
    0x00, 0x03,                                           \
    0x04, 0x05, 0x06                                      \

static const unsigned char serverinfo_custom_tls13[] = {
    0x00, 0x00, (TLS13CONTEXT >> 8) & 0xff, TLS13CONTEXT & 0xff,
    SERVERINFO_CUSTOM
};
static const unsigned char serverinfo_custom_v2[] = {
    0x00, 0x00, (SYNTHV1CONTEXT >> 8) & 0xff,  SYNTHV1CONTEXT & 0xff,
    SERVERINFO_CUSTOM
};
static const unsigned char serverinfo_custom_v1[] = {
    SERVERINFO_CUSTOM
};
static const size_t serverinfo_custom_tls13_len = sizeof(serverinfo_custom_tls13);
static const size_t serverinfo_custom_v2_len = sizeof(serverinfo_custom_v2);
static const size_t serverinfo_custom_v1_len = sizeof(serverinfo_custom_v1);

static int serverinfo_custom_parse_cb(SSL *s, unsigned int ext_type,
                                      unsigned int context,
                                      const unsigned char *in,
                                      size_t inlen, X509 *x,
                                      size_t chainidx, int *al,
                                      void *parse_arg)
{
    const size_t len = serverinfo_custom_v1_len;
    const unsigned char *si = &serverinfo_custom_v1[len - 3];
    int *p_cb_result = (int*)parse_arg;
    *p_cb_result = TEST_mem_eq(in, inlen, si, 3);
    return 1;
}

static int test_serverinfo_custom(const int idx)
{
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int cb_result = 0;

    /*
     * Following variables are set in the switch statement
     *  according to the test iteration.
     * Default values do not make much sense: test would fail with them.
     */
    int serverinfo_version = 0;
    int protocol_version = 0;
    unsigned int extension_context = 0;
    const unsigned char *si = NULL;
    size_t si_len = 0;

    const int call_use_serverinfo_ex = idx > 0;
    switch (idx) {
    case 0: /* FALLTHROUGH */
    case 1:
        serverinfo_version = SSL_SERVERINFOV1;
        protocol_version = TLS1_2_VERSION;
        extension_context = SYNTHV1CONTEXT;
        si = serverinfo_custom_v1;
        si_len = serverinfo_custom_v1_len;
        break;
    case 2:
        serverinfo_version = SSL_SERVERINFOV2;
        protocol_version = TLS1_2_VERSION;
        extension_context = SYNTHV1CONTEXT;
        si = serverinfo_custom_v2;
        si_len = serverinfo_custom_v2_len;
        break;
    case 3:
        serverinfo_version = SSL_SERVERINFOV2;
        protocol_version = TLS1_3_VERSION;
        extension_context = TLS13CONTEXT;
        si = serverinfo_custom_tls13;
        si_len = serverinfo_custom_tls13_len;
        break;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx,
                                       TLS_method(),
                                       TLS_method(),
                                       protocol_version,
                                       protocol_version,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (call_use_serverinfo_ex) {
        if (!TEST_true(SSL_CTX_use_serverinfo_ex(sctx, serverinfo_version,
                                                 si, si_len)))
            goto end;
    } else {
        if (!TEST_true(SSL_CTX_use_serverinfo(sctx, si, si_len)))
            goto end;
    }

    if (!TEST_true(SSL_CTX_add_custom_ext(cctx, TLSEXT_TYPE_signed_certificate_timestamp,
                                          extension_context,
                                          NULL, NULL, NULL,
                                          serverinfo_custom_parse_cb,
                                          &cb_result))
        || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                         NULL, NULL))
        || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                            SSL_ERROR_NONE))
        || !TEST_int_eq(SSL_do_handshake(clientssl), 1))
        goto end;

    if (!TEST_true(cb_result))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif

/*
 * Test that SSL_export_keying_material() produces expected results. There are
 * no test vectors so all we do is test that both sides of the communication
{
    const unsigned char tick_aes_key[16] = "0123456789abcdef";
    const unsigned char tick_hmac_key[16] = "0123456789abcdef";
    EVP_CIPHER *aes128cbc;
    EVP_MD *sha256;
    int ret;

    tick_key_cb_called = 1;

    if (tick_key_renew == -1)
        return 0;

    aes128cbc = EVP_CIPHER_fetch(libctx, "AES-128-CBC", NULL);
    if (!TEST_ptr(aes128cbc))
        return 0;
    sha256 = EVP_MD_fetch(libctx, "SHA-256", NULL);
    if (!TEST_ptr(sha256)) {
        EVP_CIPHER_free(aes128cbc);
        return 0;
    }

    memset(iv, 0, AES_BLOCK_SIZE);
    memset(key_name, 0, 16);
    if (aes128cbc == NULL
            || sha256 == NULL
    const unsigned char tick_aes_key[16] = "0123456789abcdef";
    unsigned char tick_hmac_key[16] = "0123456789abcdef";
    OSSL_PARAM params[2];
    EVP_CIPHER *aes128cbc;
    int ret;

    tick_key_cb_called = 1;

    if (tick_key_renew == -1)
        return 0;

    aes128cbc = EVP_CIPHER_fetch(libctx, "AES-128-CBC", NULL);
    if (!TEST_ptr(aes128cbc))
        return 0;

    memset(iv, 0, AES_BLOCK_SIZE);
    memset(key_name, 0, 16);
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_MAC_PARAM_DIGEST,
                                                 "SHA256", 0);
 * Test 9: TLSv1.3, old ticket key callback, ticket, no renewal
 * Test 10: TLSv1.2, old ticket key callback, ticket, renewal
 * Test 11: TLSv1.3, old ticket key callback, ticket, renewal
 * Test 12: TLSv1.2, old ticket key callback, no ticket
 * Test 13: TLSv1.3, old ticket key callback, no ticket
 * Test 14: TLSv1.2, ticket key callback, ticket, no renewal
 * Test 15: TLSv1.3, ticket key callback, ticket, no renewal
 * Test 16: TLSv1.2, ticket key callback, ticket, renewal
 * Test 17: TLSv1.3, ticket key callback, ticket, renewal
 * Test 18: TLSv1.2, ticket key callback, no ticket
 * Test 19: TLSv1.3, ticket key callback, no ticket
 */
static int test_ticket_callbacks(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
        return 1;
#endif
#ifdef OPENSSL_NO_DEPRECATED_3_0
    if (tst >= 8 && tst <= 13)
        return 1;
#endif

    gen_tick_called = dec_tick_called = tick_key_cb_called = 0;

    /* Which tests the ticket key callback should request renewal for */
    
    if (tst == 10 || tst == 11 || tst == 16 || tst == 17)
        tick_key_renew = 1;
    else if (tst == 12 || tst == 13 || tst == 18 || tst == 19)
        tick_key_renew = -1; /* abort sending the ticket/0-length ticket */
    else
        tick_key_renew = 0;

    /* Which tests the decrypt ticket callback should request renewal for */
                                                 NULL)))
        goto end;

    if (tst >= 14) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_evp_cb(sctx, tick_key_evp_cb)))
            goto end;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    } else if (tst >= 8) {
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
            || tick_key_renew == -1) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_TEST(test_set_verify_cert_store_ssl);
    ADD_ALL_TESTS(test_session_timeout, 1);
    ADD_TEST(test_load_dhfile);
#if !defined(OPENSSL_NO_TLS1_2) && !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_serverinfo_custom, 4);
#endif
#ifndef OPENSSL_NO_QUIC
    ADD_ALL_TESTS(test_quic_api, 9);
# ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_quic_early_data, 3);