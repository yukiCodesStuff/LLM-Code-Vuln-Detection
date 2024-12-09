struct sslapitest_log_counts {
    unsigned int rsa_key_exchange_count;
    unsigned int master_secret_count;
    unsigned int client_early_secret_count;
    unsigned int client_handshake_secret_count;
    unsigned int server_handshake_secret_count;
    unsigned int client_application_secret_count;
    unsigned int server_application_secret_count;
    unsigned int early_exporter_secret_count;
    unsigned int exporter_secret_count;
};


static int hostname_cb(SSL *s, int *al, void *arg)
{
    const char *hostname = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);

    if (hostname != NULL && (strcmp(hostname, "goodhost") == 0
                             || strcmp(hostname, "altgoodhost") == 0))
        return  SSL_TLSEXT_ERR_OK;

    return SSL_TLSEXT_ERR_NOACK;
}
        } else {
            if (!TEST_int_eq(new_called, 0)
                    || !TEST_int_eq(get_called, 1))
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
    SSL_free(serverssl1);
    SSL_free(clientssl1);
    SSL_free(serverssl2);
    SSL_free(clientssl2);
# ifndef OPENSSL_NO_TLS1_1
    SSL_free(serverssl3);
    SSL_free(clientssl3);
# endif
    SSL_SESSION_free(sess1);
    SSL_SESSION_free(sess2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* !defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2) */

static int test_session_with_only_int_cache(void)
{
#ifndef OSSL_NO_USABLE_TLS1_3
    if (!execute_test_session(TLS1_3_VERSION, 1, 0, 0))
        return 0;
#endif

#ifndef OPENSSL_NO_TLS1_2
    return execute_test_session(TLS1_2_VERSION, 1, 0, 0);
#else
    return 1;
#endif
}

static int test_session_with_only_ext_cache(void)
{
#ifndef OSSL_NO_USABLE_TLS1_3
    if (!execute_test_session(TLS1_3_VERSION, 0, 1, 0))
        return 0;
#endif

#ifndef OPENSSL_NO_TLS1_2
    return execute_test_session(TLS1_2_VERSION, 0, 1, 0);
#else
    return 1;
#endif
}

static int test_session_with_both_cache(void)
{
#ifndef OSSL_NO_USABLE_TLS1_3
    if (!execute_test_session(TLS1_3_VERSION, 1, 1, 0))
        return 0;
#endif

#ifndef OPENSSL_NO_TLS1_2
    return execute_test_session(TLS1_2_VERSION, 1, 1, 0);
#else
    return 1;
#endif
}

static int test_session_wo_ca_names(void)
{
#ifndef OSSL_NO_USABLE_TLS1_3
    if (!execute_test_session(TLS1_3_VERSION, 1, 0, SSL_OP_DISABLE_TLSEXT_CA_NAMES))
        return 0;
#endif

#ifndef OPENSSL_NO_TLS1_2
    return execute_test_session(TLS1_2_VERSION, 1, 0, SSL_OP_DISABLE_TLSEXT_CA_NAMES);
#else
    return 1;
#endif
}


#ifndef OSSL_NO_USABLE_TLS1_3
static SSL_SESSION *sesscache[6];
static int do_cache;

static int new_cachesession_cb(SSL *ssl, SSL_SESSION *sess)
{
    if (do_cache) {
        sesscache[new_called] = sess;
    } else {
        /* We don't need the reference to the session, so free it */
        SSL_SESSION_free(sess);
    }
    new_called++;

    return 1;
}

static int post_handshake_verify(SSL *sssl, SSL *cssl)
{
    SSL_set_verify(sssl, SSL_VERIFY_PEER, NULL);
    if (!TEST_true(SSL_verify_client_post_handshake(sssl)))
        return 0;

    /* Start handshake on the server and client */
    if (!TEST_int_eq(SSL_do_handshake(sssl), 1)
            || !TEST_int_le(SSL_read(cssl, NULL, 0), 0)
            || !TEST_int_le(SSL_read(sssl, NULL, 0), 0)
            || !TEST_true(create_ssl_connection(sssl, cssl,
                                                SSL_ERROR_NONE)))
        return 0;

    return 1;
}

static int setup_ticket_test(int stateful, int idx, SSL_CTX **sctx,
                             SSL_CTX **cctx)
{
    int sess_id_ctx = 1;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_VERSION, 0,
                                       sctx, cctx, cert, privkey))
            || !TEST_true(SSL_CTX_set_num_tickets(*sctx, idx))
            || !TEST_true(SSL_CTX_set_session_id_context(*sctx,
                                                         (void *)&sess_id_ctx,
                                                         sizeof(sess_id_ctx))))
        return 0;

    if (stateful)
        SSL_CTX_set_options(*sctx, SSL_OP_NO_TICKET);

    SSL_CTX_set_session_cache_mode(*cctx, SSL_SESS_CACHE_CLIENT
                                          | SSL_SESS_CACHE_NO_INTERNAL_STORE);
    SSL_CTX_sess_set_new_cb(*cctx, new_cachesession_cb);

    return 1;
}

static int check_resumption(int idx, SSL_CTX *sctx, SSL_CTX *cctx, int succ)
{
    SSL *serverssl = NULL, *clientssl = NULL;
    int i;

    /* Test that we can resume with all the tickets we got given */
    for (i = 0; i < idx * 2; i++) {
        new_called = 0;
        if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                              &clientssl, NULL, NULL))
                || !TEST_true(SSL_set_session(clientssl, sesscache[i])))
            goto end;

        SSL_set_post_handshake_auth(clientssl, 1);

        if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                                    SSL_ERROR_NONE)))
            goto end;

        /*
         * Following a successful resumption we only get 1 ticket. After a
         * failed one we should get idx tickets.
         */
        if (succ) {
            if (!TEST_true(SSL_session_reused(clientssl))
                    || !TEST_int_eq(new_called, 1))
                goto end;
        } else {
            if (!TEST_false(SSL_session_reused(clientssl))
                    || !TEST_int_eq(new_called, idx))
                goto end;
        }

        new_called = 0;
        /* After a post-handshake authentication we should get 1 new ticket */
        if (succ
                && (!post_handshake_verify(serverssl, clientssl)
                    || !TEST_int_eq(new_called, 1)))
            goto end;

        SSL_shutdown(clientssl);
        SSL_shutdown(serverssl);
        SSL_free(serverssl);
        SSL_free(clientssl);
        serverssl = clientssl = NULL;
        SSL_SESSION_free(sesscache[i]);
        sesscache[i] = NULL;
    }
    } else {
        /*
         * No Certificate message extensions in the resumption handshake,
         * 2 NewSessionTickets in the initial handshake, 1 in the resumption
         */
        if (clntaddnewcb != 2
                || clntparsenewcb != 8
                || srvaddnewcb != 8
                || srvparsenewcb != 2)
            goto end;
    }

    testresult = 1;

end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
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
 * produce the same results for different protocol versions.
 */
#define SMALL_LABEL_LEN 10
#define LONG_LABEL_LEN  249
static int test_export_key_mat(int tst)
{
    int testresult = 0;
    SSL_CTX *cctx = NULL, *sctx = NULL, *sctx2 = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    const char label[LONG_LABEL_LEN + 1] = "test label";
    const unsigned char context[] = "context";
    const unsigned char *emptycontext = NULL;
    unsigned char ckeymat1[80], ckeymat2[80], ckeymat3[80];
    unsigned char skeymat1[80], skeymat2[80], skeymat3[80];
    size_t labellen;
    const int protocols[] = {
        TLS1_VERSION,
        TLS1_1_VERSION,
        TLS1_2_VERSION,
        TLS1_3_VERSION,
        TLS1_3_VERSION,
        TLS1_3_VERSION
    };

#ifdef OPENSSL_NO_TLS1
    if (tst == 0)
        return 1;
#endif
#ifdef OPENSSL_NO_TLS1_1
    if (tst == 1)
        return 1;
#endif
    if (is_fips && (tst == 0 || tst == 1))
        return 1;
#ifdef OPENSSL_NO_TLS1_2
    if (tst == 2)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 3)
        return 1;
#endif
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    OPENSSL_assert(tst >= 0 && (size_t)tst < OSSL_NELEM(protocols));
    SSL_CTX_set_max_proto_version(cctx, protocols[tst]);
    SSL_CTX_set_min_proto_version(cctx, protocols[tst]);
    if ((protocols[tst] < TLS1_2_VERSION) &&
        (!SSL_CTX_set_cipher_list(cctx, "DEFAULT:@SECLEVEL=0")
        || !SSL_CTX_set_cipher_list(sctx, "DEFAULT:@SECLEVEL=0")))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL)))
        goto end;

    /*
     * Premature call of SSL_export_keying_material should just fail.
     */
    if (!TEST_int_le(SSL_export_keying_material(clientssl, ckeymat1,
                                                sizeof(ckeymat1), label,
                                                SMALL_LABEL_LEN + 1, context,
                                                sizeof(context) - 1, 1), 0))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                         SSL_ERROR_NONE)))
        goto end;

    if (tst == 5) {
        /*
         * TLSv1.3 imposes a maximum label len of 249 bytes. Check we fail if we
         * go over that.
         */
        if (!TEST_int_le(SSL_export_keying_material(clientssl, ckeymat1,
                                                    sizeof(ckeymat1), label,
                                                    LONG_LABEL_LEN + 1, context,
                                                    sizeof(context) - 1, 1), 0))
            goto end;

        testresult = 1;
        goto end;
    } else if (tst == 4) {
        labellen = LONG_LABEL_LEN;
    } else {
        labellen = SMALL_LABEL_LEN;
    }

    if (!TEST_int_eq(SSL_export_keying_material(clientssl, ckeymat1,
                                                sizeof(ckeymat1), label,
                                                labellen, context,
                                                sizeof(context) - 1, 1), 1)
            || !TEST_int_eq(SSL_export_keying_material(clientssl, ckeymat2,
                                                       sizeof(ckeymat2), label,
                                                       labellen,
                                                       emptycontext,
                                                       0, 1), 1)
            || !TEST_int_eq(SSL_export_keying_material(clientssl, ckeymat3,
                                                       sizeof(ckeymat3), label,
                                                       labellen,
                                                       NULL, 0, 0), 1)
            || !TEST_int_eq(SSL_export_keying_material(serverssl, skeymat1,
                                                       sizeof(skeymat1), label,
                                                       labellen,
                                                       context,
                                                       sizeof(context) -1, 1),
                            1)
            || !TEST_int_eq(SSL_export_keying_material(serverssl, skeymat2,
                                                       sizeof(skeymat2), label,
                                                       labellen,
                                                       emptycontext,
                                                       0, 1), 1)
            || !TEST_int_eq(SSL_export_keying_material(serverssl, skeymat3,
                                                       sizeof(skeymat3), label,
                                                       labellen,
                                                       NULL, 0, 0), 1)
               /*
                * Check that both sides created the same key material with the
                * same context.
                */
            || !TEST_mem_eq(ckeymat1, sizeof(ckeymat1), skeymat1,
                            sizeof(skeymat1))
               /*
                * Check that both sides created the same key material with an
                * empty context.
                */
            || !TEST_mem_eq(ckeymat2, sizeof(ckeymat2), skeymat2,
                            sizeof(skeymat2))
               /*
                * Check that both sides created the same key material without a
                * context.
                */
            || !TEST_mem_eq(ckeymat3, sizeof(ckeymat3), skeymat3,
                            sizeof(skeymat3))
               /* Different contexts should produce different results */
            || !TEST_mem_ne(ckeymat1, sizeof(ckeymat1), ckeymat2,
                            sizeof(ckeymat2)))
        goto end;

    /*
     * Check that an empty context and no context produce different results in
     * protocols less than TLSv1.3. In TLSv1.3 they should be the same.
     */
    if ((tst < 3 && !TEST_mem_ne(ckeymat2, sizeof(ckeymat2), ckeymat3,
                                  sizeof(ckeymat3)))
            || (tst >= 3 && !TEST_mem_eq(ckeymat2, sizeof(ckeymat2), ckeymat3,
                                         sizeof(ckeymat3))))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Test that SSL_export_keying_material_early() produces expected
 * results. There are no test vectors so all we do is test that both
 * sides of the communication produce the same results for different
 * protocol versions.
 */
static int test_export_key_mat_early(int idx)
{
    static const char label[] = "test label";
    static const unsigned char context[] = "context";
    int testresult = 0;
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *sess = NULL;
    const unsigned char *emptycontext = NULL;
    unsigned char ckeymat1[80], ckeymat2[80];
    unsigned char skeymat1[80], skeymat2[80];
    unsigned char buf[1];
    size_t readbytes, written;

    if (!TEST_true(setupearly_data_test(&cctx, &sctx, &clientssl, &serverssl,
                                        &sess, idx)))
        goto end;

    /* Here writing 0 length early data is enough. */
    if (!TEST_true(SSL_write_early_data(clientssl, NULL, 0, &written))
            || !TEST_int_eq(SSL_read_early_data(serverssl, buf, sizeof(buf),
                                                &readbytes),
                            SSL_READ_EARLY_DATA_ERROR)
            || !TEST_int_eq(SSL_get_early_data_status(serverssl),
                            SSL_EARLY_DATA_ACCEPTED))
        goto end;

    if (!TEST_int_eq(SSL_export_keying_material_early(
                     clientssl, ckeymat1, sizeof(ckeymat1), label,
                     sizeof(label) - 1, context, sizeof(context) - 1), 1)
            || !TEST_int_eq(SSL_export_keying_material_early(
                            clientssl, ckeymat2, sizeof(ckeymat2), label,
                            sizeof(label) - 1, emptycontext, 0), 1)
            || !TEST_int_eq(SSL_export_keying_material_early(
                            serverssl, skeymat1, sizeof(skeymat1), label,
                            sizeof(label) - 1, context, sizeof(context) - 1), 1)
            || !TEST_int_eq(SSL_export_keying_material_early(
                            serverssl, skeymat2, sizeof(skeymat2), label,
                            sizeof(label) - 1, emptycontext, 0), 1)
               /*
                * Check that both sides created the same key material with the
                * same context.
                */
            || !TEST_mem_eq(ckeymat1, sizeof(ckeymat1), skeymat1,
                            sizeof(skeymat1))
               /*
                * Check that both sides created the same key material with an
                * empty context.
                */
            || !TEST_mem_eq(ckeymat2, sizeof(ckeymat2), skeymat2,
                            sizeof(skeymat2))
               /* Different contexts should produce different results */
            || !TEST_mem_ne(ckeymat1, sizeof(ckeymat1), ckeymat2,
                            sizeof(ckeymat2)))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_SESSION_free(clientpsk);
    SSL_SESSION_free(serverpsk);
    clientpsk = serverpsk = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#define NUM_KEY_UPDATE_MESSAGES 40
/*
 * Test KeyUpdate.
 */
static int test_key_update(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0, i, j;
    char buf[20];
    static char *mess = "A test message";

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION,
                                       0,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    for (j = 0; j < 2; j++) {
        /* Send lots of KeyUpdate messages */
        for (i = 0; i < NUM_KEY_UPDATE_MESSAGES; i++) {
            if (!TEST_true(SSL_key_update(clientssl,
                                          (j == 0)
                                          ? SSL_KEY_UPDATE_NOT_REQUESTED
                                          : SSL_KEY_UPDATE_REQUESTED))
                    || !TEST_true(SSL_do_handshake(clientssl)))
                goto end;
        }

        /* Check that sending and receiving app data is ok */
        if (!TEST_int_eq(SSL_write(clientssl, mess, strlen(mess)), strlen(mess))
                || !TEST_int_eq(SSL_read(serverssl, buf, sizeof(buf)),
                                         strlen(mess)))
            goto end;

        if (!TEST_int_eq(SSL_write(serverssl, mess, strlen(mess)), strlen(mess))
                || !TEST_int_eq(SSL_read(clientssl, buf, sizeof(buf)),
                                         strlen(mess)))
            goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test we can handle a KeyUpdate (update requested) message while
 * write data is pending in peer.
 * Test 0: Client sends KeyUpdate while Server is writing
 * Test 1: Server sends KeyUpdate while Client is writing
 */
static int test_key_update_peer_in_write(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[20];
    static char *mess = "A test message";
    BIO *bretry = BIO_new(bio_s_always_retry());
    BIO *tmp = NULL;
    SSL *peerupdate = NULL, *peerwrite = NULL;

    if (!TEST_ptr(bretry)
            || !TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                              TLS_client_method(),
                                              TLS1_3_VERSION,
                                              0,
                                              &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    peerupdate = tst == 0 ? clientssl : serverssl;
    peerwrite = tst == 0 ? serverssl : clientssl;

    if (!TEST_true(SSL_key_update(peerupdate, SSL_KEY_UPDATE_REQUESTED))
            || !TEST_int_eq(SSL_do_handshake(peerupdate), 1))
        goto end;

    /* Swap the writing endpoint's write BIO to force a retry */
    tmp = SSL_get_wbio(peerwrite);
    if (!TEST_ptr(tmp) || !TEST_true(BIO_up_ref(tmp))) {
        tmp = NULL;
        goto end;
    }
    SSL_set0_wbio(peerwrite, bretry);
    bretry = NULL;

    /* Write data that we know will fail with SSL_ERROR_WANT_WRITE */
    if (!TEST_int_eq(SSL_write(peerwrite, mess, strlen(mess)), -1)
            || !TEST_int_eq(SSL_get_error(peerwrite, 0), SSL_ERROR_WANT_WRITE))
        goto end;

    /* Reinstate the original writing endpoint's write BIO */
    SSL_set0_wbio(peerwrite, tmp);
    tmp = NULL;

    /* Now read some data - we will read the key update */
    if (!TEST_int_eq(SSL_read(peerwrite, buf, sizeof(buf)), -1)
            || !TEST_int_eq(SSL_get_error(peerwrite, 0), SSL_ERROR_WANT_READ))
        goto end;

    /*
     * Complete the write we started previously and read it from the other
     * endpoint
     */
    if (!TEST_int_eq(SSL_write(peerwrite, mess, strlen(mess)), strlen(mess))
            || !TEST_int_eq(SSL_read(peerupdate, buf, sizeof(buf)), strlen(mess)))
        goto end;

    /* Write more data to ensure we send the KeyUpdate message back */
    if (!TEST_int_eq(SSL_write(peerwrite, mess, strlen(mess)), strlen(mess))
            || !TEST_int_eq(SSL_read(peerupdate, buf, sizeof(buf)), strlen(mess)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    BIO_free(bretry);
    BIO_free(tmp);

    return testresult;
}

/*
 * Test we can handle a KeyUpdate (update requested) message while
 * peer read data is pending after peer accepted keyupdate(the msg header
 * had been read 5 bytes).
 * Test 0: Client sends KeyUpdate while Server is reading
 * Test 1: Server sends KeyUpdate while Client is reading
 */
static int test_key_update_peer_in_read(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char prbuf[515], lwbuf[515] = {0};
    static char *mess = "A test message";
    BIO *lbio = NULL, *pbio = NULL;
    SSL *local = NULL, *peer = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                              TLS_client_method(),
                                              TLS1_3_VERSION,
                                              0,
                                              &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    local = tst == 0 ? clientssl : serverssl;
    peer = tst == 0 ? serverssl : clientssl;

    if (!TEST_int_eq(BIO_new_bio_pair(&lbio, 512, &pbio, 512), 1))
        goto end;

    SSL_set_bio(local, lbio, lbio);
    SSL_set_bio(peer, pbio, pbio);

    /*
     * we first write keyupdate msg then appdata in local
     * write data in local will fail with SSL_ERROR_WANT_WRITE,because
     * lwbuf app data msg size + key updata msg size > 512(the size of
     * the bio pair buffer)
     */
    if (!TEST_true(SSL_key_update(local, SSL_KEY_UPDATE_REQUESTED))
            || !TEST_int_eq(SSL_write(local, lwbuf, sizeof(lwbuf)), -1)
            || !TEST_int_eq(SSL_get_error(local, -1), SSL_ERROR_WANT_WRITE))
        goto end;

    /*
     * first read keyupdate msg in peer in peer
     * then read appdata that we know will fail with SSL_ERROR_WANT_READ
     */
    if (!TEST_int_eq(SSL_read(peer, prbuf, sizeof(prbuf)), -1)
            || !TEST_int_eq(SSL_get_error(peer, -1), SSL_ERROR_WANT_READ))
        goto end;

    /* Now write some data in peer - we will write the key update */
    if (!TEST_int_eq(SSL_write(peer, mess, strlen(mess)), strlen(mess)))
        goto end;

    /*
     * write data in local previously that we will complete
     * read data in peer previously that we will complete
     */
    if (!TEST_int_eq(SSL_write(local, lwbuf, sizeof(lwbuf)), sizeof(lwbuf))
            || !TEST_int_eq(SSL_read(peer, prbuf, sizeof(prbuf)), sizeof(prbuf)))
        goto end;

    /* check that sending and receiving appdata ok */
    if (!TEST_int_eq(SSL_write(local, mess, strlen(mess)), strlen(mess))
            || !TEST_int_eq(SSL_read(peer, prbuf, sizeof(prbuf)), strlen(mess)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test we can't send a KeyUpdate (update requested) message while
 * local write data is pending.
 * Test 0: Client sends KeyUpdate while Client is writing
 * Test 1: Server sends KeyUpdate while Server is writing
 */
static int test_key_update_local_in_write(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[20];
    static char *mess = "A test message";
    BIO *bretry = BIO_new(bio_s_always_retry());
    BIO *tmp = NULL;
    SSL *local = NULL, *peer = NULL;

    if (!TEST_ptr(bretry)
            || !TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                              TLS_client_method(),
                                              TLS1_3_VERSION,
                                              0,
                                              &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    local = tst == 0 ? clientssl : serverssl;
    peer = tst == 0 ? serverssl : clientssl;

    /* Swap the writing endpoint's write BIO to force a retry */
    tmp = SSL_get_wbio(local);
    if (!TEST_ptr(tmp) || !TEST_true(BIO_up_ref(tmp))) {
        tmp = NULL;
        goto end;
    }
    SSL_set0_wbio(local, bretry);
    bretry = NULL;

    /* write data in local will fail with SSL_ERROR_WANT_WRITE */
    if (!TEST_int_eq(SSL_write(local, mess, strlen(mess)), -1)
            || !TEST_int_eq(SSL_get_error(local, -1), SSL_ERROR_WANT_WRITE))
        goto end;

    /* Reinstate the original writing endpoint's write BIO */
    SSL_set0_wbio(local, tmp);
    tmp = NULL;

    /* SSL_key_update will fail, because writing in local*/
    if (!TEST_false(SSL_key_update(local, SSL_KEY_UPDATE_REQUESTED))
        || !TEST_int_eq(ERR_GET_REASON(ERR_peek_error()), SSL_R_BAD_WRITE_RETRY))
    goto end;

    ERR_clear_error();
    /* write data in local previously that we will complete */
    if (!TEST_int_eq(SSL_write(local, mess, strlen(mess)), strlen(mess)))
        goto end;

    /* SSL_key_update will succeed because there is no pending write data */
    if (!TEST_true(SSL_key_update(local, SSL_KEY_UPDATE_REQUESTED))
        || !TEST_int_eq(SSL_do_handshake(local), 1))
        goto end;

    /*
     * we write some appdata in local
     * read data in peer - we will read the keyupdate msg
     */
    if (!TEST_int_eq(SSL_write(local, mess, strlen(mess)), strlen(mess))
        || !TEST_int_eq(SSL_read(peer, buf, sizeof(buf)), strlen(mess)))
        goto end;

    /* Write more peer more data to ensure we send the keyupdate message back */
    if (!TEST_int_eq(SSL_write(peer, mess, strlen(mess)), strlen(mess))
            || !TEST_int_eq(SSL_read(local, buf, sizeof(buf)), strlen(mess)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    BIO_free(bretry);
    BIO_free(tmp);

    return testresult;
}

/*
 * Test we can handle a KeyUpdate (update requested) message while
 * local read data is pending(the msg header had been read 5 bytes).
 * Test 0: Client sends KeyUpdate while Client is reading
 * Test 1: Server sends KeyUpdate while Server is reading
 */
static int test_key_update_local_in_read(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char lrbuf[515], pwbuf[515] = {0}, prbuf[20];
    static char *mess = "A test message";
    BIO *lbio = NULL, *pbio = NULL;
    SSL *local = NULL, *peer = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                              TLS_client_method(),
                                              TLS1_3_VERSION,
                                              0,
                                              &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    local = tst == 0 ? clientssl : serverssl;
    peer = tst == 0 ? serverssl : clientssl;

    if (!TEST_int_eq(BIO_new_bio_pair(&lbio, 512, &pbio, 512), 1))
        goto end;

    SSL_set_bio(local, lbio, lbio);
    SSL_set_bio(peer, pbio, pbio);

    /* write app data in peer will fail with SSL_ERROR_WANT_WRITE */
    if (!TEST_int_eq(SSL_write(peer, pwbuf, sizeof(pwbuf)), -1)
        || !TEST_int_eq(SSL_get_error(peer, -1), SSL_ERROR_WANT_WRITE))
        goto end;

    /* read appdata in local will fail with SSL_ERROR_WANT_READ */
    if (!TEST_int_eq(SSL_read(local, lrbuf, sizeof(lrbuf)), -1)
            || !TEST_int_eq(SSL_get_error(local, -1), SSL_ERROR_WANT_READ))
        goto end;

    /* SSL_do_handshake will send keyupdate msg */
    if (!TEST_true(SSL_key_update(local, SSL_KEY_UPDATE_REQUESTED))
            || !TEST_int_eq(SSL_do_handshake(local), 1))
        goto end;

    /*
     * write data in peer previously that we will complete
     * read data in local previously that we will complete
     */
    if (!TEST_int_eq(SSL_write(peer, pwbuf, sizeof(pwbuf)), sizeof(pwbuf))
        || !TEST_int_eq(SSL_read(local, lrbuf, sizeof(lrbuf)), sizeof(lrbuf)))
        goto end;

    /*
     * write data in local
     * read data in peer - we will read the key update
     */
    if (!TEST_int_eq(SSL_write(local, mess, strlen(mess)), strlen(mess))
        || !TEST_int_eq(SSL_read(peer, prbuf, sizeof(prbuf)), strlen(mess)))
        goto end;

  /* Write more peer data to ensure we send the keyupdate message back */
    if (!TEST_int_eq(SSL_write(peer, mess, strlen(mess)), strlen(mess))
            || !TEST_int_eq(SSL_read(local, lrbuf, sizeof(lrbuf)), strlen(mess)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* OSSL_NO_USABLE_TLS1_3 */

static int test_ssl_clear(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (idx == 1)
        return 1;
#endif

    /* Create an initial connection */
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey))
            || (idx == 1
                && !TEST_true(SSL_CTX_set_max_proto_version(cctx,
                                                            TLS1_2_VERSION)))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                          &clientssl, NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    serverssl = NULL;

    /* Clear clientssl - we're going to reuse the object */
    if (!TEST_true(SSL_clear(clientssl)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE))
            || !TEST_true(SSL_session_reused(clientssl)))
        goto end;

    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/* Parse CH and retrieve any MFL extension value if present */
static int get_MFL_from_client_hello(BIO *bio, int *mfl_codemfl_code)
{
    long len;
    unsigned char *data;
    PACKET pkt, pkt2, pkt3;
    unsigned int MFL_code = 0, type = 0;

    if (!TEST_uint_gt( len = BIO_get_mem_data( bio, (char **) &data ), 0 ) )
        goto end;

    memset(&pkt, 0, sizeof(pkt));
    memset(&pkt2, 0, sizeof(pkt2));
    memset(&pkt3, 0, sizeof(pkt3));

    if (!TEST_long_gt(len, 0)
            || !TEST_true( PACKET_buf_init( &pkt, data, len ) )
               /* Skip the record header */
            || !PACKET_forward(&pkt, SSL3_RT_HEADER_LENGTH)
               /* Skip the handshake message header */
            || !TEST_true(PACKET_forward(&pkt, SSL3_HM_HEADER_LENGTH))
               /* Skip client version and random */
            || !TEST_true(PACKET_forward(&pkt, CLIENT_VERSION_LEN
                                               + SSL3_RANDOM_SIZE))
               /* Skip session id */
            || !TEST_true(PACKET_get_length_prefixed_1(&pkt, &pkt2))
               /* Skip ciphers */
            || !TEST_true(PACKET_get_length_prefixed_2(&pkt, &pkt2))
               /* Skip compression */
            || !TEST_true(PACKET_get_length_prefixed_1(&pkt, &pkt2))
               /* Extensions len */
            || !TEST_true(PACKET_as_length_prefixed_2(&pkt, &pkt2)))
        goto end;

    /* Loop through all extensions */
    while (PACKET_remaining(&pkt2)) {
        if (!TEST_true(PACKET_get_net_2(&pkt2, &type))
                || !TEST_true(PACKET_get_length_prefixed_2(&pkt2, &pkt3)))
            goto end;

        if (type == TLSEXT_TYPE_max_fragment_length) {
            if (!TEST_uint_ne(PACKET_remaining(&pkt3), 0)
                    || !TEST_true(PACKET_get_1(&pkt3, &MFL_code)))
                goto end;

            *mfl_codemfl_code = MFL_code;
            return 1;
        }
    }

 end:
    return 0;
}

/* Maximum-Fragment-Length TLS extension mode to test */
static const unsigned char max_fragment_len_test[] = {
    TLSEXT_max_fragment_length_512,
    TLSEXT_max_fragment_length_1024,
    TLSEXT_max_fragment_length_2048,
    TLSEXT_max_fragment_length_4096
};

static int test_max_fragment_len_ext(int idx_tst)
{
    SSL_CTX *ctx = NULL;
    SSL *con = NULL;
    int testresult = 0, MFL_mode = 0;
    BIO *rbio, *wbio;

    if (!TEST_true(create_ssl_ctx_pair(libctx, NULL, TLS_client_method(),
                                       TLS1_VERSION, 0, NULL, &ctx, NULL,
                                       NULL)))
        return 0;

    if (!TEST_true(SSL_CTX_set_tlsext_max_fragment_length(
                   ctx, max_fragment_len_test[idx_tst])))
        goto end;

    con = SSL_new(ctx);
    if (!TEST_ptr(con))
        goto end;

    rbio = BIO_new(BIO_s_mem());
    wbio = BIO_new(BIO_s_mem());
    if (!TEST_ptr(rbio)|| !TEST_ptr(wbio)) {
        BIO_free(rbio);
        BIO_free(wbio);
        goto end;
    }

    SSL_set_bio(con, rbio, wbio);

    if (!TEST_int_le(SSL_connect(con), 0)) {
        /* This shouldn't succeed because we don't have a server! */
        goto end;
    }

    if (!TEST_true(get_MFL_from_client_hello(wbio, &MFL_mode)))
        /* no MFL in client hello */
        goto end;
    if (!TEST_true(max_fragment_len_test[idx_tst] == MFL_mode))
        goto end;

    testresult = 1;

end:
    SSL_free(con);
    SSL_CTX_free(ctx);

    return testresult;
}

#ifndef OSSL_NO_USABLE_TLS1_3
static int test_pha_key_update(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    if (!TEST_true(SSL_CTX_set_min_proto_version(sctx, TLS1_3_VERSION))
        || !TEST_true(SSL_CTX_set_max_proto_version(sctx, TLS1_3_VERSION))
        || !TEST_true(SSL_CTX_set_min_proto_version(cctx, TLS1_3_VERSION))
        || !TEST_true(SSL_CTX_set_max_proto_version(cctx, TLS1_3_VERSION)))
        goto end;

    SSL_CTX_set_post_handshake_auth(cctx, 1);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                         SSL_ERROR_NONE)))
        goto end;

    SSL_set_verify(serverssl, SSL_VERIFY_PEER, NULL);
    if (!TEST_true(SSL_verify_client_post_handshake(serverssl)))
        goto end;

    if (!TEST_true(SSL_key_update(clientssl, SSL_KEY_UPDATE_NOT_REQUESTED)))
        goto end;

    /* Start handshake on the server */
    if (!TEST_int_eq(SSL_do_handshake(serverssl), 1))
        goto end;

    /* Starts with SSL_connect(), but it's really just SSL_do_handshake() */
    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                         SSL_ERROR_NONE)))
        goto end;

    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}
#endif

#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)

static SRP_VBASE *vbase = NULL;

static int ssl_srp_cb(SSL *s, int *ad, void *arg)
{
    int ret = SSL3_AL_FATAL;
    char *username;
    SRP_user_pwd *user = NULL;

    username = SSL_get_srp_username(s);
    if (username == NULL) {
        *ad = SSL_AD_INTERNAL_ERROR;
        goto err;
    }

    user = SRP_VBASE_get1_by_user(vbase, username);
    if (user == NULL) {
        *ad = SSL_AD_INTERNAL_ERROR;
        goto err;
    }

    if (SSL_set_srp_server_param(s, user->N, user->g, user->s, user->v,
                                 user->info) <= 0) {
        *ad = SSL_AD_INTERNAL_ERROR;
        goto err;
    }

    ret = 0;

 err:
    SRP_user_pwd_free(user);
    return ret;
}

static int create_new_vfile(char *userid, char *password, const char *filename)
{
    char *gNid = NULL;
    OPENSSL_STRING *row = OPENSSL_zalloc(sizeof(row) * (DB_NUMBER + 1));
    TXT_DB *db = NULL;
    int ret = 0;
    BIO *out = NULL, *dummy = BIO_new_mem_buf("", 0);
    size_t i;

    if (!TEST_ptr(dummy) || !TEST_ptr(row))
        goto end;

    gNid = SRP_create_verifier_ex(userid, password, &row[DB_srpsalt],
                                  &row[DB_srpverifier], NULL, NULL, libctx, NULL);
    if (!TEST_ptr(gNid))
        goto end;

    /*
     * The only way to create an empty TXT_DB is to provide a BIO with no data
     * in it!
     */
    db = TXT_DB_read(dummy, DB_NUMBER);
    if (!TEST_ptr(db))
        goto end;

    out = BIO_new_file(filename, "w");
    if (!TEST_ptr(out))
        goto end;

    row[DB_srpid] = OPENSSL_strdup(userid);
    row[DB_srptype] = OPENSSL_strdup("V");
    row[DB_srpgN] = OPENSSL_strdup(gNid);

    if (!TEST_ptr(row[DB_srpid])
            || !TEST_ptr(row[DB_srptype])
            || !TEST_ptr(row[DB_srpgN])
            || !TEST_true(TXT_DB_insert(db, row)))
        goto end;

    row = NULL;

    if (TXT_DB_write(out, db) <= 0)
        goto end;

    ret = 1;
 end:
    if (row != NULL) {
        for (i = 0; i < DB_NUMBER; i++)
            OPENSSL_free(row[i]);
    }
    OPENSSL_free(row);
    BIO_free(dummy);
    BIO_free(out);
    TXT_DB_free(db);

    return ret;
}

static int create_new_vbase(char *userid, char *password)
{
    BIGNUM *verifier = NULL, *salt = NULL;
    const SRP_gN *lgN = NULL;
    SRP_user_pwd *user_pwd = NULL;
    int ret = 0;

    lgN = SRP_get_default_gN(NULL);
    if (!TEST_ptr(lgN))
        goto end;

    if (!TEST_true(SRP_create_verifier_BN_ex(userid, password, &salt, &verifier,
                                             lgN->N, lgN->g, libctx, NULL)))
        goto end;

    user_pwd = OPENSSL_zalloc(sizeof(*user_pwd));
    if (!TEST_ptr(user_pwd))
        goto end;

    user_pwd->N = lgN->N;
    user_pwd->g = lgN->g;
    user_pwd->id = OPENSSL_strdup(userid);
    if (!TEST_ptr(user_pwd->id))
        goto end;

    user_pwd->v = verifier;
    user_pwd->s = salt;
    verifier = salt = NULL;

    if (sk_SRP_user_pwd_insert(vbase->users_pwd, user_pwd, 0) == 0)
        goto end;
    user_pwd = NULL;

    ret = 1;
end:
    SRP_user_pwd_free(user_pwd);
    BN_free(salt);
    BN_free(verifier);

    return ret;
}

/*
 * SRP tests
 *
 * Test 0: Simple successful SRP connection, new vbase
 * Test 1: Connection failure due to bad password, new vbase
 * Test 2: Simple successful SRP connection, vbase loaded from existing file
 * Test 3: Connection failure due to bad password, vbase loaded from existing
 *         file
 * Test 4: Simple successful SRP connection, vbase loaded from new file
 * Test 5: Connection failure due to bad password, vbase loaded from new file
 */
static int test_srp(int tst)
{
    char *userid = "test", *password = "password", *tstsrpfile;
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int ret, testresult = 0;

    vbase = SRP_VBASE_new(NULL);
    if (!TEST_ptr(vbase))
        goto end;

    if (tst == 0 || tst == 1) {
        if (!TEST_true(create_new_vbase(userid, password)))
            goto end;
    } else {
        if (tst == 4 || tst == 5) {
            if (!TEST_true(create_new_vfile(userid, password, tmpfilename)))
                goto end;
            tstsrpfile = tmpfilename;
        } else {
            tstsrpfile = srpvfile;
        }
        if (!TEST_int_eq(SRP_VBASE_init(vbase, tstsrpfile), SRP_NO_ERROR))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_int_gt(SSL_CTX_set_srp_username_callback(sctx, ssl_srp_cb), 0)
            || !TEST_true(SSL_CTX_set_cipher_list(cctx, "SRP-AES-128-CBC-SHA"))
            || !TEST_true(SSL_CTX_set_max_proto_version(sctx, TLS1_2_VERSION))
            || !TEST_true(SSL_CTX_set_max_proto_version(cctx, TLS1_2_VERSION))
            || !TEST_int_gt(SSL_CTX_set_srp_username(cctx, userid), 0))
        goto end;

    if (tst % 2 == 1) {
        if (!TEST_int_gt(SSL_CTX_set_srp_password(cctx, "badpass"), 0))
            goto end;
    } else {
        if (!TEST_int_gt(SSL_CTX_set_srp_password(cctx, password), 0))
            goto end;
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    ret = create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE);
    if (ret) {
        if (!TEST_true(tst % 2 == 0))
            goto end;
    } else {
        if (!TEST_true(tst % 2 == 1))
            goto end;
    }

    testresult = 1;

 end:
    SRP_VBASE_free(vbase);
    vbase = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif

static int info_cb_failed = 0;
static int info_cb_offset = 0;
static int info_cb_this_state = -1;

static struct info_cb_states_st {
    int where;
    const char *statestr;
} info_cb_states[][60] = {
    {
        /* TLSv1.2 server followed by resumption */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TRCH"}, {SSL_CB_LOOP, "TWSH"},
        {SSL_CB_LOOP, "TWSC"}, {SSL_CB_LOOP, "TWSKE"}, {SSL_CB_LOOP, "TWSD"},
        {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWSD"}, {SSL_CB_LOOP, "TRCKE"},
        {SSL_CB_LOOP, "TRCCS"}, {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_LOOP, "TWST"},
        {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL},
        {SSL_CB_ALERT, NULL}, {SSL_CB_HANDSHAKE_START, NULL},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TRCH"},
        {SSL_CB_LOOP, "TWSH"}, {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWFIN"}, {SSL_CB_LOOP, "TRCCS"},
        {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_HANDSHAKE_DONE, NULL},
        {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        /* TLSv1.2 client followed by resumption */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "TWCH"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWCH"},
        {SSL_CB_LOOP, "TRSH"}, {SSL_CB_LOOP, "TRSC"}, {SSL_CB_LOOP, "TRSKE"},
        {SSL_CB_LOOP, "TRSD"}, {SSL_CB_LOOP, "TWCKE"}, {SSL_CB_LOOP, "TWCCS"},
        {SSL_CB_LOOP, "TWFIN"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_LOOP, "TRST"}, {SSL_CB_LOOP, "TRCCS"}, {SSL_CB_LOOP, "TRFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL}, {SSL_CB_ALERT, NULL},
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "TWCH"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWCH"},
        {SSL_CB_LOOP, "TRSH"}, {SSL_CB_LOOP, "TRCCS"}, {SSL_CB_LOOP, "TRFIN"},
        {SSL_CB_LOOP, "TWCCS"},  {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        /* TLSv1.3 server followed by resumption */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TRCH"}, {SSL_CB_LOOP, "TWSH"},
        {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWEE"}, {SSL_CB_LOOP, "TWSC"},
        {SSL_CB_LOOP, "TWSCV"}, {SSL_CB_LOOP, "TWFIN"}, {SSL_CB_LOOP, "TED"},
        {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TED"}, {SSL_CB_LOOP, "TRFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_LOOP, "TWST"},
        {SSL_CB_LOOP, "TWST"}, {SSL_CB_EXIT, NULL}, {SSL_CB_ALERT, NULL},
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TRCH"}, {SSL_CB_LOOP, "TWSH"},
        {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWEE"}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_LOOP, "TED"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TED"},
        {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_HANDSHAKE_DONE, NULL},
        {SSL_CB_LOOP, "TWST"}, {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        /* TLSv1.3 client followed by resumption */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "TWCH"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "TWCH"},
        {SSL_CB_LOOP, "TRSH"}, {SSL_CB_LOOP, "TREE"}, {SSL_CB_LOOP, "TRSC"},
        {SSL_CB_LOOP, "TRSCV"}, {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_LOOP, "TWCCS"},
        {SSL_CB_LOOP, "TWFIN"},  {SSL_CB_HANDSHAKE_DONE, NULL},
        {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "SSLOK"}, {SSL_CB_LOOP, "SSLOK"},
        {SSL_CB_LOOP, "TRST"}, {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "SSLOK"},
        {SSL_CB_LOOP, "SSLOK"}, {SSL_CB_LOOP, "TRST"}, {SSL_CB_EXIT, NULL},
        {SSL_CB_ALERT, NULL}, {SSL_CB_HANDSHAKE_START, NULL},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TWCH"}, {SSL_CB_EXIT, NULL},
        {SSL_CB_LOOP, "TWCH"}, {SSL_CB_LOOP, "TRSH"},  {SSL_CB_LOOP, "TREE"},
        {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL},
        {SSL_CB_LOOP, "SSLOK"}, {SSL_CB_LOOP, "SSLOK"}, {SSL_CB_LOOP, "TRST"},
        {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        /* TLSv1.3 server, early_data */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "PINIT"}, {SSL_CB_LOOP, "TRCH"}, {SSL_CB_LOOP, "TWSH"},
        {SSL_CB_LOOP, "TWCCS"}, {SSL_CB_LOOP, "TWEE"}, {SSL_CB_LOOP, "TWFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL},
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "TED"},
        {SSL_CB_LOOP, "TED"}, {SSL_CB_LOOP, "TWEOED"}, {SSL_CB_LOOP, "TRFIN"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_LOOP, "TWST"},
        {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        /* TLSv1.3 client, early_data */
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "PINIT"},
        {SSL_CB_LOOP, "TWCH"}, {SSL_CB_LOOP, "TWCCS"},
        {SSL_CB_HANDSHAKE_DONE, NULL}, {SSL_CB_EXIT, NULL},
        {SSL_CB_HANDSHAKE_START, NULL}, {SSL_CB_LOOP, "TED"},
        {SSL_CB_LOOP, "TED"}, {SSL_CB_LOOP, "TRSH"}, {SSL_CB_LOOP, "TREE"},
        {SSL_CB_LOOP, "TRFIN"}, {SSL_CB_LOOP, "TPEDE"}, {SSL_CB_LOOP, "TWEOED"},
        {SSL_CB_LOOP, "TWFIN"}, {SSL_CB_HANDSHAKE_DONE, NULL},
        {SSL_CB_EXIT, NULL}, {SSL_CB_LOOP, "SSLOK"}, {SSL_CB_LOOP, "SSLOK"},
        {SSL_CB_LOOP, "TRST"}, {SSL_CB_EXIT, NULL}, {0, NULL},
    }, {
        {0, NULL},
    }
};

static void sslapi_info_callback(const SSL *s, int where, int ret)
{
    struct info_cb_states_st *state = info_cb_states[info_cb_offset];

    /* We do not ever expect a connection to fail in this test */
    if (!TEST_false(ret == 0)) {
        info_cb_failed = 1;
        return;
    }

    /*
     * Do some sanity checks. We never expect these things to happen in this
     * test
     */
    if (!TEST_false((SSL_is_server(s) && (where & SSL_ST_CONNECT) != 0))
            || !TEST_false(!SSL_is_server(s) && (where & SSL_ST_ACCEPT) != 0)
            || !TEST_int_ne(state[++info_cb_this_state].where, 0)) {
        info_cb_failed = 1;
        return;
    }

    /* Now check we're in the right state */
    if (!TEST_true((where & state[info_cb_this_state].where) != 0)) {
        info_cb_failed = 1;
        return;
    }
    if ((where & SSL_CB_LOOP) != 0
            && !TEST_int_eq(strcmp(SSL_state_string(s),
                            state[info_cb_this_state].statestr), 0)) {
        info_cb_failed = 1;
        return;
    }

    /*
     * Check that, if we've got SSL_CB_HANDSHAKE_DONE we are not in init
     */
    if ((where & SSL_CB_HANDSHAKE_DONE)
            && SSL_in_init((SSL *)s) != 0) {
        info_cb_failed = 1;
        return;
    }
}

/*
 * Test the info callback gets called when we expect it to.
 *
 * Test 0: TLSv1.2, server
 * Test 1: TLSv1.2, client
 * Test 2: TLSv1.3, server
 * Test 3: TLSv1.3, client
 * Test 4: TLSv1.3, server, early_data
 * Test 5: TLSv1.3, client, early_data
 */
static int test_info_callback(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *clntsess = NULL;
    int testresult = 0;
    int tlsvers;

    if (tst < 2) {
/* We need either ECDHE or DHE for the TLSv1.2 test to work */
#if !defined(OPENSSL_NO_TLS1_2) && (!defined(OPENSSL_NO_EC) \
                                    || !defined(OPENSSL_NO_DH))
        tlsvers = TLS1_2_VERSION;
#else
        return 1;
#endif
    } else {
#ifndef OSSL_NO_USABLE_TLS1_3
        tlsvers = TLS1_3_VERSION;
#else
        return 1;
#endif
    }

    /* Reset globals */
    info_cb_failed = 0;
    info_cb_this_state = -1;
    info_cb_offset = tst;

#ifndef OSSL_NO_USABLE_TLS1_3
    if (tst >= 4) {
        SSL_SESSION *sess = NULL;
        size_t written, readbytes;
        unsigned char buf[80];

        /* early_data tests */
        if (!TEST_true(setupearly_data_test(&cctx, &sctx, &clientssl,
                                            &serverssl, &sess, 0)))
            goto end;

        /* We don't actually need this reference */
        SSL_SESSION_free(sess);

        SSL_set_info_callback((tst % 2) == 0 ? serverssl : clientssl,
                              sslapi_info_callback);

        /* Write and read some early data and then complete the connection */
        if (!TEST_true(SSL_write_early_data(clientssl, MSG1, strlen(MSG1),
                                            &written))
                || !TEST_size_t_eq(written, strlen(MSG1))
                || !TEST_int_eq(SSL_read_early_data(serverssl, buf,
                                                    sizeof(buf), &readbytes),
                                SSL_READ_EARLY_DATA_SUCCESS)
                || !TEST_mem_eq(MSG1, readbytes, buf, strlen(MSG1))
                || !TEST_int_eq(SSL_get_early_data_status(serverssl),
                                SSL_EARLY_DATA_ACCEPTED)
                || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                    SSL_ERROR_NONE))
                || !TEST_false(info_cb_failed))
            goto end;

        testresult = 1;
        goto end;
    }
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       tlsvers, tlsvers, &sctx, &cctx, cert,
                                       privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_dh_auto(sctx, 1)))
        goto end;

    /*
     * For even numbered tests we check the server callbacks. For odd numbers we
     * check the client.
     */
    SSL_CTX_set_info_callback((tst % 2) == 0 ? sctx : cctx,
                              sslapi_info_callback);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                          &clientssl, NULL, NULL))
        || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                            SSL_ERROR_NONE))
        || !TEST_false(info_cb_failed))
    goto end;



    clntsess = SSL_get1_session(clientssl);
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
    serverssl = clientssl = NULL;

    /* Now do a resumption */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL))
            || !TEST_true(SSL_set_session(clientssl, clntsess))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE))
            || !TEST_true(SSL_session_reused(clientssl))
            || !TEST_false(info_cb_failed))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_SESSION_free(clntsess);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}

static int test_ssl_pending(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char msg[] = "A test message";
    char buf[5];
    size_t written, readbytes;

    if (tst == 0) {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION, 0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;
    } else {
#ifndef OPENSSL_NO_DTLS
        if (!TEST_true(create_ssl_ctx_pair(libctx, DTLS_server_method(),
                                           DTLS_client_method(),
                                           DTLS1_VERSION, 0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;

# ifdef OPENSSL_NO_DTLS1_2
        /* Not supported in the FIPS provider */
        if (is_fips) {
            testresult = 1;
            goto end;
        };
        /*
         * Default sigalgs are SHA1 based in <DTLS1.2 which is in security
         * level 0
         */
        if (!TEST_true(SSL_CTX_set_cipher_list(sctx, "DEFAULT:@SECLEVEL=0"))
                || !TEST_true(SSL_CTX_set_cipher_list(cctx,
                                                    "DEFAULT:@SECLEVEL=0")))
            goto end;
# endif
#else
        return 1;
#endif
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (!TEST_int_eq(SSL_pending(clientssl), 0)
            || !TEST_false(SSL_has_pending(clientssl))
            || !TEST_int_eq(SSL_pending(serverssl), 0)
            || !TEST_false(SSL_has_pending(serverssl))
            || !TEST_true(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
            || !TEST_size_t_eq(written, sizeof(msg))
            || !TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf), &readbytes))
            || !TEST_size_t_eq(readbytes, sizeof(buf))
            || !TEST_int_eq(SSL_pending(clientssl), (int)(written - readbytes))
            || !TEST_true(SSL_has_pending(clientssl)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static struct {
    unsigned int maxprot;
    const char *clntciphers;
    const char *clnttls13ciphers;
    const char *srvrciphers;
    const char *srvrtls13ciphers;
    const char *shared;
    const char *fipsshared;
} shared_ciphers_data[] = {
/*
 * We can't establish a connection (even in TLSv1.1) with these ciphersuites if
 * TLSv1.3 is enabled but TLSv1.2 is disabled.
 */
#if defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2)
    {
        TLS1_2_VERSION,
        "AES128-SHA:AES256-SHA",
        NULL,
        "AES256-SHA:DHE-RSA-AES128-SHA",
        NULL,
        "AES256-SHA",
        "AES256-SHA"
    },
# if !defined(OPENSSL_NO_CHACHA) \
     && !defined(OPENSSL_NO_POLY1305) \
     && !defined(OPENSSL_NO_EC)
    {
        TLS1_2_VERSION,
        "AES128-SHA:ECDHE-RSA-CHACHA20-POLY1305",
        NULL,
        "AES128-SHA:ECDHE-RSA-CHACHA20-POLY1305",
        NULL,
        "AES128-SHA:ECDHE-RSA-CHACHA20-POLY1305",
        "AES128-SHA"
    },
# endif
    {
        TLS1_2_VERSION,
        "AES128-SHA:DHE-RSA-AES128-SHA:AES256-SHA",
        NULL,
        "AES128-SHA:DHE-RSA-AES256-SHA:AES256-SHA",
        NULL,
        "AES128-SHA:AES256-SHA",
        "AES128-SHA:AES256-SHA"
    },
    {
        TLS1_2_VERSION,
        "AES128-SHA:AES256-SHA",
        NULL,
        "AES128-SHA:DHE-RSA-AES128-SHA",
        NULL,
        "AES128-SHA",
        "AES128-SHA"
    },
#endif
/*
 * This test combines TLSv1.3 and TLSv1.2 ciphersuites so they must both be
 * enabled.
 */
#if !defined(OSSL_NO_USABLE_TLS1_3) && !defined(OPENSSL_NO_TLS1_2) \
    && !defined(OPENSSL_NO_CHACHA) && !defined(OPENSSL_NO_POLY1305)
    {
        TLS1_3_VERSION,
        "AES128-SHA:AES256-SHA",
        NULL,
        "AES256-SHA:AES128-SHA256",
        NULL,
        "TLS_AES_256_GCM_SHA384:TLS_CHACHA20_POLY1305_SHA256:"
        "TLS_AES_128_GCM_SHA256:AES256-SHA",
        "TLS_AES_256_GCM_SHA384:TLS_AES_128_GCM_SHA256:AES256-SHA"
    },
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    {
        TLS1_3_VERSION,
        "AES128-SHA",
        "TLS_AES_256_GCM_SHA384",
        "AES256-SHA",
        "TLS_AES_256_GCM_SHA384",
        "TLS_AES_256_GCM_SHA384",
        "TLS_AES_256_GCM_SHA384"
    },
#endif
};

static int int_test_ssl_get_shared_ciphers(int tst, int clnt)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[1024];
    OSSL_LIB_CTX *tmplibctx = OSSL_LIB_CTX_new();

    if (!TEST_ptr(tmplibctx))
        goto end;

    /*
     * Regardless of whether we're testing with the FIPS provider loaded into
     * libctx, we want one peer to always use the full set of ciphersuites
     * available. Therefore we use a separate libctx with the default provider
     * loaded into it. We run the same tests twice - once with the client side
     * having the full set of ciphersuites and once with the server side.
     */
    if (clnt) {
        cctx = SSL_CTX_new_ex(tmplibctx, NULL, TLS_client_method());
        if (!TEST_ptr(cctx))
            goto end;
    } else {
        sctx = SSL_CTX_new_ex(tmplibctx, NULL, TLS_server_method());
        if (!TEST_ptr(sctx))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       shared_ciphers_data[tst].maxprot,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                        shared_ciphers_data[tst].clntciphers))
            || (shared_ciphers_data[tst].clnttls13ciphers != NULL
                && !TEST_true(SSL_CTX_set_ciphersuites(cctx,
                                    shared_ciphers_data[tst].clnttls13ciphers)))
            || !TEST_true(SSL_CTX_set_cipher_list(sctx,
                                        shared_ciphers_data[tst].srvrciphers))
            || (shared_ciphers_data[tst].srvrtls13ciphers != NULL
                && !TEST_true(SSL_CTX_set_ciphersuites(sctx,
                                    shared_ciphers_data[tst].srvrtls13ciphers))))
        goto end;


    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (!TEST_ptr(SSL_get_shared_ciphers(serverssl, buf, sizeof(buf)))
            || !TEST_int_eq(strcmp(buf,
                                   is_fips
                                   ? shared_ciphers_data[tst].fipsshared
                                   : shared_ciphers_data[tst].shared),
                                   0)) {
        TEST_info("Shared ciphers are: %s\n", buf);
        goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_LIB_CTX_free(tmplibctx);

    return testresult;
}

static int test_ssl_get_shared_ciphers(int tst)
{
    return int_test_ssl_get_shared_ciphers(tst, 0)
           && int_test_ssl_get_shared_ciphers(tst, 1);
}


static const char *appdata = "Hello World";
static int gen_tick_called, dec_tick_called, tick_key_cb_called;
static int tick_key_renew = 0;
static SSL_TICKET_RETURN tick_dec_ret = SSL_TICKET_RETURN_ABORT;

static int gen_tick_cb(SSL *s, void *arg)
{
    gen_tick_called = 1;

    return SSL_SESSION_set1_ticket_appdata(SSL_get_session(s), appdata,
                                           strlen(appdata));
}

static SSL_TICKET_RETURN dec_tick_cb(SSL *s, SSL_SESSION *ss,
                                     const unsigned char *keyname,
                                     size_t keyname_length,
                                     SSL_TICKET_STATUS status,
                                     void *arg)
{
    void *tickdata;
    size_t tickdlen;

    dec_tick_called = 1;

    if (status == SSL_TICKET_EMPTY)
        return SSL_TICKET_RETURN_IGNORE_RENEW;

    if (!TEST_true(status == SSL_TICKET_SUCCESS
                   || status == SSL_TICKET_SUCCESS_RENEW))
        return SSL_TICKET_RETURN_ABORT;

    if (!TEST_true(SSL_SESSION_get0_ticket_appdata(ss, &tickdata,
                                                   &tickdlen))
            || !TEST_size_t_eq(tickdlen, strlen(appdata))
            || !TEST_int_eq(memcmp(tickdata, appdata, tickdlen), 0))
        return SSL_TICKET_RETURN_ABORT;

    if (tick_key_cb_called)  {
        /* Don't change what the ticket key callback wanted to do */
        switch (status) {
        case SSL_TICKET_NO_DECRYPT:
            return SSL_TICKET_RETURN_IGNORE_RENEW;

        case SSL_TICKET_SUCCESS:
            return SSL_TICKET_RETURN_USE;

        case SSL_TICKET_SUCCESS_RENEW:
            return SSL_TICKET_RETURN_USE_RENEW;

        default:
            return SSL_TICKET_RETURN_ABORT;
        }
    }
    return tick_dec_ret;

}

#ifndef OPENSSL_NO_DEPRECATED_3_0
static int tick_key_cb(SSL *s, unsigned char key_name[16],
                       unsigned char iv[EVP_MAX_IV_LENGTH], EVP_CIPHER_CTX *ctx,
                       HMAC_CTX *hctx, int enc)
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
            || !EVP_CipherInit_ex(ctx, aes128cbc, NULL, tick_aes_key, iv, enc)
            || !HMAC_Init_ex(hctx, tick_hmac_key, sizeof(tick_hmac_key), sha256,
                             NULL))
        ret = -1;
    else
        ret = tick_key_renew ? 2 : 1;

    EVP_CIPHER_free(aes128cbc);
    EVP_MD_free(sha256);

    return ret;
}
#endif

static int tick_key_evp_cb(SSL *s, unsigned char key_name[16],
                           unsigned char iv[EVP_MAX_IV_LENGTH],
                           EVP_CIPHER_CTX *ctx, EVP_MAC_CTX *hctx, int enc)
{
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
    params[1] = OSSL_PARAM_construct_end();
    if (aes128cbc == NULL
            || !EVP_CipherInit_ex(ctx, aes128cbc, NULL, tick_aes_key, iv, enc)
            || !EVP_MAC_init(hctx, tick_hmac_key, sizeof(tick_hmac_key),
                             params))
        ret = -1;
    else
        ret = tick_key_renew ? 2 : 1;

    EVP_CIPHER_free(aes128cbc);

    return ret;
}

/*
 * Test the various ticket callbacks
 * Test 0: TLSv1.2, no ticket key callback, no ticket, no renewal
 * Test 1: TLSv1.3, no ticket key callback, no ticket, no renewal
 * Test 2: TLSv1.2, no ticket key callback, no ticket, renewal
 * Test 3: TLSv1.3, no ticket key callback, no ticket, renewal
 * Test 4: TLSv1.2, no ticket key callback, ticket, no renewal
 * Test 5: TLSv1.3, no ticket key callback, ticket, no renewal
 * Test 6: TLSv1.2, no ticket key callback, ticket, renewal
 * Test 7: TLSv1.3, no ticket key callback, ticket, renewal
 * Test 8: TLSv1.2, old ticket key callback, ticket, no renewal
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
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *clntsess = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst % 2 == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst % 2 == 1)
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
    switch (tst) {
    case 0:
    case 1:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE;
        break;

    case 2:
    case 3:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE_RENEW;
        break;

    case 4:
    case 5:
        tick_dec_ret = SSL_TICKET_RETURN_USE;
        break;

    case 6:
    case 7:
        tick_dec_ret = SSL_TICKET_RETURN_USE_RENEW;
        break;

    default:
        tick_dec_ret = SSL_TICKET_RETURN_ABORT;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       ((tst % 2) == 0) ? TLS1_2_VERSION
                                                        : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * We only want sessions to resume from tickets - not the session cache. So
     * switch the cache off.
     */
    if (!TEST_true(SSL_CTX_set_session_cache_mode(sctx, SSL_SESS_CACHE_OFF)))
        goto end;

    if (!TEST_true(SSL_CTX_set_session_ticket_cb(sctx, gen_tick_cb, dec_tick_cb,
                                                 NULL)))
        goto end;

    if (tst >= 14) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_evp_cb(sctx, tick_key_evp_cb)))
            goto end;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    } else if (tst >= 8) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_cb(sctx, tick_key_cb)))
            goto end;
#endif
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * The decrypt ticket key callback in TLSv1.2 should be called even though
     * we have no ticket yet, because it gets called with a status of
     * SSL_TICKET_EMPTY (the client indicates support for tickets but does not
     * actually send any ticket data). This does not happen in TLSv1.3 because
     * it is not valid to send empty ticket data in TLSv1.3.
     */
    if (!TEST_int_eq(gen_tick_called, 1)
            || !TEST_int_eq(dec_tick_called, ((tst % 2) == 0) ? 1 : 0))
        goto end;

    gen_tick_called = dec_tick_called = 0;

    clntsess = SSL_get1_session(clientssl);
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
    serverssl = clientssl = NULL;

    /* Now do a resumption */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL))
            || !TEST_true(SSL_set_session(clientssl, clntsess))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
            || tick_key_renew == -1) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
            goto end;
    }

    if (!TEST_int_eq(gen_tick_called,
                     (tick_key_renew
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(clntsess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test incorrect shutdown.
 * Test 0: client does not shutdown properly,
 *         server does not set SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_SSL
 * Test 1: client does not shutdown properly,
 *         server sets SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_ZERO_RETURN
 */
static int test_incorrect_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[80];
    BIO *c2s;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), 0, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 1)
        SSL_CTX_set_options(sctx, SSL_OP_IGNORE_UNEXPECTED_EOF);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                            NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE)))
        goto end;

    c2s = SSL_get_rbio(serverssl);
    BIO_set_mem_eof_return(c2s, 0);

    if (!TEST_false(SSL_read(serverssl, buf, sizeof(buf))))
        goto end;

    if (tst == 0 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_SSL) )
        goto end;
    if (tst == 1 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_ZERO_RETURN) )
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test bi-directional shutdown.
 * Test 0: TLSv1.2
 * Test 1: TLSv1.2, server continues to read/write after client shutdown
 * Test 2: TLSv1.3, no pending NewSessionTicket messages
 * Test 3: TLSv1.3, pending NewSessionTicket messages
 * Test 4: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends key update, client reads it
 * Test 5: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends CertificateRequest, client reads and ignores it
 * Test 6: TLSv1.3, server continues to read/write after client shutdown, client
 *                  doesn't read it
 */
static int test_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char msg[] = "A test message";
    char buf[80];
    size_t written, readbytes;
    SSL_SESSION *sess;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 1)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 2)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 1) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 5)
        SSL_CTX_set_post_handshake_auth(cctx, 1);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst == 3) {
        if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                  SSL_ERROR_NONE, 1))
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_false(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE))
            || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))) {
        goto end;
    }

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0))
        goto end;

    if (tst >= 4) {
        /*
         * Reading on the server after the client has sent close_notify should
         * fail and provide SSL_ERROR_ZERO_RETURN
         */
        if (!TEST_false(SSL_read_ex(serverssl, buf, sizeof(buf), &readbytes))
                || !TEST_int_eq(SSL_get_error(serverssl, 0),
                                SSL_ERROR_ZERO_RETURN)
                || !TEST_int_eq(SSL_get_shutdown(serverssl),
                                SSL_RECEIVED_SHUTDOWN)
                   /*
                    * Even though we're shutdown on receive we should still be
                    * able to write.
                    */
                || !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (tst == 4
                && !TEST_true(SSL_key_update(serverssl,
                                             SSL_KEY_UPDATE_REQUESTED)))
            goto end;
        if (tst == 5) {
            SSL_set_verify(serverssl, SSL_VERIFY_PEER, NULL);
            if (!TEST_true(SSL_verify_client_post_handshake(serverssl)))
                goto end;
        }
        if ((tst == 4 || tst == 5)
                && !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (!TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
        if (tst == 4 || tst == 5) {
            /* Should still be able to read data from server */
            if (!TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                       &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0)
                    || !TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                              &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0))
                goto end;
        }
    }

    /* Writing on the client after sending close_notify shouldn't be possible */
    if (!TEST_false(SSL_write_ex(clientssl, msg, sizeof(msg), &written)))
        goto end;

    if (tst < 4) {
        /*
         * For these tests the client has sent close_notify but it has not yet
         * been received by the server. The server has not sent close_notify
         * yet.
         */
        if (!TEST_int_eq(SSL_shutdown(serverssl), 0)
                   /*
                    * Writing on the server after sending close_notify shouldn't
                    * be possible.
                    */
                || !TEST_false(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
                || !TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess))
                || !TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
    } else if (tst == 4 || tst == 5) {
        /*
         * In this test the client has sent close_notify and it has been
         * received by the server which has responded with a close_notify. The
         * client needs to read the close_notify sent by the server.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else {
        /*
         * tst == 6
         *
         * The client has sent close_notify and is expecting a close_notify
         * back, but instead there is application data first. The shutdown
         * should fail with a fatal error.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), -1)
                || !TEST_int_eq(SSL_get_error(clientssl, -1), SSL_ERROR_SSL))
            goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
static int cert_cb_cnt;

static int cert_cb(SSL *s, void *arg)
{
    SSL_CTX *ctx = (SSL_CTX *)arg;
    BIO *in = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *x509 = NULL, *rootx = NULL;
    STACK_OF(X509) *chain = NULL;
    char *rootfile = NULL, *ecdsacert = NULL, *ecdsakey = NULL;
    int ret = 0;

    if (cert_cb_cnt == 0) {
        /* Suspend the handshake */
        cert_cb_cnt++;
        return -1;
    } else if (cert_cb_cnt == 1) {
        /*
         * Update the SSL_CTX, set the certificate and private key and then
         * continue the handshake normally.
         */
        if (ctx != NULL && !TEST_ptr(SSL_set_SSL_CTX(s, ctx)))
            return 0;

        if (!TEST_true(SSL_use_certificate_file(s, cert, SSL_FILETYPE_PEM))
                || !TEST_true(SSL_use_PrivateKey_file(s, privkey,
                                                      SSL_FILETYPE_PEM))
                || !TEST_true(SSL_check_private_key(s)))
            return 0;
        cert_cb_cnt++;
        return 1;
    } else if (cert_cb_cnt == 3) {
        int rv;

        rootfile = test_mk_file_path(certsdir, "rootcert.pem");
        ecdsacert = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
        ecdsakey = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
        if (!TEST_ptr(rootfile) || !TEST_ptr(ecdsacert) || !TEST_ptr(ecdsakey))
            goto out;
        chain = sk_X509_new_null();
        if (!TEST_ptr(chain))
            goto out;
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, rootfile), 0)
                || !TEST_ptr(rootx = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &rootx, NULL, NULL))
                || !TEST_true(sk_X509_push(chain, rootx)))
            goto out;
        rootx = NULL;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsacert), 0)
                || !TEST_ptr(x509 = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &x509, NULL, NULL)))
            goto out;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsakey), 0)
                || !TEST_ptr(pkey = PEM_read_bio_PrivateKey_ex(in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
            goto out;
        rv = SSL_check_chain(s, x509, pkey, chain);
        /*
         * If the cert doesn't show as valid here (e.g., because we don't
         * have any shared sigalgs), then we will not set it, and there will
         * be no certificate at all on the SSL or SSL_CTX.  This, in turn,
         * will cause tls_choose_sigalgs() to fail the connection.
         */
        if ((rv & (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE))
                == (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE)) {
            if (!SSL_use_cert_and_key(s, x509, pkey, NULL, 1))
                goto out;
        }

        ret = 1;
    }

    /* Abort the handshake */
 out:
    OPENSSL_free(ecdsacert);
    OPENSSL_free(ecdsakey);
    OPENSSL_free(rootfile);
    BIO_free(in);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    X509_free(rootx);
    sk_X509_pop_free(chain, X509_free);
    return ret;
}

/*
 * Test the certificate callback.
 * Test 0: Callback fails
 * Test 1: Success - no SSL_set_SSL_CTX() in the callback
 * Test 2: Success - SSL_set_SSL_CTX() in the callback
 * Test 3: Success - Call SSL_check_chain from the callback
 * Test 4: Failure - SSL_check_chain fails from callback due to bad cert in the
 *                   chain
 * Test 5: Failure - SSL_check_chain fails from callback due to bad ee cert
 */
static int test_cert_cb_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *snictx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0, ret;

#ifdef OPENSSL_NO_EC
    /* We use an EC cert in these tests, so we skip in a no-ec build */
    if (tst >= 3)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, NULL, NULL)))
        goto end;

    if (tst == 0)
        cert_cb_cnt = -1;
    else if (tst >= 3)
        cert_cb_cnt = 3;
    else
        cert_cb_cnt = 0;

    if (tst == 2) {
        snictx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
        if (!TEST_ptr(snictx))
            goto end;
    }

    SSL_CTX_set_cert_cb(sctx, cert_cb, snictx);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (tst == 4) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the chain doesn't meet (the root uses an RSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                                             "ecdsa_secp256r1_sha256")))
            goto end;
    } else if (tst == 5) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the ee cert doesn't meet (the ee uses an ECDSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                           "rsa_pss_rsae_sha256:rsa_pkcs1_sha256")))
            goto end;
    }

    ret = create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE);
    if (!TEST_true(tst == 0 || tst == 4 || tst == 5 ? !ret : ret)
            || (tst > 0
                && !TEST_int_eq((cert_cb_cnt - 2) * (cert_cb_cnt - 3), 0))) {
        goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    SSL_CTX_free(snictx);

    return testresult;
}
#endif

static int test_cert_cb(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_cert_cb_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_cert_cb_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

static int client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
    X509 *xcert;
    EVP_PKEY *privpkey;
    BIO *in = NULL;
    BIO *priv_in = NULL;

    /* Check that SSL_get0_peer_certificate() returns something sensible */
    if (!TEST_ptr(SSL_get0_peer_certificate(ssl)))
        return 0;

    in = BIO_new_file(cert, "r");
    if (!TEST_ptr(in))
        return 0;

    if (!TEST_ptr(xcert = X509_new_ex(libctx, NULL))
            || !TEST_ptr(PEM_read_bio_X509(in, &xcert, NULL, NULL))
            || !TEST_ptr(priv_in = BIO_new_file(privkey, "r"))
            || !TEST_ptr(privpkey = PEM_read_bio_PrivateKey_ex(priv_in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
        goto err;

    *x509 = xcert;
    *pkey = privpkey;

    BIO_free(in);
    BIO_free(priv_in);
    return 1;
err:
    X509_free(xcert);
    BIO_free(in);
    BIO_free(priv_in);
    return 0;
}

static int test_client_cert_cb(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst == 1)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       tst == 0 ? TLS1_2_VERSION
                                                : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * Test that setting a client_cert_cb results in a client certificate being
     * sent.
     */
    SSL_CTX_set_client_cert_cb(cctx, client_cert_cb);
    SSL_CTX_set_verify(sctx,
                       SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       verify_cb);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
/*
 * Test setting certificate authorities on both client and server.
 *
 * Test 0: SSL_CTX_set0_CA_list() only
 * Test 1: Both SSL_CTX_set0_CA_list() and SSL_CTX_set_client_CA_list()
 * Test 2: Only SSL_CTX_set_client_CA_list()
 */
static int test_ca_names_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    size_t i;
    X509_NAME *name[] = { NULL, NULL, NULL, NULL };
    char *strnames[] = { "Jack", "Jill", "John", "Joanne" };
    STACK_OF(X509_NAME) *sk1 = NULL, *sk2 = NULL;
    const STACK_OF(X509_NAME) *sktmp = NULL;

    for (i = 0; i < OSSL_NELEM(name); i++) {
        name[i] = X509_NAME_new();
        if (!TEST_ptr(name[i])
                || !TEST_true(X509_NAME_add_entry_by_txt(name[i], "CN",
                                                         MBSTRING_ASC,
                                                         (unsigned char *)
                                                         strnames[i],
                                                         -1, -1, 0)))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    SSL_CTX_set_verify(sctx, SSL_VERIFY_PEER, NULL);

    if (tst == 0 || tst == 1) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[1])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[1]))))
            goto end;

        SSL_CTX_set0_CA_list(sctx, sk1);
        SSL_CTX_set0_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }
    if (tst == 1 || tst == 2) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[3])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[3]))))
            goto end;

        SSL_CTX_set_client_CA_list(sctx, sk1);
        SSL_CTX_set_client_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * We only expect certificate authorities to have been sent to the server
     * if we are using TLSv1.3 and SSL_set0_CA_list() was used
     */
    sktmp = SSL_get0_peer_CA_list(serverssl);
    if (prot == TLS1_3_VERSION
            && (tst == 0 || tst == 1)) {
        if (!TEST_ptr(sktmp)
                || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                              name[0]), 0)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                              name[1]), 0))
            goto end;
    } else if (!TEST_ptr_null(sktmp)) {
        goto end;
    }

    /*
     * In all tests we expect certificate authorities to have been sent to the
     * client. However, SSL_set_client_CA_list() should override
     * SSL_set0_CA_list()
     */
    sktmp = SSL_get0_peer_CA_list(clientssl);
    if (!TEST_ptr(sktmp)
            || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                          name[tst == 0 ? 0 : 2]), 0)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                          name[tst == 0 ? 1 : 3]), 0))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    for (i = 0; i < OSSL_NELEM(name); i++)
        X509_NAME_free(name[i]);
    sk_X509_NAME_pop_free(sk1, X509_NAME_free);
    sk_X509_NAME_pop_free(sk2, X509_NAME_free);

    return testresult;
}
#endif

static int test_ca_names(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_ca_names_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_ca_names_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

#ifndef OPENSSL_NO_TLS1_2
static const char *multiblock_cipherlist_data[]=
{
    "AES128-SHA",
    "AES128-SHA256",
    "AES256-SHA",
    "AES256-SHA256",
};

/* Reduce the fragment size - so the multiblock test buffer can be small */
# define MULTIBLOCK_FRAGSIZE 512

static int test_multiblock_write(int test_index)
{
    static const char *fetchable_ciphers[]=
    {
        "AES-128-CBC-HMAC-SHA1",
        "AES-128-CBC-HMAC-SHA256",
        "AES-256-CBC-HMAC-SHA1",
        "AES-256-CBC-HMAC-SHA256"
    };
    const char *cipherlist = multiblock_cipherlist_data[test_index];
    const SSL_METHOD *smeth = TLS_server_method();
    const SSL_METHOD *cmeth = TLS_client_method();
    int min_version = TLS1_VERSION;
    int max_version = TLS1_2_VERSION; /* Don't select TLS1_3 */
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /*
     * Choose a buffer large enough to perform a multi-block operation
     * i.e: write_len >= 4 * frag_size
     * 9 * is chosen so that multiple multiblocks are used + some leftover.
     */
    unsigned char msg[MULTIBLOCK_FRAGSIZE * 9];
    unsigned char buf[sizeof(msg)], *p = buf;
    size_t readbytes, written, len;
    EVP_CIPHER *ciph = NULL;

    /*
     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
    EVP_CIPHER_free(ciph);

    /* Set up a buffer with some data that will be sent to the client */
    RAND_bytes(msg, sizeof(msg));

    if (!TEST_true(create_ssl_ctx_pair(libctx, smeth, cmeth, min_version,
                                       max_version, &sctx, &cctx, cert,
                                       privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_max_send_fragment(sctx, MULTIBLOCK_FRAGSIZE)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
            goto end;

    /* settings to force it to use AES-CBC-HMAC_SHA */
    SSL_set_options(serverssl, SSL_OP_NO_ENCRYPT_THEN_MAC);
    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, cipherlist)))
       goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
        || !TEST_size_t_eq(written, sizeof(msg)))
        goto end;

    len = written;
    while (len > 0) {
        if (!TEST_true(SSL_read_ex(clientssl, p, MULTIBLOCK_FRAGSIZE, &readbytes)))
            goto end;
        p += readbytes;
        len -= readbytes;
    }
    if (!TEST_mem_eq(msg, sizeof(msg), buf, sizeof(buf)))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* OPENSSL_NO_TLS1_2 */

static int test_session_timeout(int test)
{
    /*
     * Test session ordering and timeout
     * Can't explicitly test performance of the new code,
     * but can test to see if the ordering of the sessions
     * are correct, and they they are removed as expected
     */
    SSL_SESSION *early = NULL;
    SSL_SESSION *middle = NULL;
    SSL_SESSION *late = NULL;
    SSL_CTX *ctx;
    int testresult = 0;
    long now = (long)time(NULL);
#define TIMEOUT 10

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_method()))
        || !TEST_ptr(early = SSL_SESSION_new())
        || !TEST_ptr(middle = SSL_SESSION_new())
        || !TEST_ptr(late = SSL_SESSION_new()))
        goto end;

    /* assign unique session ids */
    early->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(early->session_id, 1, SSL3_SSL_SESSION_ID_LENGTH);
    middle->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(middle->session_id, 2, SSL3_SSL_SESSION_ID_LENGTH);
    late->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(late->session_id, 3, SSL3_SSL_SESSION_ID_LENGTH);

    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_time(early, now - 10), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(middle, now), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(late, now + 10), 0))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_timeout(early, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(middle, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(late, TIMEOUT), 0))
        goto end;

    /* Make sure they are all still there */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* Make sure they are in the expected order */
    if (!TEST_ptr_eq(late->next, middle)
        || !TEST_ptr_eq(middle->next, early)
        || !TEST_ptr_eq(early->prev, middle)
        || !TEST_ptr_eq(middle->prev, late))
        goto end;

    /* This should remove "early" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT - 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "middle" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "late" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 11);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    /* Add them back in again */
    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove all of them */
    SSL_CTX_flush_sessions(ctx, 0);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    (void)SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_UPDATE_TIME
                                         | SSL_CTX_get_session_cache_mode(ctx));

    /* make sure |now| is NOT  equal to the current time */
    now -= 10;
    if (!TEST_int_ne(SSL_SESSION_set_time(early, now), 0)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_long_ne(SSL_SESSION_get_time(early), now))
        goto end;

    testresult = 1;
 end:
    SSL_CTX_free(ctx);
    SSL_SESSION_free(early);
    SSL_SESSION_free(middle);
    SSL_SESSION_free(late);
    return testresult;
}

/*
 * Test 0: Client sets servername and server acknowledges it (TLSv1.2)
 * Test 1: Client sets servername and server does not acknowledge it (TLSv1.2)
 * Test 2: Client sets inconsistent servername on resumption (TLSv1.2)
 * Test 3: Client does not set servername on initial handshake (TLSv1.2)
 * Test 4: Client does not set servername on resumption handshake (TLSv1.2)
 * Test 5: Client sets servername and server acknowledges it (TLSv1.3)
 * Test 6: Client sets servername and server does not acknowledge it (TLSv1.3)
 * Test 7: Client sets inconsistent servername on resumption (TLSv1.3)
 * Test 8: Client does not set servername on initial handshake(TLSv1.3)
 * Test 9: Client does not set servername on resumption handshake (TLSv1.3)
 */
static int test_servername(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;
    const char *sexpectedhost = NULL, *cexpectedhost = NULL;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 4)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 5)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 4) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst != 1 && tst != 6) {
        if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx,
                                                              hostname_cb)))
            goto end;
    }

    if (tst != 3 && tst != 8) {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        sexpectedhost = cexpectedhost = "goodhost";
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(SSL_get_servername(clientssl, TLSEXT_NAMETYPE_host_name),
                     cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    /* Now repeat with a resumption handshake */

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0)
            || !TEST_ptr_ne(sess = SSL_get1_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))
            || !TEST_int_eq(SSL_shutdown(serverssl), 0))
        goto end;

    SSL_free(clientssl);
    SSL_free(serverssl);
    clientssl = serverssl = NULL;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL)))
        goto end;

    if (!TEST_true(SSL_set_session(clientssl, sess)))
        goto end;

    sexpectedhost = cexpectedhost = "goodhost";
    if (tst == 2 || tst == 7) {
        /* Set an inconsistent hostname */
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "altgoodhost")))
            goto end;
        /*
         * In TLSv1.2 we expect the hostname from the original handshake, in
         * TLSv1.3 we expect the hostname from this handshake
         */
        if (tst == 7)
            sexpectedhost = cexpectedhost = "altgoodhost";

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "altgoodhost"))
            goto end;
    } else if (tst == 4 || tst == 9) {
        /*
         * A TLSv1.3 session does not associate a session with a servername,
         * but a TLSv1.2 session does.
         */
        if (tst == 9)
            sexpectedhost = cexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         cexpectedhost))
            goto end;
    } else {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        /*
         * In a TLSv1.2 resumption where the hostname was not acknowledged
         * we expect the hostname on the server to be empty. On the client we
         * return what was requested in this case.
         *
         * Similarly if the client didn't set a hostname on an original TLSv1.2
         * session but is now, the server hostname will be empty, but the client
         * is as we set it.
         */
        if (tst == 1 || tst == 3)
            sexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "goodhost"))
            goto end;
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_session_reused(clientssl))
            || !TEST_true(SSL_session_reused(serverssl))
            || !TEST_str_eq(SSL_get_servername(clientssl,
                                               TLSEXT_NAMETYPE_host_name),
                            cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
/*
 * Test that if signature algorithms are not available, then we do not offer or
 * accept them.
 * Test 0: Two RSA sig algs available: both RSA sig algs shared
 * Test 1: The client only has SHA2-256: only SHA2-256 algorithms shared
 * Test 2: The server only has SHA2-256: only SHA2-256 algorithms shared
 * Test 3: An RSA and an ECDSA sig alg available: both sig algs shared
 * Test 4: The client only has an ECDSA sig alg: only ECDSA algorithms shared
 * Test 5: The server only has an ECDSA sig alg: only ECDSA algorithms shared
 */
static int test_sigalgs_available(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_LIB_CTX *clientctx = libctx, *serverctx = libctx;
    OSSL_PROVIDER *filterprov = NULL;
    int sig, hash;

    if (!TEST_ptr(tmpctx))
        goto end;

    if (idx != 0 && idx != 3) {
        if (!TEST_true(OSSL_PROVIDER_add_builtin(tmpctx, "filter",
                                                 filter_provider_init)))
            goto end;

        filterprov = OSSL_PROVIDER_load(tmpctx, "filter");
        if (!TEST_ptr(filterprov))
            goto end;

        if (idx < 3) {
            /*
             * Only enable SHA2-256 so rsa_pss_rsae_sha384 should not be offered
             * or accepted for the peer that uses this libctx. Note that libssl
             * *requires* SHA2-256 to be available so we cannot disable that. We
             * also need SHA1 for our certificate.
             */
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_DIGEST,
                                                      "SHA2-256:SHA1")))
                goto end;
        } else {
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_SIGNATURE,
                                                      "ECDSA"))
                    || !TEST_true(filter_provider_set_filter(OSSL_OP_KEYMGMT,
                                                             "EC:X25519:X448")))
                goto end;
        }

        if (idx == 1 || idx == 4)
            clientctx = tmpctx;
        else
            serverctx = tmpctx;
    }

    cctx = SSL_CTX_new_ex(clientctx, NULL, TLS_client_method());
    sctx = SSL_CTX_new_ex(serverctx, NULL, TLS_server_method());
    if (!TEST_ptr(cctx) || !TEST_ptr(sctx))
        goto end;

    if (idx != 5) {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;
    } else {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert2, privkey2)))
            goto end;
    }

    /* Ensure we only use TLSv1.2 ciphersuites based on SHA256 */
    if (idx < 4) {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-RSA-AES128-GCM-SHA256")))
            goto end;
    } else {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-ECDSA-AES128-GCM-SHA256")))
            goto end;
    }

    if (idx < 3) {
        if (!SSL_CTX_set1_sigalgs_list(cctx,
                                       "rsa_pss_rsae_sha384"
                                       ":rsa_pss_rsae_sha256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha384"
                                              ":rsa_pss_rsae_sha256"))
            goto end;
    } else {
        if (!SSL_CTX_set1_sigalgs_list(cctx, "rsa_pss_rsae_sha256:ECDSA+SHA256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha256:ECDSA+SHA256"))
            goto end;
    }

    if (idx != 5
        && (!TEST_int_eq(SSL_CTX_use_certificate_file(sctx, cert2,
                                                      SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_use_PrivateKey_file(sctx,
                                                        privkey2,
                                                        SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_check_private_key(sctx), 1)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    /* For tests 0 and 3 we expect 2 shared sigalgs, otherwise exactly 1 */
    if (!TEST_int_eq(SSL_get_shared_sigalgs(serverssl, 0, &sig, &hash, NULL,
                                            NULL, NULL),
                     (idx == 0 || idx == 3) ? 2 : 1))
        goto end;

    if (!TEST_int_eq(hash, idx == 0 ? NID_sha384 : NID_sha256))
        goto end;

    if (!TEST_int_eq(sig, (idx == 4 || idx == 5) ? EVP_PKEY_EC
                                                 : NID_rsassaPss))
        goto end;

    testresult = filter_provider_check_clean_finish();

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(filterprov);
    OSSL_LIB_CTX_free(tmpctx);

    return testresult;
}
#endif /*
        * !defined(OPENSSL_NO_EC) \
        * && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
        */

#ifndef OPENSSL_NO_TLS1_3
/* This test can run in TLSv1.3 even if ec and dh are disabled */
static int test_pluggable_group(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_PROVIDER *tlsprov = OSSL_PROVIDER_load(libctx, "tls-provider");
    /* Check that we are not impacted by a provider without any groups */
    OSSL_PROVIDER *legacyprov = OSSL_PROVIDER_load(libctx, "legacy");
    const char *group_name = idx == 0 ? "xorgroup" : "xorkemgroup";

    if (!TEST_ptr(tlsprov))
        goto end;

    if (legacyprov == NULL) {
        /*
         * In this case we assume we've been built with "no-legacy" and skip
         * this test (there is no OPENSSL_NO_LEGACY)
         */
        testresult = 1;
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION,
                                       TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set1_groups_list(serverssl, group_name))
            || !TEST_true(SSL_set1_groups_list(clientssl, group_name)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(group_name,
                     SSL_group_to_name(serverssl, SSL_get_shared_group(serverssl, 0))))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(tlsprov);
    OSSL_PROVIDER_unload(legacyprov);

    return testresult;
}
#endif

#ifndef OPENSSL_NO_TLS1_2
static int test_ssl_dup(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL, *client2ssl = NULL;
    int testresult = 0;
    BIO *rbio = NULL, *wbio = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_min_proto_version(clientssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(clientssl, TLS1_2_VERSION)))
        goto end;

    client2ssl = SSL_dup(clientssl);
    rbio = SSL_get_rbio(clientssl);
    if (!TEST_ptr(rbio)
            || !TEST_true(BIO_up_ref(rbio)))
        goto end;
    SSL_set0_rbio(client2ssl, rbio);
    rbio = NULL;

    wbio = SSL_get_wbio(clientssl);
    if (!TEST_ptr(wbio) || !TEST_true(BIO_up_ref(wbio)))
        goto end;
    SSL_set0_wbio(client2ssl, wbio);
    rbio = NULL;

    if (!TEST_ptr(client2ssl)
               /* Handshake not started so pointers should be different */
            || !TEST_ptr_ne(clientssl, client2ssl))
        goto end;

    if (!TEST_int_eq(SSL_get_min_proto_version(client2ssl), TLS1_2_VERSION)
            || !TEST_int_eq(SSL_get_max_proto_version(client2ssl), TLS1_2_VERSION))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, client2ssl, SSL_ERROR_NONE)))
        goto end;

    SSL_free(clientssl);
    clientssl = SSL_dup(client2ssl);
    if (!TEST_ptr(clientssl)
               /* Handshake has finished so pointers should be the same */
            || !TEST_ptr_eq(clientssl, client2ssl))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_free(client2ssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

# ifndef OPENSSL_NO_DH

static EVP_PKEY *tmp_dh_params = NULL;

/* Helper function for the test_set_tmp_dh() tests */
static EVP_PKEY *get_tmp_dh_params(void)
{
    if (tmp_dh_params == NULL) {
        BIGNUM *p = NULL;
        OSSL_PARAM_BLD *tmpl = NULL;
        EVP_PKEY_CTX *pctx = NULL;
        OSSL_PARAM *params = NULL;
        EVP_PKEY *dhpkey = NULL;

        p = BN_get_rfc3526_prime_2048(NULL);
        if (!TEST_ptr(p))
            goto end;

        pctx = EVP_PKEY_CTX_new_from_name(libctx, "DH", NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_fromdata_init(pctx), 1))
            goto end;

        tmpl = OSSL_PARAM_BLD_new();
        if (!TEST_ptr(tmpl)
                || !TEST_true(OSSL_PARAM_BLD_push_BN(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_P,
                                                        p))
                || !TEST_true(OSSL_PARAM_BLD_push_uint(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_G,
                                                        2)))
            goto end;

        params = OSSL_PARAM_BLD_to_param(tmpl);
        if (!TEST_ptr(params)
                || !TEST_int_eq(EVP_PKEY_fromdata(pctx, &dhpkey,
                                                  EVP_PKEY_KEY_PARAMETERS,
                                                  params), 1))
            goto end;

        tmp_dh_params = dhpkey;
    end:
        BN_free(p);
        EVP_PKEY_CTX_free(pctx);
        OSSL_PARAM_BLD_free(tmpl);
        OSSL_PARAM_free(params);
    }

    if (tmp_dh_params != NULL && !EVP_PKEY_up_ref(tmp_dh_params))
        return NULL;

    return tmp_dh_params;
}

#  ifndef OPENSSL_NO_DEPRECATED_3_0
/* Callback used by test_set_tmp_dh() */
static DH *tmp_dh_callback(SSL *s, int is_export, int keylen)
{
    EVP_PKEY *dhpkey = get_tmp_dh_params();
    DH *ret = NULL;

    if (!TEST_ptr(dhpkey))
        return NULL;

    /*
     * libssl does not free the returned DH, so we free it now knowing that even
     * after we free dhpkey, there will still be a reference to the owning
     * EVP_PKEY in tmp_dh_params, and so the DH object will live for the length
     * of time we need it for.
     */
    ret = EVP_PKEY_get1_DH(dhpkey);
    DH_free(ret);

    EVP_PKEY_free(dhpkey);

    return ret;
}
#  endif

/*
 * Test the various methods for setting temporary DH parameters
 *
 * Test  0: Default (no auto) setting
 * Test  1: Explicit SSL_CTX auto off
 * Test  2: Explicit SSL auto off
 * Test  3: Explicit SSL_CTX auto on
 * Test  4: Explicit SSL auto on
 * Test  5: Explicit SSL_CTX auto off, custom DH params via EVP_PKEY
 * Test  6: Explicit SSL auto off, custom DH params via EVP_PKEY
 *
 * The following are testing deprecated APIs, so we only run them if available
 * Test  7: Explicit SSL_CTX auto off, custom DH params via DH
 * Test  8: Explicit SSL auto off, custom DH params via DH
 * Test  9: Explicit SSL_CTX auto off, custom DH params via callback
 * Test 10: Explicit SSL auto off, custom DH params via callback
 */
static int test_set_tmp_dh(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int dhauto = (idx == 3 || idx == 4) ? 1 : 0;
    int expected = (idx <= 2) ? 0 : 1;
    EVP_PKEY *dhpkey = NULL;
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH *dh = NULL;
#  else

    if (idx >= 7)
        return 1;
#  endif

    if (idx >= 5 && idx <= 8) {
        dhpkey = get_tmp_dh_params();
        if (!TEST_ptr(dhpkey))
            goto end;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    if (idx == 7 || idx == 8) {
        dh = EVP_PKEY_get1_DH(dhpkey);
        if (!TEST_ptr(dh))
            goto end;
    }
#  endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if ((idx & 1) == 1) {
        if (!TEST_true(SSL_CTX_set_dh_auto(sctx, dhauto)))
            goto end;
    }

    if (idx == 5) {
        if (!TEST_true(SSL_CTX_set0_tmp_dh_pkey(sctx, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 7) {
        if (!TEST_true(SSL_CTX_set_tmp_dh(sctx, dh)))
            goto end;
    } else if (idx == 9) {
        SSL_CTX_set_tmp_dh_callback(sctx, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if ((idx & 1) == 0 && idx != 0) {
        if (!TEST_true(SSL_set_dh_auto(serverssl, dhauto)))
            goto end;
    }
    if (idx == 6) {
        if (!TEST_true(SSL_set0_tmp_dh_pkey(serverssl, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 8) {
        if (!TEST_true(SSL_set_tmp_dh(serverssl, dh)))
            goto end;
    } else if (idx == 10) {
        SSL_set_tmp_dh_callback(serverssl, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, "DHE-RSA-AES128-SHA")))
        goto end;

    /*
     * If autoon then we should succeed. Otherwise we expect failure because
     * there are no parameters
     */
    if (!TEST_int_eq(create_ssl_connection(serverssl, clientssl,
                                           SSL_ERROR_NONE), expected))
        goto end;

    testresult = 1;

 end:
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH_free(dh);
#  endif
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(dhpkey);

    return testresult;
}

/*
 * Test the auto DH keys are appropriately sized
 */
static int test_dh_auto(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    EVP_PKEY *tmpkey = NULL;
    char *thiscert = NULL, *thiskey = NULL;
    size_t expdhsize = 0;
    const char *ciphersuite = "DHE-RSA-AES128-SHA";

    switch (idx) {
    case 0:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        thiscert = cert1024;
        thiskey = privkey1024;
        expdhsize = 1024;
        break;
    case 1:
        /* 2048 bit prime */
        thiscert = cert;
        thiskey = privkey;
        expdhsize = 2048;
        break;
    case 2:
        thiscert = cert3072;
        thiskey = privkey3072;
        expdhsize = 3072;
        break;
    case 3:
        thiscert = cert4096;
        thiskey = privkey4096;
        expdhsize = 4096;
        break;
    case 4:
        thiscert = cert8192;
        thiskey = privkey8192;
        expdhsize = 8192;
        break;
    /* No certificate cases */
    case 5:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        ciphersuite = "ADH-AES128-SHA256:@SECLEVEL=0";
        expdhsize = 1024;
        break;
    case 6:
        ciphersuite = "ADH-AES256-SHA256:@SECLEVEL=0";
        expdhsize = 3072;
        break;
    default:
        TEST_error("Invalid text index");
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, thiscert, thiskey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_dh_auto(serverssl, 1))
            || !TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, ciphersuite))
            || !TEST_true(SSL_set_cipher_list(clientssl, ciphersuite)))
        goto end;

    /*
     * Send the server's first flight. At this point the server has created the
     * temporary DH key but hasn't finished using it yet. Once used it is
     * removed, so we cannot test it.
     */
    if (!TEST_int_le(SSL_connect(clientssl), 0)
            || !TEST_int_le(SSL_accept(serverssl), 0))
        goto end;

    if (!TEST_int_gt(SSL_get_tmp_key(serverssl, &tmpkey), 0))
        goto end;
    if (!TEST_size_t_eq(EVP_PKEY_get_bits(tmpkey), expdhsize))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(tmpkey);

    return testresult;

}
# endif /* OPENSSL_NO_DH */
#endif /* OPENSSL_NO_TLS1_2 */

#ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Test that setting an SNI callback works with TLSv1.3. Specifically we check
 * that it works even without a certificate configured for the original
 * SSL_CTX
 */
static int test_sni_tls13(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *sctx2 = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /* Reset callback counter */
    snicb = 0;

    /* Create an initial SSL_CTX with no certificate configured */
    sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(sctx))
        goto end;
    /* Require TLSv1.3 as a minimum */
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_3_VERSION, 0,
                                       &sctx2, &cctx, cert, privkey)))
        goto end;

    /* Set up SNI */
    if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx, sni_cb))
            || !TEST_true(SSL_CTX_set_tlsext_servername_arg(sctx, sctx2)))
        goto end;

    /*
     * Connection should still succeed because the final SSL_CTX has the right
     * certificates configured.
     */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /* We should have had the SNI callback called exactly once */
    if (!TEST_int_eq(snicb, 1))
        goto end;

    testresult = 1;

end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}

/*
 * Test that the lifetime hint of a TLSv1.3 ticket is no more than 1 week
 * 0 = TLSv1.2
 * 1 = TLSv1.3
 */
static int test_ticket_lifetime(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int version = TLS1_3_VERSION;

#define ONE_WEEK_SEC (7 * 24 * 60 * 60)
#define TWO_WEEK_SEC (2 * ONE_WEEK_SEC)

    if (idx == 0) {
#ifdef OPENSSL_NO_TLS1_2
        return TEST_skip("TLS 1.2 is disabled.");
#else
        version = TLS1_2_VERSION;
#endif
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), version, version,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL)))
        goto end;

    /*
     * Set the timeout to be more than 1 week
     * make sure the returned value is the default
     */
    if (!TEST_long_eq(SSL_CTX_set_timeout(sctx, TWO_WEEK_SEC),
                      SSL_get_default_timeout(serverssl)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (idx == 0) {
        /* TLSv1.2 uses the set value */
        if (!TEST_ulong_eq(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), TWO_WEEK_SEC))
            goto end;
    } else {
        /* TLSv1.3 uses the limited value */
        if (!TEST_ulong_le(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), ONE_WEEK_SEC))
            goto end;
    }
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
 * Test that setting an ALPN does not violate RFC
 */
static int test_set_alpn(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int testresult = 0;

    unsigned char bad0[] = { 0x00, 'b', 'a', 'd' };
    unsigned char good[] = { 0x04, 'g', 'o', 'o', 'd' };
    unsigned char bad1[] = { 0x01, 'b', 'a', 'd' };
    unsigned char bad2[] = { 0x03, 'b', 'a', 'd', 0x00};
    unsigned char bad3[] = { 0x03, 'b', 'a', 'd', 0x01, 'b', 'a', 'd'};
    unsigned char bad4[] = { 0x03, 'b', 'a', 'd', 0x06, 'b', 'a', 'd'};

    /* Create an initial SSL_CTX with no certificate configured */
    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    /* the set_alpn functions return 0 (false) on success, non-zero (true) on failure */
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, 0)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, good, 1)))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad4, sizeof(bad4))))
        goto end;

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    if (!TEST_false(SSL_set_alpn_protos(ssl, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, 0)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, good, 1)))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad4, sizeof(bad4))))
        goto end;

    testresult = 1;

end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return testresult;
}

/*
 * Test SSL_CTX_set1_verify/chain_cert_store and SSL_CTX_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl_ctx(void)
{
   SSL_CTX *ctx = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, new_store)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, NULL)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_CTX_free(ctx);
   return testresult;
}

/*
 * Test SSL_set1_verify/chain_cert_store and SSL_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl(void)
{
   SSL_CTX *ctx = NULL;
   SSL *ssl = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Create an SSL object. */
   ssl = SSL_new(ctx);
   if (!TEST_ptr(ssl))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, new_store)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, NULL)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_free(ssl);
   SSL_CTX_free(ctx);
   return testresult;
}


static int test_inherit_verify_param(void)
{
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    X509_VERIFY_PARAM *cp = NULL;
    SSL *ssl = NULL;
    X509_VERIFY_PARAM *sp = NULL;
    int hostflags = X509_CHECK_FLAG_NEVER_CHECK_SUBJECT;

    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    cp = SSL_CTX_get0_param(ctx);
    if (!TEST_ptr(cp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(cp), 0))
        goto end;

    X509_VERIFY_PARAM_set_hostflags(cp, hostflags);

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    sp = SSL_get0_param(ssl);
    if (!TEST_ptr(sp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(sp), hostflags))
        goto end;

    testresult = 1;

 end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return testresult;
}


static int test_load_dhfile(void)
{
#ifndef OPENSSL_NO_DH
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    SSL_CONF_CTX *cctx = NULL;

    if (dhfile == NULL)
        return 1;

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_client_method()))
        || !TEST_ptr(cctx = SSL_CONF_CTX_new()))
        goto end;

    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
    SSL_CONF_CTX_set_flags(cctx,
                           SSL_CONF_FLAG_CERTIFICATE
                           | SSL_CONF_FLAG_SERVER
                           | SSL_CONF_FLAG_FILE);

    if (!TEST_int_eq(SSL_CONF_cmd(cctx, "DHParameters", dhfile), 2))
        goto end;

    testresult = 1;
end:
    SSL_CONF_CTX_free(cctx);
    SSL_CTX_free(ctx);

    return testresult;
#else
    return TEST_skip("DH not supported by this build");
#endif
}

#ifndef OPENSSL_NO_QUIC
static int test_quic_set_encryption_secrets(SSL *ssl,
                                            OSSL_ENCRYPTION_LEVEL level,
                                            const uint8_t *read_secret,
                                            const uint8_t *write_secret,
                                            size_t secret_len)
{
    test_printf_stderr("quic_set_encryption_secrets() %s, lvl=%d, len=%zd\n",
                       ssl->server ? "server" : "client", level, secret_len);
    return 1;
}

static int test_quic_add_handshake_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                                        const uint8_t *data, size_t len)
{
    SSL *peer = (SSL*)SSL_get_app_data(ssl);

    TEST_info("quic_add_handshake_data() %s, lvl=%d, *data=0x%02X, len=%zd\n",
              ssl->server ? "server" : "client", level, (int)*data, len);
    if (!TEST_ptr(peer))
        return 0;

    /* We're called with what is locally written; this gives it to the peer */
    if (!TEST_true(SSL_provide_quic_data(peer, level, data, len))) {
        ERR_print_errors_fp(stderr);
        return 0;
    }

    return 1;
}

static int test_quic_flush_flight(SSL *ssl)
{
    test_printf_stderr("quic_flush_flight() %s\n", ssl->server ? "server" : "client");
    return 1;
}

static int test_quic_send_alert(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert)
{
    test_printf_stderr("quic_send_alert() %s, lvl=%d, alert=%d\n",
                       ssl->server ? "server" : "client", level, alert);
    return 1;
}

static SSL_QUIC_METHOD quic_method = {
    test_quic_set_encryption_secrets,
    test_quic_add_handshake_data,
    test_quic_flush_flight,
    test_quic_send_alert,
};

static int test_quic_api_set_versions(SSL *ssl, int ver)
{
    SSL_set_quic_transport_version(ssl, ver);
    return 1;
}

static int test_quic_api_version(int clnt, int srvr)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";
    const uint8_t *peer_str;
    size_t peer_str_len;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);

    if (!TEST_true(create_ssl_ctx_pair(libctx,
                                       TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION, 0,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_true(SSL_CTX_set_quic_method(cctx, &quic_method))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                             &clientssl, NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(serverssl, clientssl))
            || !TEST_true(SSL_set_app_data(clientssl, serverssl))
            || !TEST_true(test_quic_api_set_versions(clientssl, clnt))
            || !TEST_true(test_quic_api_set_versions(serverssl, srvr))
            || !TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                     SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_version(serverssl) == TLS1_3_VERSION)
            || !TEST_true(SSL_version(clientssl) == TLS1_3_VERSION)
            || !(TEST_int_eq(SSL_quic_read_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_read_level(serverssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(serverssl), ssl_encryption_application)))
        goto end;

    SSL_get_peer_quic_transport_params(serverssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, client_str, strlen(client_str)+1))
        goto end;
    SSL_get_peer_quic_transport_params(clientssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, server_str, strlen(server_str)+1))
        goto end;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(clientssl)))
        goto end;

    /* Dummy handshake call should succeed */
    if (!TEST_true(SSL_do_handshake(clientssl)))
        goto end;
    /* Test that we (correctly) fail to send KeyUpdate */
    if (!TEST_true(SSL_key_update(clientssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(clientssl), 0))
        goto end;
    if (!TEST_true(SSL_key_update(serverssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(serverssl), 0))
        goto end;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (srvr == 0 && clnt == 0)
        srvr = clnt = TLSEXT_TYPE_quic_transport_parameters;
    else if (srvr == 0)
        srvr = clnt;
    else if (clnt == 0)
        clnt = srvr;
    TEST_info("expected clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(serverssl), clnt))
        goto end;
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(clientssl), srvr))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_quic_api(int tst)
{
    SSL_CTX *sctx = NULL;
    SSL *serverssl = NULL;
    int testresult = 0;
    static int clnt_params[] = { 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int srvr_params[] = { 0,
                                 0,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int results[] = { 1, 1, 1, 1, 1, 0, 1, 0, 1 };

    /* Failure cases:
     * test 6/[5] clnt = parameters, srvr = draft
     * test 8/[7] clnt = draft, srvr = parameters
     */

    /* Clean up logging space */
    memset(client_log_buffer, 0, sizeof(client_log_buffer));
    memset(server_log_buffer, 0, sizeof(server_log_buffer));
    client_log_buffer_index = 0;
    server_log_buffer_index = 0;
    error_writing_log = 0;

    if (!TEST_ptr(sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method()))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_ptr(sctx->quic_method)
            || !TEST_ptr(serverssl = SSL_new(sctx))
            || !TEST_true(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, NULL))
            || !TEST_false(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, &quic_method))
            || !TEST_true(SSL_IS_QUIC(serverssl)))
        goto end;

    if (!TEST_int_eq(test_quic_api_version(clnt_params[tst], srvr_params[tst]), results[tst]))
        goto end;

    testresult = 1;

end:
    SSL_CTX_free(sctx);
    sctx = NULL;
    SSL_free(serverssl);
    serverssl = NULL;
    return testresult;
}

# ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Helper method to setup objects for QUIC early data test. Caller
 * frees objects on error.
 */
static int quic_setupearly_data_test(SSL_CTX **cctx, SSL_CTX **sctx,
                                     SSL **clientssl, SSL **serverssl,
                                     SSL_SESSION **sess, int idx)
{
    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";

    if (*sctx == NULL
            && (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                               TLS_client_method(),
                                               TLS1_3_VERSION, 0,
                                               sctx, cctx, cert, privkey))
                || !TEST_true(SSL_CTX_set_quic_method(*sctx, &quic_method))
                || !TEST_true(SSL_CTX_set_quic_method(*cctx, &quic_method))
                || !TEST_true(SSL_CTX_set_max_early_data(*sctx, 0xffffffffu))))
        return 0;

    if (idx == 1) {
        /* When idx == 1 we repeat the tests with read_ahead set */
        SSL_CTX_set_read_ahead(*cctx, 1);
        SSL_CTX_set_read_ahead(*sctx, 1);
    } else if (idx == 2) {
        /* When idx == 2 we are doing early_data with a PSK. Set up callbacks */
        SSL_CTX_set_psk_use_session_callback(*cctx, use_session_cb);
        SSL_CTX_set_psk_find_session_callback(*sctx, find_session_cb);
        use_session_cb_cnt = 0;
        find_session_cb_cnt = 0;
        srvid = pskid;
    }

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl, clientssl,
                                      NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    /*
     * For one of the run throughs (doesn't matter which one), we'll try sending
     * some SNI data in the initial ClientHello. This will be ignored (because
     * there is no SNI cb set up by the server), so it should not impact
     * early_data.
     */
    if (idx == 1
            && !TEST_true(SSL_set_tlsext_host_name(*clientssl, "localhost")))
        return 0;

    if (idx == 2) {
        clientpsk = create_a_psk(*clientssl);
        if (!TEST_ptr(clientpsk)
                || !TEST_true(SSL_SESSION_set_max_early_data(clientpsk,
                                                             0xffffffffu))
                || !TEST_true(SSL_SESSION_up_ref(clientpsk))) {
            SSL_SESSION_free(clientpsk);
            clientpsk = NULL;
            return 0;
        }
        serverpsk = clientpsk;

        if (sess != NULL) {
            if (!TEST_true(SSL_SESSION_up_ref(clientpsk))) {
                SSL_SESSION_free(clientpsk);
                SSL_SESSION_free(serverpsk);
                clientpsk = serverpsk = NULL;
                return 0;
            }
            *sess = clientpsk;
        }

        SSL_set_quic_early_data_enabled(*serverssl, 1);
        SSL_set_quic_early_data_enabled(*clientssl, 1);

        return 1;
    }

    if (sess == NULL)
        return 1;

    if (!TEST_true(create_bare_ssl_connection(*serverssl, *clientssl,
                                              SSL_ERROR_NONE, 0)))
        return 0;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(*clientssl)))
        return 0;

    *sess = SSL_get1_session(*clientssl);
    SSL_shutdown(*clientssl);
    SSL_shutdown(*serverssl);
    SSL_free(*serverssl);
    SSL_free(*clientssl);
    *serverssl = *clientssl = NULL;

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl,
                                      clientssl, NULL, NULL))
            || !TEST_true(SSL_set_session(*clientssl, *sess))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    SSL_set_quic_early_data_enabled(*serverssl, 1);
    SSL_set_quic_early_data_enabled(*clientssl, 1);

    return 1;
}

static int test_quic_early_data(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;

    if (!TEST_true(quic_setupearly_data_test(&cctx, &sctx, &clientssl,
                                             &serverssl, &sess, tst)))
        goto end;

    if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_get_early_data_status(serverssl)))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_SESSION_free(clientpsk);
    SSL_SESSION_free(serverpsk);
    clientpsk = serverpsk = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}
# endif /* OSSL_NO_USABLE_TLS1_3 */
#endif /* OPENSSL_NO_QUIC */

OPT_TEST_DECLARE_USAGE("certfile privkeyfile srpvfile tmpfile provider config dhfile\n")

int setup_tests(void)
{
    char *modulename;
    char *configfile;

    libctx = OSSL_LIB_CTX_new();
    if (!TEST_ptr(libctx))
        return 0;

    defctxnull = OSSL_PROVIDER_load(NULL, "null");

    /*
     * Verify that the default and fips providers in the default libctx are not
     * available
     */
    if (!TEST_false(OSSL_PROVIDER_available(NULL, "default"))
            || !TEST_false(OSSL_PROVIDER_available(NULL, "fips")))
        return 0;

    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(certsdir = test_get_argument(0))
            || !TEST_ptr(srpvfile = test_get_argument(1))
            || !TEST_ptr(tmpfilename = test_get_argument(2))
            || !TEST_ptr(modulename = test_get_argument(3))
            || !TEST_ptr(configfile = test_get_argument(4))
            || !TEST_ptr(dhfile = test_get_argument(5)))
        return 0;

    if (!TEST_true(OSSL_LIB_CTX_load_config(libctx, configfile)))
        return 0;

    /* Check we have the expected provider available */
    if (!TEST_true(OSSL_PROVIDER_available(libctx, modulename)))
        return 0;

    /* Check the default provider is not available */
    if (strcmp(modulename, "default") != 0
            && !TEST_false(OSSL_PROVIDER_available(libctx, "default")))
        return 0;

    if (strcmp(modulename, "fips") == 0)
        is_fips = 1;

    /*
     * We add, but don't load the test "tls-provider". We'll load it when we
     * need it.
     */
    if (!TEST_true(OSSL_PROVIDER_add_builtin(libctx, "tls-provider",
                                             tls_provider_init)))
        return 0;


    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
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
{
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
    params[1] = OSSL_PARAM_construct_end();
    if (aes128cbc == NULL
            || !EVP_CipherInit_ex(ctx, aes128cbc, NULL, tick_aes_key, iv, enc)
            || !EVP_MAC_init(hctx, tick_hmac_key, sizeof(tick_hmac_key),
                             params))
        ret = -1;
    else
        ret = tick_key_renew ? 2 : 1;

    EVP_CIPHER_free(aes128cbc);

    return ret;
}

/*
 * Test the various ticket callbacks
 * Test 0: TLSv1.2, no ticket key callback, no ticket, no renewal
 * Test 1: TLSv1.3, no ticket key callback, no ticket, no renewal
 * Test 2: TLSv1.2, no ticket key callback, no ticket, renewal
 * Test 3: TLSv1.3, no ticket key callback, no ticket, renewal
 * Test 4: TLSv1.2, no ticket key callback, ticket, no renewal
 * Test 5: TLSv1.3, no ticket key callback, ticket, no renewal
 * Test 6: TLSv1.2, no ticket key callback, ticket, renewal
 * Test 7: TLSv1.3, no ticket key callback, ticket, renewal
 * Test 8: TLSv1.2, old ticket key callback, ticket, no renewal
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
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *clntsess = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst % 2 == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst % 2 == 1)
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
    switch (tst) {
    case 0:
    case 1:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE;
        break;

    case 2:
    case 3:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE_RENEW;
        break;

    case 4:
    case 5:
        tick_dec_ret = SSL_TICKET_RETURN_USE;
        break;

    case 6:
    case 7:
        tick_dec_ret = SSL_TICKET_RETURN_USE_RENEW;
        break;

    default:
        tick_dec_ret = SSL_TICKET_RETURN_ABORT;
    }
{
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
    params[1] = OSSL_PARAM_construct_end();
    if (aes128cbc == NULL
            || !EVP_CipherInit_ex(ctx, aes128cbc, NULL, tick_aes_key, iv, enc)
            || !EVP_MAC_init(hctx, tick_hmac_key, sizeof(tick_hmac_key),
                             params))
        ret = -1;
    else
        ret = tick_key_renew ? 2 : 1;

    EVP_CIPHER_free(aes128cbc);

    return ret;
}

/*
 * Test the various ticket callbacks
 * Test 0: TLSv1.2, no ticket key callback, no ticket, no renewal
 * Test 1: TLSv1.3, no ticket key callback, no ticket, no renewal
 * Test 2: TLSv1.2, no ticket key callback, no ticket, renewal
 * Test 3: TLSv1.3, no ticket key callback, no ticket, renewal
 * Test 4: TLSv1.2, no ticket key callback, ticket, no renewal
 * Test 5: TLSv1.3, no ticket key callback, ticket, no renewal
 * Test 6: TLSv1.2, no ticket key callback, ticket, renewal
 * Test 7: TLSv1.3, no ticket key callback, ticket, renewal
 * Test 8: TLSv1.2, old ticket key callback, ticket, no renewal
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
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *clntsess = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst % 2 == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst % 2 == 1)
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
    switch (tst) {
    case 0:
    case 1:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE;
        break;

    case 2:
    case 3:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE_RENEW;
        break;

    case 4:
    case 5:
        tick_dec_ret = SSL_TICKET_RETURN_USE;
        break;

    case 6:
    case 7:
        tick_dec_ret = SSL_TICKET_RETURN_USE_RENEW;
        break;

    default:
        tick_dec_ret = SSL_TICKET_RETURN_ABORT;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       ((tst % 2) == 0) ? TLS1_2_VERSION
                                                        : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * We only want sessions to resume from tickets - not the session cache. So
     * switch the cache off.
     */
    if (!TEST_true(SSL_CTX_set_session_cache_mode(sctx, SSL_SESS_CACHE_OFF)))
        goto end;

    if (!TEST_true(SSL_CTX_set_session_ticket_cb(sctx, gen_tick_cb, dec_tick_cb,
                                                 NULL)))
        goto end;

    if (tst >= 14) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_evp_cb(sctx, tick_key_evp_cb)))
            goto end;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    } else if (tst >= 8) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_cb(sctx, tick_key_cb)))
            goto end;
#endif
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * The decrypt ticket key callback in TLSv1.2 should be called even though
     * we have no ticket yet, because it gets called with a status of
     * SSL_TICKET_EMPTY (the client indicates support for tickets but does not
     * actually send any ticket data). This does not happen in TLSv1.3 because
     * it is not valid to send empty ticket data in TLSv1.3.
     */
    if (!TEST_int_eq(gen_tick_called, 1)
            || !TEST_int_eq(dec_tick_called, ((tst % 2) == 0) ? 1 : 0))
        goto end;

    gen_tick_called = dec_tick_called = 0;

    clntsess = SSL_get1_session(clientssl);
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
    serverssl = clientssl = NULL;

    /* Now do a resumption */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL))
            || !TEST_true(SSL_set_session(clientssl, clntsess))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
            || tick_key_renew == -1) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
            goto end;
    }

    if (!TEST_int_eq(gen_tick_called,
                     (tick_key_renew
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(clntsess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test incorrect shutdown.
 * Test 0: client does not shutdown properly,
 *         server does not set SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_SSL
 * Test 1: client does not shutdown properly,
 *         server sets SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_ZERO_RETURN
 */
static int test_incorrect_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[80];
    BIO *c2s;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), 0, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 1)
        SSL_CTX_set_options(sctx, SSL_OP_IGNORE_UNEXPECTED_EOF);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                            NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE)))
        goto end;

    c2s = SSL_get_rbio(serverssl);
    BIO_set_mem_eof_return(c2s, 0);

    if (!TEST_false(SSL_read(serverssl, buf, sizeof(buf))))
        goto end;

    if (tst == 0 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_SSL) )
        goto end;
    if (tst == 1 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_ZERO_RETURN) )
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test bi-directional shutdown.
 * Test 0: TLSv1.2
 * Test 1: TLSv1.2, server continues to read/write after client shutdown
 * Test 2: TLSv1.3, no pending NewSessionTicket messages
 * Test 3: TLSv1.3, pending NewSessionTicket messages
 * Test 4: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends key update, client reads it
 * Test 5: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends CertificateRequest, client reads and ignores it
 * Test 6: TLSv1.3, server continues to read/write after client shutdown, client
 *                  doesn't read it
 */
static int test_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char msg[] = "A test message";
    char buf[80];
    size_t written, readbytes;
    SSL_SESSION *sess;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 1)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 2)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 1) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 5)
        SSL_CTX_set_post_handshake_auth(cctx, 1);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst == 3) {
        if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                  SSL_ERROR_NONE, 1))
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_false(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE))
            || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))) {
        goto end;
    }

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0))
        goto end;

    if (tst >= 4) {
        /*
         * Reading on the server after the client has sent close_notify should
         * fail and provide SSL_ERROR_ZERO_RETURN
         */
        if (!TEST_false(SSL_read_ex(serverssl, buf, sizeof(buf), &readbytes))
                || !TEST_int_eq(SSL_get_error(serverssl, 0),
                                SSL_ERROR_ZERO_RETURN)
                || !TEST_int_eq(SSL_get_shutdown(serverssl),
                                SSL_RECEIVED_SHUTDOWN)
                   /*
                    * Even though we're shutdown on receive we should still be
                    * able to write.
                    */
                || !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (tst == 4
                && !TEST_true(SSL_key_update(serverssl,
                                             SSL_KEY_UPDATE_REQUESTED)))
            goto end;
        if (tst == 5) {
            SSL_set_verify(serverssl, SSL_VERIFY_PEER, NULL);
            if (!TEST_true(SSL_verify_client_post_handshake(serverssl)))
                goto end;
        }
        if ((tst == 4 || tst == 5)
                && !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (!TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
        if (tst == 4 || tst == 5) {
            /* Should still be able to read data from server */
            if (!TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                       &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0)
                    || !TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                              &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0))
                goto end;
        }
    }

    /* Writing on the client after sending close_notify shouldn't be possible */
    if (!TEST_false(SSL_write_ex(clientssl, msg, sizeof(msg), &written)))
        goto end;

    if (tst < 4) {
        /*
         * For these tests the client has sent close_notify but it has not yet
         * been received by the server. The server has not sent close_notify
         * yet.
         */
        if (!TEST_int_eq(SSL_shutdown(serverssl), 0)
                   /*
                    * Writing on the server after sending close_notify shouldn't
                    * be possible.
                    */
                || !TEST_false(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
                || !TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess))
                || !TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
    } else if (tst == 4 || tst == 5) {
        /*
         * In this test the client has sent close_notify and it has been
         * received by the server which has responded with a close_notify. The
         * client needs to read the close_notify sent by the server.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else {
        /*
         * tst == 6
         *
         * The client has sent close_notify and is expecting a close_notify
         * back, but instead there is application data first. The shutdown
         * should fail with a fatal error.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), -1)
                || !TEST_int_eq(SSL_get_error(clientssl, -1), SSL_ERROR_SSL))
            goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
static int cert_cb_cnt;

static int cert_cb(SSL *s, void *arg)
{
    SSL_CTX *ctx = (SSL_CTX *)arg;
    BIO *in = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *x509 = NULL, *rootx = NULL;
    STACK_OF(X509) *chain = NULL;
    char *rootfile = NULL, *ecdsacert = NULL, *ecdsakey = NULL;
    int ret = 0;

    if (cert_cb_cnt == 0) {
        /* Suspend the handshake */
        cert_cb_cnt++;
        return -1;
    } else if (cert_cb_cnt == 1) {
        /*
         * Update the SSL_CTX, set the certificate and private key and then
         * continue the handshake normally.
         */
        if (ctx != NULL && !TEST_ptr(SSL_set_SSL_CTX(s, ctx)))
            return 0;

        if (!TEST_true(SSL_use_certificate_file(s, cert, SSL_FILETYPE_PEM))
                || !TEST_true(SSL_use_PrivateKey_file(s, privkey,
                                                      SSL_FILETYPE_PEM))
                || !TEST_true(SSL_check_private_key(s)))
            return 0;
        cert_cb_cnt++;
        return 1;
    } else if (cert_cb_cnt == 3) {
        int rv;

        rootfile = test_mk_file_path(certsdir, "rootcert.pem");
        ecdsacert = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
        ecdsakey = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
        if (!TEST_ptr(rootfile) || !TEST_ptr(ecdsacert) || !TEST_ptr(ecdsakey))
            goto out;
        chain = sk_X509_new_null();
        if (!TEST_ptr(chain))
            goto out;
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, rootfile), 0)
                || !TEST_ptr(rootx = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &rootx, NULL, NULL))
                || !TEST_true(sk_X509_push(chain, rootx)))
            goto out;
        rootx = NULL;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsacert), 0)
                || !TEST_ptr(x509 = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &x509, NULL, NULL)))
            goto out;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsakey), 0)
                || !TEST_ptr(pkey = PEM_read_bio_PrivateKey_ex(in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
            goto out;
        rv = SSL_check_chain(s, x509, pkey, chain);
        /*
         * If the cert doesn't show as valid here (e.g., because we don't
         * have any shared sigalgs), then we will not set it, and there will
         * be no certificate at all on the SSL or SSL_CTX.  This, in turn,
         * will cause tls_choose_sigalgs() to fail the connection.
         */
        if ((rv & (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE))
                == (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE)) {
            if (!SSL_use_cert_and_key(s, x509, pkey, NULL, 1))
                goto out;
        }

        ret = 1;
    }

    /* Abort the handshake */
 out:
    OPENSSL_free(ecdsacert);
    OPENSSL_free(ecdsakey);
    OPENSSL_free(rootfile);
    BIO_free(in);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    X509_free(rootx);
    sk_X509_pop_free(chain, X509_free);
    return ret;
}

/*
 * Test the certificate callback.
 * Test 0: Callback fails
 * Test 1: Success - no SSL_set_SSL_CTX() in the callback
 * Test 2: Success - SSL_set_SSL_CTX() in the callback
 * Test 3: Success - Call SSL_check_chain from the callback
 * Test 4: Failure - SSL_check_chain fails from callback due to bad cert in the
 *                   chain
 * Test 5: Failure - SSL_check_chain fails from callback due to bad ee cert
 */
static int test_cert_cb_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *snictx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0, ret;

#ifdef OPENSSL_NO_EC
    /* We use an EC cert in these tests, so we skip in a no-ec build */
    if (tst >= 3)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, NULL, NULL)))
        goto end;

    if (tst == 0)
        cert_cb_cnt = -1;
    else if (tst >= 3)
        cert_cb_cnt = 3;
    else
        cert_cb_cnt = 0;

    if (tst == 2) {
        snictx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
        if (!TEST_ptr(snictx))
            goto end;
    }

    SSL_CTX_set_cert_cb(sctx, cert_cb, snictx);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (tst == 4) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the chain doesn't meet (the root uses an RSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                                             "ecdsa_secp256r1_sha256")))
            goto end;
    } else if (tst == 5) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the ee cert doesn't meet (the ee uses an ECDSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                           "rsa_pss_rsae_sha256:rsa_pkcs1_sha256")))
            goto end;
    }

    ret = create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE);
    if (!TEST_true(tst == 0 || tst == 4 || tst == 5 ? !ret : ret)
            || (tst > 0
                && !TEST_int_eq((cert_cb_cnt - 2) * (cert_cb_cnt - 3), 0))) {
        goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    SSL_CTX_free(snictx);

    return testresult;
}
#endif

static int test_cert_cb(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_cert_cb_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_cert_cb_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

static int client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
    X509 *xcert;
    EVP_PKEY *privpkey;
    BIO *in = NULL;
    BIO *priv_in = NULL;

    /* Check that SSL_get0_peer_certificate() returns something sensible */
    if (!TEST_ptr(SSL_get0_peer_certificate(ssl)))
        return 0;

    in = BIO_new_file(cert, "r");
    if (!TEST_ptr(in))
        return 0;

    if (!TEST_ptr(xcert = X509_new_ex(libctx, NULL))
            || !TEST_ptr(PEM_read_bio_X509(in, &xcert, NULL, NULL))
            || !TEST_ptr(priv_in = BIO_new_file(privkey, "r"))
            || !TEST_ptr(privpkey = PEM_read_bio_PrivateKey_ex(priv_in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
        goto err;

    *x509 = xcert;
    *pkey = privpkey;

    BIO_free(in);
    BIO_free(priv_in);
    return 1;
err:
    X509_free(xcert);
    BIO_free(in);
    BIO_free(priv_in);
    return 0;
}

static int test_client_cert_cb(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst == 1)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       tst == 0 ? TLS1_2_VERSION
                                                : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * Test that setting a client_cert_cb results in a client certificate being
     * sent.
     */
    SSL_CTX_set_client_cert_cb(cctx, client_cert_cb);
    SSL_CTX_set_verify(sctx,
                       SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       verify_cb);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
/*
 * Test setting certificate authorities on both client and server.
 *
 * Test 0: SSL_CTX_set0_CA_list() only
 * Test 1: Both SSL_CTX_set0_CA_list() and SSL_CTX_set_client_CA_list()
 * Test 2: Only SSL_CTX_set_client_CA_list()
 */
static int test_ca_names_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    size_t i;
    X509_NAME *name[] = { NULL, NULL, NULL, NULL };
    char *strnames[] = { "Jack", "Jill", "John", "Joanne" };
    STACK_OF(X509_NAME) *sk1 = NULL, *sk2 = NULL;
    const STACK_OF(X509_NAME) *sktmp = NULL;

    for (i = 0; i < OSSL_NELEM(name); i++) {
        name[i] = X509_NAME_new();
        if (!TEST_ptr(name[i])
                || !TEST_true(X509_NAME_add_entry_by_txt(name[i], "CN",
                                                         MBSTRING_ASC,
                                                         (unsigned char *)
                                                         strnames[i],
                                                         -1, -1, 0)))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    SSL_CTX_set_verify(sctx, SSL_VERIFY_PEER, NULL);

    if (tst == 0 || tst == 1) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[1])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[1]))))
            goto end;

        SSL_CTX_set0_CA_list(sctx, sk1);
        SSL_CTX_set0_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }
    if (tst == 1 || tst == 2) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[3])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[3]))))
            goto end;

        SSL_CTX_set_client_CA_list(sctx, sk1);
        SSL_CTX_set_client_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * We only expect certificate authorities to have been sent to the server
     * if we are using TLSv1.3 and SSL_set0_CA_list() was used
     */
    sktmp = SSL_get0_peer_CA_list(serverssl);
    if (prot == TLS1_3_VERSION
            && (tst == 0 || tst == 1)) {
        if (!TEST_ptr(sktmp)
                || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                              name[0]), 0)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                              name[1]), 0))
            goto end;
    } else if (!TEST_ptr_null(sktmp)) {
        goto end;
    }

    /*
     * In all tests we expect certificate authorities to have been sent to the
     * client. However, SSL_set_client_CA_list() should override
     * SSL_set0_CA_list()
     */
    sktmp = SSL_get0_peer_CA_list(clientssl);
    if (!TEST_ptr(sktmp)
            || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                          name[tst == 0 ? 0 : 2]), 0)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                          name[tst == 0 ? 1 : 3]), 0))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    for (i = 0; i < OSSL_NELEM(name); i++)
        X509_NAME_free(name[i]);
    sk_X509_NAME_pop_free(sk1, X509_NAME_free);
    sk_X509_NAME_pop_free(sk2, X509_NAME_free);

    return testresult;
}
#endif

static int test_ca_names(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_ca_names_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_ca_names_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

#ifndef OPENSSL_NO_TLS1_2
static const char *multiblock_cipherlist_data[]=
{
    "AES128-SHA",
    "AES128-SHA256",
    "AES256-SHA",
    "AES256-SHA256",
};

/* Reduce the fragment size - so the multiblock test buffer can be small */
# define MULTIBLOCK_FRAGSIZE 512

static int test_multiblock_write(int test_index)
{
    static const char *fetchable_ciphers[]=
    {
        "AES-128-CBC-HMAC-SHA1",
        "AES-128-CBC-HMAC-SHA256",
        "AES-256-CBC-HMAC-SHA1",
        "AES-256-CBC-HMAC-SHA256"
    };
    const char *cipherlist = multiblock_cipherlist_data[test_index];
    const SSL_METHOD *smeth = TLS_server_method();
    const SSL_METHOD *cmeth = TLS_client_method();
    int min_version = TLS1_VERSION;
    int max_version = TLS1_2_VERSION; /* Don't select TLS1_3 */
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /*
     * Choose a buffer large enough to perform a multi-block operation
     * i.e: write_len >= 4 * frag_size
     * 9 * is chosen so that multiple multiblocks are used + some leftover.
     */
    unsigned char msg[MULTIBLOCK_FRAGSIZE * 9];
    unsigned char buf[sizeof(msg)], *p = buf;
    size_t readbytes, written, len;
    EVP_CIPHER *ciph = NULL;

    /*
     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
    EVP_CIPHER_free(ciph);

    /* Set up a buffer with some data that will be sent to the client */
    RAND_bytes(msg, sizeof(msg));

    if (!TEST_true(create_ssl_ctx_pair(libctx, smeth, cmeth, min_version,
                                       max_version, &sctx, &cctx, cert,
                                       privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_max_send_fragment(sctx, MULTIBLOCK_FRAGSIZE)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
            goto end;

    /* settings to force it to use AES-CBC-HMAC_SHA */
    SSL_set_options(serverssl, SSL_OP_NO_ENCRYPT_THEN_MAC);
    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, cipherlist)))
       goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
        || !TEST_size_t_eq(written, sizeof(msg)))
        goto end;

    len = written;
    while (len > 0) {
        if (!TEST_true(SSL_read_ex(clientssl, p, MULTIBLOCK_FRAGSIZE, &readbytes)))
            goto end;
        p += readbytes;
        len -= readbytes;
    }
    if (!TEST_mem_eq(msg, sizeof(msg), buf, sizeof(buf)))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* OPENSSL_NO_TLS1_2 */

static int test_session_timeout(int test)
{
    /*
     * Test session ordering and timeout
     * Can't explicitly test performance of the new code,
     * but can test to see if the ordering of the sessions
     * are correct, and they they are removed as expected
     */
    SSL_SESSION *early = NULL;
    SSL_SESSION *middle = NULL;
    SSL_SESSION *late = NULL;
    SSL_CTX *ctx;
    int testresult = 0;
    long now = (long)time(NULL);
#define TIMEOUT 10

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_method()))
        || !TEST_ptr(early = SSL_SESSION_new())
        || !TEST_ptr(middle = SSL_SESSION_new())
        || !TEST_ptr(late = SSL_SESSION_new()))
        goto end;

    /* assign unique session ids */
    early->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(early->session_id, 1, SSL3_SSL_SESSION_ID_LENGTH);
    middle->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(middle->session_id, 2, SSL3_SSL_SESSION_ID_LENGTH);
    late->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(late->session_id, 3, SSL3_SSL_SESSION_ID_LENGTH);

    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_time(early, now - 10), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(middle, now), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(late, now + 10), 0))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_timeout(early, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(middle, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(late, TIMEOUT), 0))
        goto end;

    /* Make sure they are all still there */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* Make sure they are in the expected order */
    if (!TEST_ptr_eq(late->next, middle)
        || !TEST_ptr_eq(middle->next, early)
        || !TEST_ptr_eq(early->prev, middle)
        || !TEST_ptr_eq(middle->prev, late))
        goto end;

    /* This should remove "early" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT - 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "middle" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "late" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 11);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    /* Add them back in again */
    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove all of them */
    SSL_CTX_flush_sessions(ctx, 0);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    (void)SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_UPDATE_TIME
                                         | SSL_CTX_get_session_cache_mode(ctx));

    /* make sure |now| is NOT  equal to the current time */
    now -= 10;
    if (!TEST_int_ne(SSL_SESSION_set_time(early, now), 0)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_long_ne(SSL_SESSION_get_time(early), now))
        goto end;

    testresult = 1;
 end:
    SSL_CTX_free(ctx);
    SSL_SESSION_free(early);
    SSL_SESSION_free(middle);
    SSL_SESSION_free(late);
    return testresult;
}

/*
 * Test 0: Client sets servername and server acknowledges it (TLSv1.2)
 * Test 1: Client sets servername and server does not acknowledge it (TLSv1.2)
 * Test 2: Client sets inconsistent servername on resumption (TLSv1.2)
 * Test 3: Client does not set servername on initial handshake (TLSv1.2)
 * Test 4: Client does not set servername on resumption handshake (TLSv1.2)
 * Test 5: Client sets servername and server acknowledges it (TLSv1.3)
 * Test 6: Client sets servername and server does not acknowledge it (TLSv1.3)
 * Test 7: Client sets inconsistent servername on resumption (TLSv1.3)
 * Test 8: Client does not set servername on initial handshake(TLSv1.3)
 * Test 9: Client does not set servername on resumption handshake (TLSv1.3)
 */
static int test_servername(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;
    const char *sexpectedhost = NULL, *cexpectedhost = NULL;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 4)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 5)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 4) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst != 1 && tst != 6) {
        if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx,
                                                              hostname_cb)))
            goto end;
    }

    if (tst != 3 && tst != 8) {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        sexpectedhost = cexpectedhost = "goodhost";
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(SSL_get_servername(clientssl, TLSEXT_NAMETYPE_host_name),
                     cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    /* Now repeat with a resumption handshake */

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0)
            || !TEST_ptr_ne(sess = SSL_get1_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))
            || !TEST_int_eq(SSL_shutdown(serverssl), 0))
        goto end;

    SSL_free(clientssl);
    SSL_free(serverssl);
    clientssl = serverssl = NULL;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL)))
        goto end;

    if (!TEST_true(SSL_set_session(clientssl, sess)))
        goto end;

    sexpectedhost = cexpectedhost = "goodhost";
    if (tst == 2 || tst == 7) {
        /* Set an inconsistent hostname */
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "altgoodhost")))
            goto end;
        /*
         * In TLSv1.2 we expect the hostname from the original handshake, in
         * TLSv1.3 we expect the hostname from this handshake
         */
        if (tst == 7)
            sexpectedhost = cexpectedhost = "altgoodhost";

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "altgoodhost"))
            goto end;
    } else if (tst == 4 || tst == 9) {
        /*
         * A TLSv1.3 session does not associate a session with a servername,
         * but a TLSv1.2 session does.
         */
        if (tst == 9)
            sexpectedhost = cexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         cexpectedhost))
            goto end;
    } else {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        /*
         * In a TLSv1.2 resumption where the hostname was not acknowledged
         * we expect the hostname on the server to be empty. On the client we
         * return what was requested in this case.
         *
         * Similarly if the client didn't set a hostname on an original TLSv1.2
         * session but is now, the server hostname will be empty, but the client
         * is as we set it.
         */
        if (tst == 1 || tst == 3)
            sexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "goodhost"))
            goto end;
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_session_reused(clientssl))
            || !TEST_true(SSL_session_reused(serverssl))
            || !TEST_str_eq(SSL_get_servername(clientssl,
                                               TLSEXT_NAMETYPE_host_name),
                            cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
/*
 * Test that if signature algorithms are not available, then we do not offer or
 * accept them.
 * Test 0: Two RSA sig algs available: both RSA sig algs shared
 * Test 1: The client only has SHA2-256: only SHA2-256 algorithms shared
 * Test 2: The server only has SHA2-256: only SHA2-256 algorithms shared
 * Test 3: An RSA and an ECDSA sig alg available: both sig algs shared
 * Test 4: The client only has an ECDSA sig alg: only ECDSA algorithms shared
 * Test 5: The server only has an ECDSA sig alg: only ECDSA algorithms shared
 */
static int test_sigalgs_available(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_LIB_CTX *clientctx = libctx, *serverctx = libctx;
    OSSL_PROVIDER *filterprov = NULL;
    int sig, hash;

    if (!TEST_ptr(tmpctx))
        goto end;

    if (idx != 0 && idx != 3) {
        if (!TEST_true(OSSL_PROVIDER_add_builtin(tmpctx, "filter",
                                                 filter_provider_init)))
            goto end;

        filterprov = OSSL_PROVIDER_load(tmpctx, "filter");
        if (!TEST_ptr(filterprov))
            goto end;

        if (idx < 3) {
            /*
             * Only enable SHA2-256 so rsa_pss_rsae_sha384 should not be offered
             * or accepted for the peer that uses this libctx. Note that libssl
             * *requires* SHA2-256 to be available so we cannot disable that. We
             * also need SHA1 for our certificate.
             */
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_DIGEST,
                                                      "SHA2-256:SHA1")))
                goto end;
        } else {
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_SIGNATURE,
                                                      "ECDSA"))
                    || !TEST_true(filter_provider_set_filter(OSSL_OP_KEYMGMT,
                                                             "EC:X25519:X448")))
                goto end;
        }

        if (idx == 1 || idx == 4)
            clientctx = tmpctx;
        else
            serverctx = tmpctx;
    }

    cctx = SSL_CTX_new_ex(clientctx, NULL, TLS_client_method());
    sctx = SSL_CTX_new_ex(serverctx, NULL, TLS_server_method());
    if (!TEST_ptr(cctx) || !TEST_ptr(sctx))
        goto end;

    if (idx != 5) {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;
    } else {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert2, privkey2)))
            goto end;
    }

    /* Ensure we only use TLSv1.2 ciphersuites based on SHA256 */
    if (idx < 4) {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-RSA-AES128-GCM-SHA256")))
            goto end;
    } else {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-ECDSA-AES128-GCM-SHA256")))
            goto end;
    }

    if (idx < 3) {
        if (!SSL_CTX_set1_sigalgs_list(cctx,
                                       "rsa_pss_rsae_sha384"
                                       ":rsa_pss_rsae_sha256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha384"
                                              ":rsa_pss_rsae_sha256"))
            goto end;
    } else {
        if (!SSL_CTX_set1_sigalgs_list(cctx, "rsa_pss_rsae_sha256:ECDSA+SHA256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha256:ECDSA+SHA256"))
            goto end;
    }

    if (idx != 5
        && (!TEST_int_eq(SSL_CTX_use_certificate_file(sctx, cert2,
                                                      SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_use_PrivateKey_file(sctx,
                                                        privkey2,
                                                        SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_check_private_key(sctx), 1)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    /* For tests 0 and 3 we expect 2 shared sigalgs, otherwise exactly 1 */
    if (!TEST_int_eq(SSL_get_shared_sigalgs(serverssl, 0, &sig, &hash, NULL,
                                            NULL, NULL),
                     (idx == 0 || idx == 3) ? 2 : 1))
        goto end;

    if (!TEST_int_eq(hash, idx == 0 ? NID_sha384 : NID_sha256))
        goto end;

    if (!TEST_int_eq(sig, (idx == 4 || idx == 5) ? EVP_PKEY_EC
                                                 : NID_rsassaPss))
        goto end;

    testresult = filter_provider_check_clean_finish();

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(filterprov);
    OSSL_LIB_CTX_free(tmpctx);

    return testresult;
}
#endif /*
        * !defined(OPENSSL_NO_EC) \
        * && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
        */

#ifndef OPENSSL_NO_TLS1_3
/* This test can run in TLSv1.3 even if ec and dh are disabled */
static int test_pluggable_group(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_PROVIDER *tlsprov = OSSL_PROVIDER_load(libctx, "tls-provider");
    /* Check that we are not impacted by a provider without any groups */
    OSSL_PROVIDER *legacyprov = OSSL_PROVIDER_load(libctx, "legacy");
    const char *group_name = idx == 0 ? "xorgroup" : "xorkemgroup";

    if (!TEST_ptr(tlsprov))
        goto end;

    if (legacyprov == NULL) {
        /*
         * In this case we assume we've been built with "no-legacy" and skip
         * this test (there is no OPENSSL_NO_LEGACY)
         */
        testresult = 1;
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION,
                                       TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set1_groups_list(serverssl, group_name))
            || !TEST_true(SSL_set1_groups_list(clientssl, group_name)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(group_name,
                     SSL_group_to_name(serverssl, SSL_get_shared_group(serverssl, 0))))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(tlsprov);
    OSSL_PROVIDER_unload(legacyprov);

    return testresult;
}
#endif

#ifndef OPENSSL_NO_TLS1_2
static int test_ssl_dup(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL, *client2ssl = NULL;
    int testresult = 0;
    BIO *rbio = NULL, *wbio = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_min_proto_version(clientssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(clientssl, TLS1_2_VERSION)))
        goto end;

    client2ssl = SSL_dup(clientssl);
    rbio = SSL_get_rbio(clientssl);
    if (!TEST_ptr(rbio)
            || !TEST_true(BIO_up_ref(rbio)))
        goto end;
    SSL_set0_rbio(client2ssl, rbio);
    rbio = NULL;

    wbio = SSL_get_wbio(clientssl);
    if (!TEST_ptr(wbio) || !TEST_true(BIO_up_ref(wbio)))
        goto end;
    SSL_set0_wbio(client2ssl, wbio);
    rbio = NULL;

    if (!TEST_ptr(client2ssl)
               /* Handshake not started so pointers should be different */
            || !TEST_ptr_ne(clientssl, client2ssl))
        goto end;

    if (!TEST_int_eq(SSL_get_min_proto_version(client2ssl), TLS1_2_VERSION)
            || !TEST_int_eq(SSL_get_max_proto_version(client2ssl), TLS1_2_VERSION))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, client2ssl, SSL_ERROR_NONE)))
        goto end;

    SSL_free(clientssl);
    clientssl = SSL_dup(client2ssl);
    if (!TEST_ptr(clientssl)
               /* Handshake has finished so pointers should be the same */
            || !TEST_ptr_eq(clientssl, client2ssl))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_free(client2ssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

# ifndef OPENSSL_NO_DH

static EVP_PKEY *tmp_dh_params = NULL;

/* Helper function for the test_set_tmp_dh() tests */
static EVP_PKEY *get_tmp_dh_params(void)
{
    if (tmp_dh_params == NULL) {
        BIGNUM *p = NULL;
        OSSL_PARAM_BLD *tmpl = NULL;
        EVP_PKEY_CTX *pctx = NULL;
        OSSL_PARAM *params = NULL;
        EVP_PKEY *dhpkey = NULL;

        p = BN_get_rfc3526_prime_2048(NULL);
        if (!TEST_ptr(p))
            goto end;

        pctx = EVP_PKEY_CTX_new_from_name(libctx, "DH", NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_fromdata_init(pctx), 1))
            goto end;

        tmpl = OSSL_PARAM_BLD_new();
        if (!TEST_ptr(tmpl)
                || !TEST_true(OSSL_PARAM_BLD_push_BN(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_P,
                                                        p))
                || !TEST_true(OSSL_PARAM_BLD_push_uint(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_G,
                                                        2)))
            goto end;

        params = OSSL_PARAM_BLD_to_param(tmpl);
        if (!TEST_ptr(params)
                || !TEST_int_eq(EVP_PKEY_fromdata(pctx, &dhpkey,
                                                  EVP_PKEY_KEY_PARAMETERS,
                                                  params), 1))
            goto end;

        tmp_dh_params = dhpkey;
    end:
        BN_free(p);
        EVP_PKEY_CTX_free(pctx);
        OSSL_PARAM_BLD_free(tmpl);
        OSSL_PARAM_free(params);
    }

    if (tmp_dh_params != NULL && !EVP_PKEY_up_ref(tmp_dh_params))
        return NULL;

    return tmp_dh_params;
}

#  ifndef OPENSSL_NO_DEPRECATED_3_0
/* Callback used by test_set_tmp_dh() */
static DH *tmp_dh_callback(SSL *s, int is_export, int keylen)
{
    EVP_PKEY *dhpkey = get_tmp_dh_params();
    DH *ret = NULL;

    if (!TEST_ptr(dhpkey))
        return NULL;

    /*
     * libssl does not free the returned DH, so we free it now knowing that even
     * after we free dhpkey, there will still be a reference to the owning
     * EVP_PKEY in tmp_dh_params, and so the DH object will live for the length
     * of time we need it for.
     */
    ret = EVP_PKEY_get1_DH(dhpkey);
    DH_free(ret);

    EVP_PKEY_free(dhpkey);

    return ret;
}
#  endif

/*
 * Test the various methods for setting temporary DH parameters
 *
 * Test  0: Default (no auto) setting
 * Test  1: Explicit SSL_CTX auto off
 * Test  2: Explicit SSL auto off
 * Test  3: Explicit SSL_CTX auto on
 * Test  4: Explicit SSL auto on
 * Test  5: Explicit SSL_CTX auto off, custom DH params via EVP_PKEY
 * Test  6: Explicit SSL auto off, custom DH params via EVP_PKEY
 *
 * The following are testing deprecated APIs, so we only run them if available
 * Test  7: Explicit SSL_CTX auto off, custom DH params via DH
 * Test  8: Explicit SSL auto off, custom DH params via DH
 * Test  9: Explicit SSL_CTX auto off, custom DH params via callback
 * Test 10: Explicit SSL auto off, custom DH params via callback
 */
static int test_set_tmp_dh(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int dhauto = (idx == 3 || idx == 4) ? 1 : 0;
    int expected = (idx <= 2) ? 0 : 1;
    EVP_PKEY *dhpkey = NULL;
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH *dh = NULL;
#  else

    if (idx >= 7)
        return 1;
#  endif

    if (idx >= 5 && idx <= 8) {
        dhpkey = get_tmp_dh_params();
        if (!TEST_ptr(dhpkey))
            goto end;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    if (idx == 7 || idx == 8) {
        dh = EVP_PKEY_get1_DH(dhpkey);
        if (!TEST_ptr(dh))
            goto end;
    }
#  endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if ((idx & 1) == 1) {
        if (!TEST_true(SSL_CTX_set_dh_auto(sctx, dhauto)))
            goto end;
    }

    if (idx == 5) {
        if (!TEST_true(SSL_CTX_set0_tmp_dh_pkey(sctx, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 7) {
        if (!TEST_true(SSL_CTX_set_tmp_dh(sctx, dh)))
            goto end;
    } else if (idx == 9) {
        SSL_CTX_set_tmp_dh_callback(sctx, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if ((idx & 1) == 0 && idx != 0) {
        if (!TEST_true(SSL_set_dh_auto(serverssl, dhauto)))
            goto end;
    }
    if (idx == 6) {
        if (!TEST_true(SSL_set0_tmp_dh_pkey(serverssl, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 8) {
        if (!TEST_true(SSL_set_tmp_dh(serverssl, dh)))
            goto end;
    } else if (idx == 10) {
        SSL_set_tmp_dh_callback(serverssl, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, "DHE-RSA-AES128-SHA")))
        goto end;

    /*
     * If autoon then we should succeed. Otherwise we expect failure because
     * there are no parameters
     */
    if (!TEST_int_eq(create_ssl_connection(serverssl, clientssl,
                                           SSL_ERROR_NONE), expected))
        goto end;

    testresult = 1;

 end:
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH_free(dh);
#  endif
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(dhpkey);

    return testresult;
}

/*
 * Test the auto DH keys are appropriately sized
 */
static int test_dh_auto(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    EVP_PKEY *tmpkey = NULL;
    char *thiscert = NULL, *thiskey = NULL;
    size_t expdhsize = 0;
    const char *ciphersuite = "DHE-RSA-AES128-SHA";

    switch (idx) {
    case 0:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        thiscert = cert1024;
        thiskey = privkey1024;
        expdhsize = 1024;
        break;
    case 1:
        /* 2048 bit prime */
        thiscert = cert;
        thiskey = privkey;
        expdhsize = 2048;
        break;
    case 2:
        thiscert = cert3072;
        thiskey = privkey3072;
        expdhsize = 3072;
        break;
    case 3:
        thiscert = cert4096;
        thiskey = privkey4096;
        expdhsize = 4096;
        break;
    case 4:
        thiscert = cert8192;
        thiskey = privkey8192;
        expdhsize = 8192;
        break;
    /* No certificate cases */
    case 5:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        ciphersuite = "ADH-AES128-SHA256:@SECLEVEL=0";
        expdhsize = 1024;
        break;
    case 6:
        ciphersuite = "ADH-AES256-SHA256:@SECLEVEL=0";
        expdhsize = 3072;
        break;
    default:
        TEST_error("Invalid text index");
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, thiscert, thiskey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_dh_auto(serverssl, 1))
            || !TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, ciphersuite))
            || !TEST_true(SSL_set_cipher_list(clientssl, ciphersuite)))
        goto end;

    /*
     * Send the server's first flight. At this point the server has created the
     * temporary DH key but hasn't finished using it yet. Once used it is
     * removed, so we cannot test it.
     */
    if (!TEST_int_le(SSL_connect(clientssl), 0)
            || !TEST_int_le(SSL_accept(serverssl), 0))
        goto end;

    if (!TEST_int_gt(SSL_get_tmp_key(serverssl, &tmpkey), 0))
        goto end;
    if (!TEST_size_t_eq(EVP_PKEY_get_bits(tmpkey), expdhsize))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(tmpkey);

    return testresult;

}
# endif /* OPENSSL_NO_DH */
#endif /* OPENSSL_NO_TLS1_2 */

#ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Test that setting an SNI callback works with TLSv1.3. Specifically we check
 * that it works even without a certificate configured for the original
 * SSL_CTX
 */
static int test_sni_tls13(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *sctx2 = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /* Reset callback counter */
    snicb = 0;

    /* Create an initial SSL_CTX with no certificate configured */
    sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(sctx))
        goto end;
    /* Require TLSv1.3 as a minimum */
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_3_VERSION, 0,
                                       &sctx2, &cctx, cert, privkey)))
        goto end;

    /* Set up SNI */
    if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx, sni_cb))
            || !TEST_true(SSL_CTX_set_tlsext_servername_arg(sctx, sctx2)))
        goto end;

    /*
     * Connection should still succeed because the final SSL_CTX has the right
     * certificates configured.
     */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /* We should have had the SNI callback called exactly once */
    if (!TEST_int_eq(snicb, 1))
        goto end;

    testresult = 1;

end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}

/*
 * Test that the lifetime hint of a TLSv1.3 ticket is no more than 1 week
 * 0 = TLSv1.2
 * 1 = TLSv1.3
 */
static int test_ticket_lifetime(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int version = TLS1_3_VERSION;

#define ONE_WEEK_SEC (7 * 24 * 60 * 60)
#define TWO_WEEK_SEC (2 * ONE_WEEK_SEC)

    if (idx == 0) {
#ifdef OPENSSL_NO_TLS1_2
        return TEST_skip("TLS 1.2 is disabled.");
#else
        version = TLS1_2_VERSION;
#endif
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), version, version,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL)))
        goto end;

    /*
     * Set the timeout to be more than 1 week
     * make sure the returned value is the default
     */
    if (!TEST_long_eq(SSL_CTX_set_timeout(sctx, TWO_WEEK_SEC),
                      SSL_get_default_timeout(serverssl)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (idx == 0) {
        /* TLSv1.2 uses the set value */
        if (!TEST_ulong_eq(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), TWO_WEEK_SEC))
            goto end;
    } else {
        /* TLSv1.3 uses the limited value */
        if (!TEST_ulong_le(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), ONE_WEEK_SEC))
            goto end;
    }
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
 * Test that setting an ALPN does not violate RFC
 */
static int test_set_alpn(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int testresult = 0;

    unsigned char bad0[] = { 0x00, 'b', 'a', 'd' };
    unsigned char good[] = { 0x04, 'g', 'o', 'o', 'd' };
    unsigned char bad1[] = { 0x01, 'b', 'a', 'd' };
    unsigned char bad2[] = { 0x03, 'b', 'a', 'd', 0x00};
    unsigned char bad3[] = { 0x03, 'b', 'a', 'd', 0x01, 'b', 'a', 'd'};
    unsigned char bad4[] = { 0x03, 'b', 'a', 'd', 0x06, 'b', 'a', 'd'};

    /* Create an initial SSL_CTX with no certificate configured */
    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    /* the set_alpn functions return 0 (false) on success, non-zero (true) on failure */
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, 0)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, good, 1)))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad4, sizeof(bad4))))
        goto end;

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    if (!TEST_false(SSL_set_alpn_protos(ssl, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, 0)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, good, 1)))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad4, sizeof(bad4))))
        goto end;

    testresult = 1;

end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return testresult;
}

/*
 * Test SSL_CTX_set1_verify/chain_cert_store and SSL_CTX_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl_ctx(void)
{
   SSL_CTX *ctx = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, new_store)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, NULL)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_CTX_free(ctx);
   return testresult;
}

/*
 * Test SSL_set1_verify/chain_cert_store and SSL_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl(void)
{
   SSL_CTX *ctx = NULL;
   SSL *ssl = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Create an SSL object. */
   ssl = SSL_new(ctx);
   if (!TEST_ptr(ssl))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, new_store)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, NULL)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_free(ssl);
   SSL_CTX_free(ctx);
   return testresult;
}


static int test_inherit_verify_param(void)
{
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    X509_VERIFY_PARAM *cp = NULL;
    SSL *ssl = NULL;
    X509_VERIFY_PARAM *sp = NULL;
    int hostflags = X509_CHECK_FLAG_NEVER_CHECK_SUBJECT;

    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    cp = SSL_CTX_get0_param(ctx);
    if (!TEST_ptr(cp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(cp), 0))
        goto end;

    X509_VERIFY_PARAM_set_hostflags(cp, hostflags);

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    sp = SSL_get0_param(ssl);
    if (!TEST_ptr(sp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(sp), hostflags))
        goto end;

    testresult = 1;

 end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return testresult;
}


static int test_load_dhfile(void)
{
#ifndef OPENSSL_NO_DH
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    SSL_CONF_CTX *cctx = NULL;

    if (dhfile == NULL)
        return 1;

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_client_method()))
        || !TEST_ptr(cctx = SSL_CONF_CTX_new()))
        goto end;

    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
    SSL_CONF_CTX_set_flags(cctx,
                           SSL_CONF_FLAG_CERTIFICATE
                           | SSL_CONF_FLAG_SERVER
                           | SSL_CONF_FLAG_FILE);

    if (!TEST_int_eq(SSL_CONF_cmd(cctx, "DHParameters", dhfile), 2))
        goto end;

    testresult = 1;
end:
    SSL_CONF_CTX_free(cctx);
    SSL_CTX_free(ctx);

    return testresult;
#else
    return TEST_skip("DH not supported by this build");
#endif
}

#ifndef OPENSSL_NO_QUIC
static int test_quic_set_encryption_secrets(SSL *ssl,
                                            OSSL_ENCRYPTION_LEVEL level,
                                            const uint8_t *read_secret,
                                            const uint8_t *write_secret,
                                            size_t secret_len)
{
    test_printf_stderr("quic_set_encryption_secrets() %s, lvl=%d, len=%zd\n",
                       ssl->server ? "server" : "client", level, secret_len);
    return 1;
}

static int test_quic_add_handshake_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                                        const uint8_t *data, size_t len)
{
    SSL *peer = (SSL*)SSL_get_app_data(ssl);

    TEST_info("quic_add_handshake_data() %s, lvl=%d, *data=0x%02X, len=%zd\n",
              ssl->server ? "server" : "client", level, (int)*data, len);
    if (!TEST_ptr(peer))
        return 0;

    /* We're called with what is locally written; this gives it to the peer */
    if (!TEST_true(SSL_provide_quic_data(peer, level, data, len))) {
        ERR_print_errors_fp(stderr);
        return 0;
    }

    return 1;
}

static int test_quic_flush_flight(SSL *ssl)
{
    test_printf_stderr("quic_flush_flight() %s\n", ssl->server ? "server" : "client");
    return 1;
}

static int test_quic_send_alert(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert)
{
    test_printf_stderr("quic_send_alert() %s, lvl=%d, alert=%d\n",
                       ssl->server ? "server" : "client", level, alert);
    return 1;
}

static SSL_QUIC_METHOD quic_method = {
    test_quic_set_encryption_secrets,
    test_quic_add_handshake_data,
    test_quic_flush_flight,
    test_quic_send_alert,
};

static int test_quic_api_set_versions(SSL *ssl, int ver)
{
    SSL_set_quic_transport_version(ssl, ver);
    return 1;
}

static int test_quic_api_version(int clnt, int srvr)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";
    const uint8_t *peer_str;
    size_t peer_str_len;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);

    if (!TEST_true(create_ssl_ctx_pair(libctx,
                                       TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION, 0,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_true(SSL_CTX_set_quic_method(cctx, &quic_method))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                             &clientssl, NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(serverssl, clientssl))
            || !TEST_true(SSL_set_app_data(clientssl, serverssl))
            || !TEST_true(test_quic_api_set_versions(clientssl, clnt))
            || !TEST_true(test_quic_api_set_versions(serverssl, srvr))
            || !TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                     SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_version(serverssl) == TLS1_3_VERSION)
            || !TEST_true(SSL_version(clientssl) == TLS1_3_VERSION)
            || !(TEST_int_eq(SSL_quic_read_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_read_level(serverssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(serverssl), ssl_encryption_application)))
        goto end;

    SSL_get_peer_quic_transport_params(serverssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, client_str, strlen(client_str)+1))
        goto end;
    SSL_get_peer_quic_transport_params(clientssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, server_str, strlen(server_str)+1))
        goto end;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(clientssl)))
        goto end;

    /* Dummy handshake call should succeed */
    if (!TEST_true(SSL_do_handshake(clientssl)))
        goto end;
    /* Test that we (correctly) fail to send KeyUpdate */
    if (!TEST_true(SSL_key_update(clientssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(clientssl), 0))
        goto end;
    if (!TEST_true(SSL_key_update(serverssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(serverssl), 0))
        goto end;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (srvr == 0 && clnt == 0)
        srvr = clnt = TLSEXT_TYPE_quic_transport_parameters;
    else if (srvr == 0)
        srvr = clnt;
    else if (clnt == 0)
        clnt = srvr;
    TEST_info("expected clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(serverssl), clnt))
        goto end;
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(clientssl), srvr))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_quic_api(int tst)
{
    SSL_CTX *sctx = NULL;
    SSL *serverssl = NULL;
    int testresult = 0;
    static int clnt_params[] = { 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int srvr_params[] = { 0,
                                 0,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int results[] = { 1, 1, 1, 1, 1, 0, 1, 0, 1 };

    /* Failure cases:
     * test 6/[5] clnt = parameters, srvr = draft
     * test 8/[7] clnt = draft, srvr = parameters
     */

    /* Clean up logging space */
    memset(client_log_buffer, 0, sizeof(client_log_buffer));
    memset(server_log_buffer, 0, sizeof(server_log_buffer));
    client_log_buffer_index = 0;
    server_log_buffer_index = 0;
    error_writing_log = 0;

    if (!TEST_ptr(sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method()))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_ptr(sctx->quic_method)
            || !TEST_ptr(serverssl = SSL_new(sctx))
            || !TEST_true(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, NULL))
            || !TEST_false(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, &quic_method))
            || !TEST_true(SSL_IS_QUIC(serverssl)))
        goto end;

    if (!TEST_int_eq(test_quic_api_version(clnt_params[tst], srvr_params[tst]), results[tst]))
        goto end;

    testresult = 1;

end:
    SSL_CTX_free(sctx);
    sctx = NULL;
    SSL_free(serverssl);
    serverssl = NULL;
    return testresult;
}

# ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Helper method to setup objects for QUIC early data test. Caller
 * frees objects on error.
 */
static int quic_setupearly_data_test(SSL_CTX **cctx, SSL_CTX **sctx,
                                     SSL **clientssl, SSL **serverssl,
                                     SSL_SESSION **sess, int idx)
{
    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";

    if (*sctx == NULL
            && (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                               TLS_client_method(),
                                               TLS1_3_VERSION, 0,
                                               sctx, cctx, cert, privkey))
                || !TEST_true(SSL_CTX_set_quic_method(*sctx, &quic_method))
                || !TEST_true(SSL_CTX_set_quic_method(*cctx, &quic_method))
                || !TEST_true(SSL_CTX_set_max_early_data(*sctx, 0xffffffffu))))
        return 0;

    if (idx == 1) {
        /* When idx == 1 we repeat the tests with read_ahead set */
        SSL_CTX_set_read_ahead(*cctx, 1);
        SSL_CTX_set_read_ahead(*sctx, 1);
    } else if (idx == 2) {
        /* When idx == 2 we are doing early_data with a PSK. Set up callbacks */
        SSL_CTX_set_psk_use_session_callback(*cctx, use_session_cb);
        SSL_CTX_set_psk_find_session_callback(*sctx, find_session_cb);
        use_session_cb_cnt = 0;
        find_session_cb_cnt = 0;
        srvid = pskid;
    }

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl, clientssl,
                                      NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    /*
     * For one of the run throughs (doesn't matter which one), we'll try sending
     * some SNI data in the initial ClientHello. This will be ignored (because
     * there is no SNI cb set up by the server), so it should not impact
     * early_data.
     */
    if (idx == 1
            && !TEST_true(SSL_set_tlsext_host_name(*clientssl, "localhost")))
        return 0;

    if (idx == 2) {
        clientpsk = create_a_psk(*clientssl);
        if (!TEST_ptr(clientpsk)
                || !TEST_true(SSL_SESSION_set_max_early_data(clientpsk,
                                                             0xffffffffu))
                || !TEST_true(SSL_SESSION_up_ref(clientpsk))) {
            SSL_SESSION_free(clientpsk);
            clientpsk = NULL;
            return 0;
        }
        serverpsk = clientpsk;

        if (sess != NULL) {
            if (!TEST_true(SSL_SESSION_up_ref(clientpsk))) {
                SSL_SESSION_free(clientpsk);
                SSL_SESSION_free(serverpsk);
                clientpsk = serverpsk = NULL;
                return 0;
            }
            *sess = clientpsk;
        }

        SSL_set_quic_early_data_enabled(*serverssl, 1);
        SSL_set_quic_early_data_enabled(*clientssl, 1);

        return 1;
    }

    if (sess == NULL)
        return 1;

    if (!TEST_true(create_bare_ssl_connection(*serverssl, *clientssl,
                                              SSL_ERROR_NONE, 0)))
        return 0;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(*clientssl)))
        return 0;

    *sess = SSL_get1_session(*clientssl);
    SSL_shutdown(*clientssl);
    SSL_shutdown(*serverssl);
    SSL_free(*serverssl);
    SSL_free(*clientssl);
    *serverssl = *clientssl = NULL;

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl,
                                      clientssl, NULL, NULL))
            || !TEST_true(SSL_set_session(*clientssl, *sess))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    SSL_set_quic_early_data_enabled(*serverssl, 1);
    SSL_set_quic_early_data_enabled(*clientssl, 1);

    return 1;
}

static int test_quic_early_data(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;

    if (!TEST_true(quic_setupearly_data_test(&cctx, &sctx, &clientssl,
                                             &serverssl, &sess, tst)))
        goto end;

    if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_get_early_data_status(serverssl)))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_SESSION_free(clientpsk);
    SSL_SESSION_free(serverpsk);
    clientpsk = serverpsk = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}
# endif /* OSSL_NO_USABLE_TLS1_3 */
#endif /* OPENSSL_NO_QUIC */

OPT_TEST_DECLARE_USAGE("certfile privkeyfile srpvfile tmpfile provider config dhfile\n")

int setup_tests(void)
{
    char *modulename;
    char *configfile;

    libctx = OSSL_LIB_CTX_new();
    if (!TEST_ptr(libctx))
        return 0;

    defctxnull = OSSL_PROVIDER_load(NULL, "null");

    /*
     * Verify that the default and fips providers in the default libctx are not
     * available
     */
    if (!TEST_false(OSSL_PROVIDER_available(NULL, "default"))
            || !TEST_false(OSSL_PROVIDER_available(NULL, "fips")))
        return 0;

    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(certsdir = test_get_argument(0))
            || !TEST_ptr(srpvfile = test_get_argument(1))
            || !TEST_ptr(tmpfilename = test_get_argument(2))
            || !TEST_ptr(modulename = test_get_argument(3))
            || !TEST_ptr(configfile = test_get_argument(4))
            || !TEST_ptr(dhfile = test_get_argument(5)))
        return 0;

    if (!TEST_true(OSSL_LIB_CTX_load_config(libctx, configfile)))
        return 0;

    /* Check we have the expected provider available */
    if (!TEST_true(OSSL_PROVIDER_available(libctx, modulename)))
        return 0;

    /* Check the default provider is not available */
    if (strcmp(modulename, "default") != 0
            && !TEST_false(OSSL_PROVIDER_available(libctx, "default")))
        return 0;

    if (strcmp(modulename, "fips") == 0)
        is_fips = 1;

    /*
     * We add, but don't load the test "tls-provider". We'll load it when we
     * need it.
     */
    if (!TEST_true(OSSL_PROVIDER_add_builtin(libctx, "tls-provider",
                                             tls_provider_init)))
        return 0;


    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    SSL_SESSION *clntsess = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst % 2 == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst % 2 == 1)
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
    switch (tst) {
    case 0:
    case 1:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE;
        break;

    case 2:
    case 3:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE_RENEW;
        break;

    case 4:
    case 5:
        tick_dec_ret = SSL_TICKET_RETURN_USE;
        break;

    case 6:
    case 7:
        tick_dec_ret = SSL_TICKET_RETURN_USE_RENEW;
        break;

    default:
        tick_dec_ret = SSL_TICKET_RETURN_ABORT;
    }
    switch (tst) {
    case 0:
    case 1:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE;
        break;

    case 2:
    case 3:
        tick_dec_ret = SSL_TICKET_RETURN_IGNORE_RENEW;
        break;

    case 4:
    case 5:
        tick_dec_ret = SSL_TICKET_RETURN_USE;
        break;

    case 6:
    case 7:
        tick_dec_ret = SSL_TICKET_RETURN_USE_RENEW;
        break;

    default:
        tick_dec_ret = SSL_TICKET_RETURN_ABORT;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       ((tst % 2) == 0) ? TLS1_2_VERSION
                                                        : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * We only want sessions to resume from tickets - not the session cache. So
     * switch the cache off.
     */
    if (!TEST_true(SSL_CTX_set_session_cache_mode(sctx, SSL_SESS_CACHE_OFF)))
        goto end;

    if (!TEST_true(SSL_CTX_set_session_ticket_cb(sctx, gen_tick_cb, dec_tick_cb,
                                                 NULL)))
        goto end;

    if (tst >= 14) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_evp_cb(sctx, tick_key_evp_cb)))
            goto end;
#ifndef OPENSSL_NO_DEPRECATED_3_0
    } else if (tst >= 8) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_cb(sctx, tick_key_cb)))
            goto end;
#endif
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * The decrypt ticket key callback in TLSv1.2 should be called even though
     * we have no ticket yet, because it gets called with a status of
     * SSL_TICKET_EMPTY (the client indicates support for tickets but does not
     * actually send any ticket data). This does not happen in TLSv1.3 because
     * it is not valid to send empty ticket data in TLSv1.3.
     */
    if (!TEST_int_eq(gen_tick_called, 1)
            || !TEST_int_eq(dec_tick_called, ((tst % 2) == 0) ? 1 : 0))
        goto end;

    gen_tick_called = dec_tick_called = 0;

    clntsess = SSL_get1_session(clientssl);
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
    serverssl = clientssl = NULL;

    /* Now do a resumption */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL))
            || !TEST_true(SSL_set_session(clientssl, clntsess))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
            || tick_key_renew == -1) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
            goto end;
    }

    if (!TEST_int_eq(gen_tick_called,
                     (tick_key_renew
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(clntsess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test incorrect shutdown.
 * Test 0: client does not shutdown properly,
 *         server does not set SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_SSL
 * Test 1: client does not shutdown properly,
 *         server sets SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_ZERO_RETURN
 */
static int test_incorrect_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[80];
    BIO *c2s;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), 0, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 1)
        SSL_CTX_set_options(sctx, SSL_OP_IGNORE_UNEXPECTED_EOF);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                            NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE)))
        goto end;

    c2s = SSL_get_rbio(serverssl);
    BIO_set_mem_eof_return(c2s, 0);

    if (!TEST_false(SSL_read(serverssl, buf, sizeof(buf))))
        goto end;

    if (tst == 0 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_SSL) )
        goto end;
    if (tst == 1 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_ZERO_RETURN) )
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test bi-directional shutdown.
 * Test 0: TLSv1.2
 * Test 1: TLSv1.2, server continues to read/write after client shutdown
 * Test 2: TLSv1.3, no pending NewSessionTicket messages
 * Test 3: TLSv1.3, pending NewSessionTicket messages
 * Test 4: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends key update, client reads it
 * Test 5: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends CertificateRequest, client reads and ignores it
 * Test 6: TLSv1.3, server continues to read/write after client shutdown, client
 *                  doesn't read it
 */
static int test_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char msg[] = "A test message";
    char buf[80];
    size_t written, readbytes;
    SSL_SESSION *sess;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 1)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 2)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 1) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 5)
        SSL_CTX_set_post_handshake_auth(cctx, 1);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst == 3) {
        if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                  SSL_ERROR_NONE, 1))
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_false(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE))
            || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))) {
        goto end;
    }

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0))
        goto end;

    if (tst >= 4) {
        /*
         * Reading on the server after the client has sent close_notify should
         * fail and provide SSL_ERROR_ZERO_RETURN
         */
        if (!TEST_false(SSL_read_ex(serverssl, buf, sizeof(buf), &readbytes))
                || !TEST_int_eq(SSL_get_error(serverssl, 0),
                                SSL_ERROR_ZERO_RETURN)
                || !TEST_int_eq(SSL_get_shutdown(serverssl),
                                SSL_RECEIVED_SHUTDOWN)
                   /*
                    * Even though we're shutdown on receive we should still be
                    * able to write.
                    */
                || !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (tst == 4
                && !TEST_true(SSL_key_update(serverssl,
                                             SSL_KEY_UPDATE_REQUESTED)))
            goto end;
        if (tst == 5) {
            SSL_set_verify(serverssl, SSL_VERIFY_PEER, NULL);
            if (!TEST_true(SSL_verify_client_post_handshake(serverssl)))
                goto end;
        }
        if ((tst == 4 || tst == 5)
                && !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (!TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
        if (tst == 4 || tst == 5) {
            /* Should still be able to read data from server */
            if (!TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                       &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0)
                    || !TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                              &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0))
                goto end;
        }
    }

    /* Writing on the client after sending close_notify shouldn't be possible */
    if (!TEST_false(SSL_write_ex(clientssl, msg, sizeof(msg), &written)))
        goto end;

    if (tst < 4) {
        /*
         * For these tests the client has sent close_notify but it has not yet
         * been received by the server. The server has not sent close_notify
         * yet.
         */
        if (!TEST_int_eq(SSL_shutdown(serverssl), 0)
                   /*
                    * Writing on the server after sending close_notify shouldn't
                    * be possible.
                    */
                || !TEST_false(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
                || !TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess))
                || !TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
    } else if (tst == 4 || tst == 5) {
        /*
         * In this test the client has sent close_notify and it has been
         * received by the server which has responded with a close_notify. The
         * client needs to read the close_notify sent by the server.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else {
        /*
         * tst == 6
         *
         * The client has sent close_notify and is expecting a close_notify
         * back, but instead there is application data first. The shutdown
         * should fail with a fatal error.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), -1)
                || !TEST_int_eq(SSL_get_error(clientssl, -1), SSL_ERROR_SSL))
            goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
static int cert_cb_cnt;

static int cert_cb(SSL *s, void *arg)
{
    SSL_CTX *ctx = (SSL_CTX *)arg;
    BIO *in = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *x509 = NULL, *rootx = NULL;
    STACK_OF(X509) *chain = NULL;
    char *rootfile = NULL, *ecdsacert = NULL, *ecdsakey = NULL;
    int ret = 0;

    if (cert_cb_cnt == 0) {
        /* Suspend the handshake */
        cert_cb_cnt++;
        return -1;
    } else if (cert_cb_cnt == 1) {
        /*
         * Update the SSL_CTX, set the certificate and private key and then
         * continue the handshake normally.
         */
        if (ctx != NULL && !TEST_ptr(SSL_set_SSL_CTX(s, ctx)))
            return 0;

        if (!TEST_true(SSL_use_certificate_file(s, cert, SSL_FILETYPE_PEM))
                || !TEST_true(SSL_use_PrivateKey_file(s, privkey,
                                                      SSL_FILETYPE_PEM))
                || !TEST_true(SSL_check_private_key(s)))
            return 0;
        cert_cb_cnt++;
        return 1;
    } else if (cert_cb_cnt == 3) {
        int rv;

        rootfile = test_mk_file_path(certsdir, "rootcert.pem");
        ecdsacert = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
        ecdsakey = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
        if (!TEST_ptr(rootfile) || !TEST_ptr(ecdsacert) || !TEST_ptr(ecdsakey))
            goto out;
        chain = sk_X509_new_null();
        if (!TEST_ptr(chain))
            goto out;
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, rootfile), 0)
                || !TEST_ptr(rootx = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &rootx, NULL, NULL))
                || !TEST_true(sk_X509_push(chain, rootx)))
            goto out;
        rootx = NULL;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsacert), 0)
                || !TEST_ptr(x509 = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &x509, NULL, NULL)))
            goto out;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsakey), 0)
                || !TEST_ptr(pkey = PEM_read_bio_PrivateKey_ex(in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
            goto out;
        rv = SSL_check_chain(s, x509, pkey, chain);
        /*
         * If the cert doesn't show as valid here (e.g., because we don't
         * have any shared sigalgs), then we will not set it, and there will
         * be no certificate at all on the SSL or SSL_CTX.  This, in turn,
         * will cause tls_choose_sigalgs() to fail the connection.
         */
        if ((rv & (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE))
                == (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE)) {
            if (!SSL_use_cert_and_key(s, x509, pkey, NULL, 1))
                goto out;
        }

        ret = 1;
    }

    /* Abort the handshake */
 out:
    OPENSSL_free(ecdsacert);
    OPENSSL_free(ecdsakey);
    OPENSSL_free(rootfile);
    BIO_free(in);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    X509_free(rootx);
    sk_X509_pop_free(chain, X509_free);
    return ret;
}

/*
 * Test the certificate callback.
 * Test 0: Callback fails
 * Test 1: Success - no SSL_set_SSL_CTX() in the callback
 * Test 2: Success - SSL_set_SSL_CTX() in the callback
 * Test 3: Success - Call SSL_check_chain from the callback
 * Test 4: Failure - SSL_check_chain fails from callback due to bad cert in the
 *                   chain
 * Test 5: Failure - SSL_check_chain fails from callback due to bad ee cert
 */
static int test_cert_cb_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *snictx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0, ret;

#ifdef OPENSSL_NO_EC
    /* We use an EC cert in these tests, so we skip in a no-ec build */
    if (tst >= 3)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, NULL, NULL)))
        goto end;

    if (tst == 0)
        cert_cb_cnt = -1;
    else if (tst >= 3)
        cert_cb_cnt = 3;
    else
        cert_cb_cnt = 0;

    if (tst == 2) {
        snictx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
        if (!TEST_ptr(snictx))
            goto end;
    }

    SSL_CTX_set_cert_cb(sctx, cert_cb, snictx);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (tst == 4) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the chain doesn't meet (the root uses an RSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                                             "ecdsa_secp256r1_sha256")))
            goto end;
    } else if (tst == 5) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the ee cert doesn't meet (the ee uses an ECDSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                           "rsa_pss_rsae_sha256:rsa_pkcs1_sha256")))
            goto end;
    }

    ret = create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE);
    if (!TEST_true(tst == 0 || tst == 4 || tst == 5 ? !ret : ret)
            || (tst > 0
                && !TEST_int_eq((cert_cb_cnt - 2) * (cert_cb_cnt - 3), 0))) {
        goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    SSL_CTX_free(snictx);

    return testresult;
}
#endif

static int test_cert_cb(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_cert_cb_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_cert_cb_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

static int client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
    X509 *xcert;
    EVP_PKEY *privpkey;
    BIO *in = NULL;
    BIO *priv_in = NULL;

    /* Check that SSL_get0_peer_certificate() returns something sensible */
    if (!TEST_ptr(SSL_get0_peer_certificate(ssl)))
        return 0;

    in = BIO_new_file(cert, "r");
    if (!TEST_ptr(in))
        return 0;

    if (!TEST_ptr(xcert = X509_new_ex(libctx, NULL))
            || !TEST_ptr(PEM_read_bio_X509(in, &xcert, NULL, NULL))
            || !TEST_ptr(priv_in = BIO_new_file(privkey, "r"))
            || !TEST_ptr(privpkey = PEM_read_bio_PrivateKey_ex(priv_in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
        goto err;

    *x509 = xcert;
    *pkey = privpkey;

    BIO_free(in);
    BIO_free(priv_in);
    return 1;
err:
    X509_free(xcert);
    BIO_free(in);
    BIO_free(priv_in);
    return 0;
}

static int test_client_cert_cb(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst == 1)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       tst == 0 ? TLS1_2_VERSION
                                                : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * Test that setting a client_cert_cb results in a client certificate being
     * sent.
     */
    SSL_CTX_set_client_cert_cb(cctx, client_cert_cb);
    SSL_CTX_set_verify(sctx,
                       SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       verify_cb);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
/*
 * Test setting certificate authorities on both client and server.
 *
 * Test 0: SSL_CTX_set0_CA_list() only
 * Test 1: Both SSL_CTX_set0_CA_list() and SSL_CTX_set_client_CA_list()
 * Test 2: Only SSL_CTX_set_client_CA_list()
 */
static int test_ca_names_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    size_t i;
    X509_NAME *name[] = { NULL, NULL, NULL, NULL };
    char *strnames[] = { "Jack", "Jill", "John", "Joanne" };
    STACK_OF(X509_NAME) *sk1 = NULL, *sk2 = NULL;
    const STACK_OF(X509_NAME) *sktmp = NULL;

    for (i = 0; i < OSSL_NELEM(name); i++) {
        name[i] = X509_NAME_new();
        if (!TEST_ptr(name[i])
                || !TEST_true(X509_NAME_add_entry_by_txt(name[i], "CN",
                                                         MBSTRING_ASC,
                                                         (unsigned char *)
                                                         strnames[i],
                                                         -1, -1, 0)))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    SSL_CTX_set_verify(sctx, SSL_VERIFY_PEER, NULL);

    if (tst == 0 || tst == 1) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[1])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[1]))))
            goto end;

        SSL_CTX_set0_CA_list(sctx, sk1);
        SSL_CTX_set0_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }
    if (tst == 1 || tst == 2) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[3])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[3]))))
            goto end;

        SSL_CTX_set_client_CA_list(sctx, sk1);
        SSL_CTX_set_client_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * We only expect certificate authorities to have been sent to the server
     * if we are using TLSv1.3 and SSL_set0_CA_list() was used
     */
    sktmp = SSL_get0_peer_CA_list(serverssl);
    if (prot == TLS1_3_VERSION
            && (tst == 0 || tst == 1)) {
        if (!TEST_ptr(sktmp)
                || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                              name[0]), 0)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                              name[1]), 0))
            goto end;
    } else if (!TEST_ptr_null(sktmp)) {
        goto end;
    }

    /*
     * In all tests we expect certificate authorities to have been sent to the
     * client. However, SSL_set_client_CA_list() should override
     * SSL_set0_CA_list()
     */
    sktmp = SSL_get0_peer_CA_list(clientssl);
    if (!TEST_ptr(sktmp)
            || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                          name[tst == 0 ? 0 : 2]), 0)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                          name[tst == 0 ? 1 : 3]), 0))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    for (i = 0; i < OSSL_NELEM(name); i++)
        X509_NAME_free(name[i]);
    sk_X509_NAME_pop_free(sk1, X509_NAME_free);
    sk_X509_NAME_pop_free(sk2, X509_NAME_free);

    return testresult;
}
#endif

static int test_ca_names(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_ca_names_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_ca_names_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

#ifndef OPENSSL_NO_TLS1_2
static const char *multiblock_cipherlist_data[]=
{
    "AES128-SHA",
    "AES128-SHA256",
    "AES256-SHA",
    "AES256-SHA256",
};

/* Reduce the fragment size - so the multiblock test buffer can be small */
# define MULTIBLOCK_FRAGSIZE 512

static int test_multiblock_write(int test_index)
{
    static const char *fetchable_ciphers[]=
    {
        "AES-128-CBC-HMAC-SHA1",
        "AES-128-CBC-HMAC-SHA256",
        "AES-256-CBC-HMAC-SHA1",
        "AES-256-CBC-HMAC-SHA256"
    };
    const char *cipherlist = multiblock_cipherlist_data[test_index];
    const SSL_METHOD *smeth = TLS_server_method();
    const SSL_METHOD *cmeth = TLS_client_method();
    int min_version = TLS1_VERSION;
    int max_version = TLS1_2_VERSION; /* Don't select TLS1_3 */
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /*
     * Choose a buffer large enough to perform a multi-block operation
     * i.e: write_len >= 4 * frag_size
     * 9 * is chosen so that multiple multiblocks are used + some leftover.
     */
    unsigned char msg[MULTIBLOCK_FRAGSIZE * 9];
    unsigned char buf[sizeof(msg)], *p = buf;
    size_t readbytes, written, len;
    EVP_CIPHER *ciph = NULL;

    /*
     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
    EVP_CIPHER_free(ciph);

    /* Set up a buffer with some data that will be sent to the client */
    RAND_bytes(msg, sizeof(msg));

    if (!TEST_true(create_ssl_ctx_pair(libctx, smeth, cmeth, min_version,
                                       max_version, &sctx, &cctx, cert,
                                       privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_max_send_fragment(sctx, MULTIBLOCK_FRAGSIZE)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
            goto end;

    /* settings to force it to use AES-CBC-HMAC_SHA */
    SSL_set_options(serverssl, SSL_OP_NO_ENCRYPT_THEN_MAC);
    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, cipherlist)))
       goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
        || !TEST_size_t_eq(written, sizeof(msg)))
        goto end;

    len = written;
    while (len > 0) {
        if (!TEST_true(SSL_read_ex(clientssl, p, MULTIBLOCK_FRAGSIZE, &readbytes)))
            goto end;
        p += readbytes;
        len -= readbytes;
    }
    if (!TEST_mem_eq(msg, sizeof(msg), buf, sizeof(buf)))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* OPENSSL_NO_TLS1_2 */

static int test_session_timeout(int test)
{
    /*
     * Test session ordering and timeout
     * Can't explicitly test performance of the new code,
     * but can test to see if the ordering of the sessions
     * are correct, and they they are removed as expected
     */
    SSL_SESSION *early = NULL;
    SSL_SESSION *middle = NULL;
    SSL_SESSION *late = NULL;
    SSL_CTX *ctx;
    int testresult = 0;
    long now = (long)time(NULL);
#define TIMEOUT 10

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_method()))
        || !TEST_ptr(early = SSL_SESSION_new())
        || !TEST_ptr(middle = SSL_SESSION_new())
        || !TEST_ptr(late = SSL_SESSION_new()))
        goto end;

    /* assign unique session ids */
    early->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(early->session_id, 1, SSL3_SSL_SESSION_ID_LENGTH);
    middle->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(middle->session_id, 2, SSL3_SSL_SESSION_ID_LENGTH);
    late->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(late->session_id, 3, SSL3_SSL_SESSION_ID_LENGTH);

    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_time(early, now - 10), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(middle, now), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(late, now + 10), 0))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_timeout(early, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(middle, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(late, TIMEOUT), 0))
        goto end;

    /* Make sure they are all still there */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* Make sure they are in the expected order */
    if (!TEST_ptr_eq(late->next, middle)
        || !TEST_ptr_eq(middle->next, early)
        || !TEST_ptr_eq(early->prev, middle)
        || !TEST_ptr_eq(middle->prev, late))
        goto end;

    /* This should remove "early" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT - 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "middle" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "late" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 11);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    /* Add them back in again */
    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove all of them */
    SSL_CTX_flush_sessions(ctx, 0);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    (void)SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_UPDATE_TIME
                                         | SSL_CTX_get_session_cache_mode(ctx));

    /* make sure |now| is NOT  equal to the current time */
    now -= 10;
    if (!TEST_int_ne(SSL_SESSION_set_time(early, now), 0)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_long_ne(SSL_SESSION_get_time(early), now))
        goto end;

    testresult = 1;
 end:
    SSL_CTX_free(ctx);
    SSL_SESSION_free(early);
    SSL_SESSION_free(middle);
    SSL_SESSION_free(late);
    return testresult;
}

/*
 * Test 0: Client sets servername and server acknowledges it (TLSv1.2)
 * Test 1: Client sets servername and server does not acknowledge it (TLSv1.2)
 * Test 2: Client sets inconsistent servername on resumption (TLSv1.2)
 * Test 3: Client does not set servername on initial handshake (TLSv1.2)
 * Test 4: Client does not set servername on resumption handshake (TLSv1.2)
 * Test 5: Client sets servername and server acknowledges it (TLSv1.3)
 * Test 6: Client sets servername and server does not acknowledge it (TLSv1.3)
 * Test 7: Client sets inconsistent servername on resumption (TLSv1.3)
 * Test 8: Client does not set servername on initial handshake(TLSv1.3)
 * Test 9: Client does not set servername on resumption handshake (TLSv1.3)
 */
static int test_servername(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;
    const char *sexpectedhost = NULL, *cexpectedhost = NULL;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 4)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 5)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 4) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst != 1 && tst != 6) {
        if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx,
                                                              hostname_cb)))
            goto end;
    }

    if (tst != 3 && tst != 8) {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        sexpectedhost = cexpectedhost = "goodhost";
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(SSL_get_servername(clientssl, TLSEXT_NAMETYPE_host_name),
                     cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    /* Now repeat with a resumption handshake */

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0)
            || !TEST_ptr_ne(sess = SSL_get1_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))
            || !TEST_int_eq(SSL_shutdown(serverssl), 0))
        goto end;

    SSL_free(clientssl);
    SSL_free(serverssl);
    clientssl = serverssl = NULL;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL)))
        goto end;

    if (!TEST_true(SSL_set_session(clientssl, sess)))
        goto end;

    sexpectedhost = cexpectedhost = "goodhost";
    if (tst == 2 || tst == 7) {
        /* Set an inconsistent hostname */
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "altgoodhost")))
            goto end;
        /*
         * In TLSv1.2 we expect the hostname from the original handshake, in
         * TLSv1.3 we expect the hostname from this handshake
         */
        if (tst == 7)
            sexpectedhost = cexpectedhost = "altgoodhost";

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "altgoodhost"))
            goto end;
    } else if (tst == 4 || tst == 9) {
        /*
         * A TLSv1.3 session does not associate a session with a servername,
         * but a TLSv1.2 session does.
         */
        if (tst == 9)
            sexpectedhost = cexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         cexpectedhost))
            goto end;
    } else {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        /*
         * In a TLSv1.2 resumption where the hostname was not acknowledged
         * we expect the hostname on the server to be empty. On the client we
         * return what was requested in this case.
         *
         * Similarly if the client didn't set a hostname on an original TLSv1.2
         * session but is now, the server hostname will be empty, but the client
         * is as we set it.
         */
        if (tst == 1 || tst == 3)
            sexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "goodhost"))
            goto end;
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_session_reused(clientssl))
            || !TEST_true(SSL_session_reused(serverssl))
            || !TEST_str_eq(SSL_get_servername(clientssl,
                                               TLSEXT_NAMETYPE_host_name),
                            cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
/*
 * Test that if signature algorithms are not available, then we do not offer or
 * accept them.
 * Test 0: Two RSA sig algs available: both RSA sig algs shared
 * Test 1: The client only has SHA2-256: only SHA2-256 algorithms shared
 * Test 2: The server only has SHA2-256: only SHA2-256 algorithms shared
 * Test 3: An RSA and an ECDSA sig alg available: both sig algs shared
 * Test 4: The client only has an ECDSA sig alg: only ECDSA algorithms shared
 * Test 5: The server only has an ECDSA sig alg: only ECDSA algorithms shared
 */
static int test_sigalgs_available(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_LIB_CTX *clientctx = libctx, *serverctx = libctx;
    OSSL_PROVIDER *filterprov = NULL;
    int sig, hash;

    if (!TEST_ptr(tmpctx))
        goto end;

    if (idx != 0 && idx != 3) {
        if (!TEST_true(OSSL_PROVIDER_add_builtin(tmpctx, "filter",
                                                 filter_provider_init)))
            goto end;

        filterprov = OSSL_PROVIDER_load(tmpctx, "filter");
        if (!TEST_ptr(filterprov))
            goto end;

        if (idx < 3) {
            /*
             * Only enable SHA2-256 so rsa_pss_rsae_sha384 should not be offered
             * or accepted for the peer that uses this libctx. Note that libssl
             * *requires* SHA2-256 to be available so we cannot disable that. We
             * also need SHA1 for our certificate.
             */
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_DIGEST,
                                                      "SHA2-256:SHA1")))
                goto end;
        } else {
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_SIGNATURE,
                                                      "ECDSA"))
                    || !TEST_true(filter_provider_set_filter(OSSL_OP_KEYMGMT,
                                                             "EC:X25519:X448")))
                goto end;
        }

        if (idx == 1 || idx == 4)
            clientctx = tmpctx;
        else
            serverctx = tmpctx;
    }

    cctx = SSL_CTX_new_ex(clientctx, NULL, TLS_client_method());
    sctx = SSL_CTX_new_ex(serverctx, NULL, TLS_server_method());
    if (!TEST_ptr(cctx) || !TEST_ptr(sctx))
        goto end;

    if (idx != 5) {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;
    } else {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert2, privkey2)))
            goto end;
    }

    /* Ensure we only use TLSv1.2 ciphersuites based on SHA256 */
    if (idx < 4) {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-RSA-AES128-GCM-SHA256")))
            goto end;
    } else {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-ECDSA-AES128-GCM-SHA256")))
            goto end;
    }

    if (idx < 3) {
        if (!SSL_CTX_set1_sigalgs_list(cctx,
                                       "rsa_pss_rsae_sha384"
                                       ":rsa_pss_rsae_sha256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha384"
                                              ":rsa_pss_rsae_sha256"))
            goto end;
    } else {
        if (!SSL_CTX_set1_sigalgs_list(cctx, "rsa_pss_rsae_sha256:ECDSA+SHA256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha256:ECDSA+SHA256"))
            goto end;
    }

    if (idx != 5
        && (!TEST_int_eq(SSL_CTX_use_certificate_file(sctx, cert2,
                                                      SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_use_PrivateKey_file(sctx,
                                                        privkey2,
                                                        SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_check_private_key(sctx), 1)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    /* For tests 0 and 3 we expect 2 shared sigalgs, otherwise exactly 1 */
    if (!TEST_int_eq(SSL_get_shared_sigalgs(serverssl, 0, &sig, &hash, NULL,
                                            NULL, NULL),
                     (idx == 0 || idx == 3) ? 2 : 1))
        goto end;

    if (!TEST_int_eq(hash, idx == 0 ? NID_sha384 : NID_sha256))
        goto end;

    if (!TEST_int_eq(sig, (idx == 4 || idx == 5) ? EVP_PKEY_EC
                                                 : NID_rsassaPss))
        goto end;

    testresult = filter_provider_check_clean_finish();

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(filterprov);
    OSSL_LIB_CTX_free(tmpctx);

    return testresult;
}
#endif /*
        * !defined(OPENSSL_NO_EC) \
        * && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
        */

#ifndef OPENSSL_NO_TLS1_3
/* This test can run in TLSv1.3 even if ec and dh are disabled */
static int test_pluggable_group(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_PROVIDER *tlsprov = OSSL_PROVIDER_load(libctx, "tls-provider");
    /* Check that we are not impacted by a provider without any groups */
    OSSL_PROVIDER *legacyprov = OSSL_PROVIDER_load(libctx, "legacy");
    const char *group_name = idx == 0 ? "xorgroup" : "xorkemgroup";

    if (!TEST_ptr(tlsprov))
        goto end;

    if (legacyprov == NULL) {
        /*
         * In this case we assume we've been built with "no-legacy" and skip
         * this test (there is no OPENSSL_NO_LEGACY)
         */
        testresult = 1;
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION,
                                       TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set1_groups_list(serverssl, group_name))
            || !TEST_true(SSL_set1_groups_list(clientssl, group_name)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(group_name,
                     SSL_group_to_name(serverssl, SSL_get_shared_group(serverssl, 0))))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(tlsprov);
    OSSL_PROVIDER_unload(legacyprov);

    return testresult;
}
#endif

#ifndef OPENSSL_NO_TLS1_2
static int test_ssl_dup(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL, *client2ssl = NULL;
    int testresult = 0;
    BIO *rbio = NULL, *wbio = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_min_proto_version(clientssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(clientssl, TLS1_2_VERSION)))
        goto end;

    client2ssl = SSL_dup(clientssl);
    rbio = SSL_get_rbio(clientssl);
    if (!TEST_ptr(rbio)
            || !TEST_true(BIO_up_ref(rbio)))
        goto end;
    SSL_set0_rbio(client2ssl, rbio);
    rbio = NULL;

    wbio = SSL_get_wbio(clientssl);
    if (!TEST_ptr(wbio) || !TEST_true(BIO_up_ref(wbio)))
        goto end;
    SSL_set0_wbio(client2ssl, wbio);
    rbio = NULL;

    if (!TEST_ptr(client2ssl)
               /* Handshake not started so pointers should be different */
            || !TEST_ptr_ne(clientssl, client2ssl))
        goto end;

    if (!TEST_int_eq(SSL_get_min_proto_version(client2ssl), TLS1_2_VERSION)
            || !TEST_int_eq(SSL_get_max_proto_version(client2ssl), TLS1_2_VERSION))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, client2ssl, SSL_ERROR_NONE)))
        goto end;

    SSL_free(clientssl);
    clientssl = SSL_dup(client2ssl);
    if (!TEST_ptr(clientssl)
               /* Handshake has finished so pointers should be the same */
            || !TEST_ptr_eq(clientssl, client2ssl))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_free(client2ssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

# ifndef OPENSSL_NO_DH

static EVP_PKEY *tmp_dh_params = NULL;

/* Helper function for the test_set_tmp_dh() tests */
static EVP_PKEY *get_tmp_dh_params(void)
{
    if (tmp_dh_params == NULL) {
        BIGNUM *p = NULL;
        OSSL_PARAM_BLD *tmpl = NULL;
        EVP_PKEY_CTX *pctx = NULL;
        OSSL_PARAM *params = NULL;
        EVP_PKEY *dhpkey = NULL;

        p = BN_get_rfc3526_prime_2048(NULL);
        if (!TEST_ptr(p))
            goto end;

        pctx = EVP_PKEY_CTX_new_from_name(libctx, "DH", NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_fromdata_init(pctx), 1))
            goto end;

        tmpl = OSSL_PARAM_BLD_new();
        if (!TEST_ptr(tmpl)
                || !TEST_true(OSSL_PARAM_BLD_push_BN(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_P,
                                                        p))
                || !TEST_true(OSSL_PARAM_BLD_push_uint(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_G,
                                                        2)))
            goto end;

        params = OSSL_PARAM_BLD_to_param(tmpl);
        if (!TEST_ptr(params)
                || !TEST_int_eq(EVP_PKEY_fromdata(pctx, &dhpkey,
                                                  EVP_PKEY_KEY_PARAMETERS,
                                                  params), 1))
            goto end;

        tmp_dh_params = dhpkey;
    end:
        BN_free(p);
        EVP_PKEY_CTX_free(pctx);
        OSSL_PARAM_BLD_free(tmpl);
        OSSL_PARAM_free(params);
    }

    if (tmp_dh_params != NULL && !EVP_PKEY_up_ref(tmp_dh_params))
        return NULL;

    return tmp_dh_params;
}

#  ifndef OPENSSL_NO_DEPRECATED_3_0
/* Callback used by test_set_tmp_dh() */
static DH *tmp_dh_callback(SSL *s, int is_export, int keylen)
{
    EVP_PKEY *dhpkey = get_tmp_dh_params();
    DH *ret = NULL;

    if (!TEST_ptr(dhpkey))
        return NULL;

    /*
     * libssl does not free the returned DH, so we free it now knowing that even
     * after we free dhpkey, there will still be a reference to the owning
     * EVP_PKEY in tmp_dh_params, and so the DH object will live for the length
     * of time we need it for.
     */
    ret = EVP_PKEY_get1_DH(dhpkey);
    DH_free(ret);

    EVP_PKEY_free(dhpkey);

    return ret;
}
#  endif

/*
 * Test the various methods for setting temporary DH parameters
 *
 * Test  0: Default (no auto) setting
 * Test  1: Explicit SSL_CTX auto off
 * Test  2: Explicit SSL auto off
 * Test  3: Explicit SSL_CTX auto on
 * Test  4: Explicit SSL auto on
 * Test  5: Explicit SSL_CTX auto off, custom DH params via EVP_PKEY
 * Test  6: Explicit SSL auto off, custom DH params via EVP_PKEY
 *
 * The following are testing deprecated APIs, so we only run them if available
 * Test  7: Explicit SSL_CTX auto off, custom DH params via DH
 * Test  8: Explicit SSL auto off, custom DH params via DH
 * Test  9: Explicit SSL_CTX auto off, custom DH params via callback
 * Test 10: Explicit SSL auto off, custom DH params via callback
 */
static int test_set_tmp_dh(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int dhauto = (idx == 3 || idx == 4) ? 1 : 0;
    int expected = (idx <= 2) ? 0 : 1;
    EVP_PKEY *dhpkey = NULL;
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH *dh = NULL;
#  else

    if (idx >= 7)
        return 1;
#  endif

    if (idx >= 5 && idx <= 8) {
        dhpkey = get_tmp_dh_params();
        if (!TEST_ptr(dhpkey))
            goto end;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    if (idx == 7 || idx == 8) {
        dh = EVP_PKEY_get1_DH(dhpkey);
        if (!TEST_ptr(dh))
            goto end;
    }
#  endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if ((idx & 1) == 1) {
        if (!TEST_true(SSL_CTX_set_dh_auto(sctx, dhauto)))
            goto end;
    }

    if (idx == 5) {
        if (!TEST_true(SSL_CTX_set0_tmp_dh_pkey(sctx, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 7) {
        if (!TEST_true(SSL_CTX_set_tmp_dh(sctx, dh)))
            goto end;
    } else if (idx == 9) {
        SSL_CTX_set_tmp_dh_callback(sctx, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if ((idx & 1) == 0 && idx != 0) {
        if (!TEST_true(SSL_set_dh_auto(serverssl, dhauto)))
            goto end;
    }
    if (idx == 6) {
        if (!TEST_true(SSL_set0_tmp_dh_pkey(serverssl, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 8) {
        if (!TEST_true(SSL_set_tmp_dh(serverssl, dh)))
            goto end;
    } else if (idx == 10) {
        SSL_set_tmp_dh_callback(serverssl, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, "DHE-RSA-AES128-SHA")))
        goto end;

    /*
     * If autoon then we should succeed. Otherwise we expect failure because
     * there are no parameters
     */
    if (!TEST_int_eq(create_ssl_connection(serverssl, clientssl,
                                           SSL_ERROR_NONE), expected))
        goto end;

    testresult = 1;

 end:
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH_free(dh);
#  endif
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(dhpkey);

    return testresult;
}

/*
 * Test the auto DH keys are appropriately sized
 */
static int test_dh_auto(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    EVP_PKEY *tmpkey = NULL;
    char *thiscert = NULL, *thiskey = NULL;
    size_t expdhsize = 0;
    const char *ciphersuite = "DHE-RSA-AES128-SHA";

    switch (idx) {
    case 0:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        thiscert = cert1024;
        thiskey = privkey1024;
        expdhsize = 1024;
        break;
    case 1:
        /* 2048 bit prime */
        thiscert = cert;
        thiskey = privkey;
        expdhsize = 2048;
        break;
    case 2:
        thiscert = cert3072;
        thiskey = privkey3072;
        expdhsize = 3072;
        break;
    case 3:
        thiscert = cert4096;
        thiskey = privkey4096;
        expdhsize = 4096;
        break;
    case 4:
        thiscert = cert8192;
        thiskey = privkey8192;
        expdhsize = 8192;
        break;
    /* No certificate cases */
    case 5:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        ciphersuite = "ADH-AES128-SHA256:@SECLEVEL=0";
        expdhsize = 1024;
        break;
    case 6:
        ciphersuite = "ADH-AES256-SHA256:@SECLEVEL=0";
        expdhsize = 3072;
        break;
    default:
        TEST_error("Invalid text index");
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, thiscert, thiskey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_dh_auto(serverssl, 1))
            || !TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, ciphersuite))
            || !TEST_true(SSL_set_cipher_list(clientssl, ciphersuite)))
        goto end;

    /*
     * Send the server's first flight. At this point the server has created the
     * temporary DH key but hasn't finished using it yet. Once used it is
     * removed, so we cannot test it.
     */
    if (!TEST_int_le(SSL_connect(clientssl), 0)
            || !TEST_int_le(SSL_accept(serverssl), 0))
        goto end;

    if (!TEST_int_gt(SSL_get_tmp_key(serverssl, &tmpkey), 0))
        goto end;
    if (!TEST_size_t_eq(EVP_PKEY_get_bits(tmpkey), expdhsize))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(tmpkey);

    return testresult;

}
# endif /* OPENSSL_NO_DH */
#endif /* OPENSSL_NO_TLS1_2 */

#ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Test that setting an SNI callback works with TLSv1.3. Specifically we check
 * that it works even without a certificate configured for the original
 * SSL_CTX
 */
static int test_sni_tls13(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *sctx2 = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /* Reset callback counter */
    snicb = 0;

    /* Create an initial SSL_CTX with no certificate configured */
    sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(sctx))
        goto end;
    /* Require TLSv1.3 as a minimum */
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_3_VERSION, 0,
                                       &sctx2, &cctx, cert, privkey)))
        goto end;

    /* Set up SNI */
    if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx, sni_cb))
            || !TEST_true(SSL_CTX_set_tlsext_servername_arg(sctx, sctx2)))
        goto end;

    /*
     * Connection should still succeed because the final SSL_CTX has the right
     * certificates configured.
     */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /* We should have had the SNI callback called exactly once */
    if (!TEST_int_eq(snicb, 1))
        goto end;

    testresult = 1;

end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}

/*
 * Test that the lifetime hint of a TLSv1.3 ticket is no more than 1 week
 * 0 = TLSv1.2
 * 1 = TLSv1.3
 */
static int test_ticket_lifetime(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int version = TLS1_3_VERSION;

#define ONE_WEEK_SEC (7 * 24 * 60 * 60)
#define TWO_WEEK_SEC (2 * ONE_WEEK_SEC)

    if (idx == 0) {
#ifdef OPENSSL_NO_TLS1_2
        return TEST_skip("TLS 1.2 is disabled.");
#else
        version = TLS1_2_VERSION;
#endif
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), version, version,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL)))
        goto end;

    /*
     * Set the timeout to be more than 1 week
     * make sure the returned value is the default
     */
    if (!TEST_long_eq(SSL_CTX_set_timeout(sctx, TWO_WEEK_SEC),
                      SSL_get_default_timeout(serverssl)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (idx == 0) {
        /* TLSv1.2 uses the set value */
        if (!TEST_ulong_eq(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), TWO_WEEK_SEC))
            goto end;
    } else {
        /* TLSv1.3 uses the limited value */
        if (!TEST_ulong_le(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), ONE_WEEK_SEC))
            goto end;
    }
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
 * Test that setting an ALPN does not violate RFC
 */
static int test_set_alpn(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int testresult = 0;

    unsigned char bad0[] = { 0x00, 'b', 'a', 'd' };
    unsigned char good[] = { 0x04, 'g', 'o', 'o', 'd' };
    unsigned char bad1[] = { 0x01, 'b', 'a', 'd' };
    unsigned char bad2[] = { 0x03, 'b', 'a', 'd', 0x00};
    unsigned char bad3[] = { 0x03, 'b', 'a', 'd', 0x01, 'b', 'a', 'd'};
    unsigned char bad4[] = { 0x03, 'b', 'a', 'd', 0x06, 'b', 'a', 'd'};

    /* Create an initial SSL_CTX with no certificate configured */
    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    /* the set_alpn functions return 0 (false) on success, non-zero (true) on failure */
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, 0)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, good, 1)))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad4, sizeof(bad4))))
        goto end;

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    if (!TEST_false(SSL_set_alpn_protos(ssl, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, 0)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, good, 1)))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad4, sizeof(bad4))))
        goto end;

    testresult = 1;

end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return testresult;
}

/*
 * Test SSL_CTX_set1_verify/chain_cert_store and SSL_CTX_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl_ctx(void)
{
   SSL_CTX *ctx = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, new_store)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, NULL)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_CTX_free(ctx);
   return testresult;
}

/*
 * Test SSL_set1_verify/chain_cert_store and SSL_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl(void)
{
   SSL_CTX *ctx = NULL;
   SSL *ssl = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Create an SSL object. */
   ssl = SSL_new(ctx);
   if (!TEST_ptr(ssl))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, new_store)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, NULL)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_free(ssl);
   SSL_CTX_free(ctx);
   return testresult;
}


static int test_inherit_verify_param(void)
{
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    X509_VERIFY_PARAM *cp = NULL;
    SSL *ssl = NULL;
    X509_VERIFY_PARAM *sp = NULL;
    int hostflags = X509_CHECK_FLAG_NEVER_CHECK_SUBJECT;

    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    cp = SSL_CTX_get0_param(ctx);
    if (!TEST_ptr(cp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(cp), 0))
        goto end;

    X509_VERIFY_PARAM_set_hostflags(cp, hostflags);

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    sp = SSL_get0_param(ssl);
    if (!TEST_ptr(sp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(sp), hostflags))
        goto end;

    testresult = 1;

 end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return testresult;
}


static int test_load_dhfile(void)
{
#ifndef OPENSSL_NO_DH
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    SSL_CONF_CTX *cctx = NULL;

    if (dhfile == NULL)
        return 1;

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_client_method()))
        || !TEST_ptr(cctx = SSL_CONF_CTX_new()))
        goto end;

    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
    SSL_CONF_CTX_set_flags(cctx,
                           SSL_CONF_FLAG_CERTIFICATE
                           | SSL_CONF_FLAG_SERVER
                           | SSL_CONF_FLAG_FILE);

    if (!TEST_int_eq(SSL_CONF_cmd(cctx, "DHParameters", dhfile), 2))
        goto end;

    testresult = 1;
end:
    SSL_CONF_CTX_free(cctx);
    SSL_CTX_free(ctx);

    return testresult;
#else
    return TEST_skip("DH not supported by this build");
#endif
}

#ifndef OPENSSL_NO_QUIC
static int test_quic_set_encryption_secrets(SSL *ssl,
                                            OSSL_ENCRYPTION_LEVEL level,
                                            const uint8_t *read_secret,
                                            const uint8_t *write_secret,
                                            size_t secret_len)
{
    test_printf_stderr("quic_set_encryption_secrets() %s, lvl=%d, len=%zd\n",
                       ssl->server ? "server" : "client", level, secret_len);
    return 1;
}

static int test_quic_add_handshake_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                                        const uint8_t *data, size_t len)
{
    SSL *peer = (SSL*)SSL_get_app_data(ssl);

    TEST_info("quic_add_handshake_data() %s, lvl=%d, *data=0x%02X, len=%zd\n",
              ssl->server ? "server" : "client", level, (int)*data, len);
    if (!TEST_ptr(peer))
        return 0;

    /* We're called with what is locally written; this gives it to the peer */
    if (!TEST_true(SSL_provide_quic_data(peer, level, data, len))) {
        ERR_print_errors_fp(stderr);
        return 0;
    }

    return 1;
}

static int test_quic_flush_flight(SSL *ssl)
{
    test_printf_stderr("quic_flush_flight() %s\n", ssl->server ? "server" : "client");
    return 1;
}

static int test_quic_send_alert(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert)
{
    test_printf_stderr("quic_send_alert() %s, lvl=%d, alert=%d\n",
                       ssl->server ? "server" : "client", level, alert);
    return 1;
}

static SSL_QUIC_METHOD quic_method = {
    test_quic_set_encryption_secrets,
    test_quic_add_handshake_data,
    test_quic_flush_flight,
    test_quic_send_alert,
};

static int test_quic_api_set_versions(SSL *ssl, int ver)
{
    SSL_set_quic_transport_version(ssl, ver);
    return 1;
}

static int test_quic_api_version(int clnt, int srvr)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";
    const uint8_t *peer_str;
    size_t peer_str_len;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);

    if (!TEST_true(create_ssl_ctx_pair(libctx,
                                       TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION, 0,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_true(SSL_CTX_set_quic_method(cctx, &quic_method))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                             &clientssl, NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(serverssl, clientssl))
            || !TEST_true(SSL_set_app_data(clientssl, serverssl))
            || !TEST_true(test_quic_api_set_versions(clientssl, clnt))
            || !TEST_true(test_quic_api_set_versions(serverssl, srvr))
            || !TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                     SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_version(serverssl) == TLS1_3_VERSION)
            || !TEST_true(SSL_version(clientssl) == TLS1_3_VERSION)
            || !(TEST_int_eq(SSL_quic_read_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_read_level(serverssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(serverssl), ssl_encryption_application)))
        goto end;

    SSL_get_peer_quic_transport_params(serverssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, client_str, strlen(client_str)+1))
        goto end;
    SSL_get_peer_quic_transport_params(clientssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, server_str, strlen(server_str)+1))
        goto end;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(clientssl)))
        goto end;

    /* Dummy handshake call should succeed */
    if (!TEST_true(SSL_do_handshake(clientssl)))
        goto end;
    /* Test that we (correctly) fail to send KeyUpdate */
    if (!TEST_true(SSL_key_update(clientssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(clientssl), 0))
        goto end;
    if (!TEST_true(SSL_key_update(serverssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(serverssl), 0))
        goto end;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (srvr == 0 && clnt == 0)
        srvr = clnt = TLSEXT_TYPE_quic_transport_parameters;
    else if (srvr == 0)
        srvr = clnt;
    else if (clnt == 0)
        clnt = srvr;
    TEST_info("expected clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(serverssl), clnt))
        goto end;
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(clientssl), srvr))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_quic_api(int tst)
{
    SSL_CTX *sctx = NULL;
    SSL *serverssl = NULL;
    int testresult = 0;
    static int clnt_params[] = { 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int srvr_params[] = { 0,
                                 0,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int results[] = { 1, 1, 1, 1, 1, 0, 1, 0, 1 };

    /* Failure cases:
     * test 6/[5] clnt = parameters, srvr = draft
     * test 8/[7] clnt = draft, srvr = parameters
     */

    /* Clean up logging space */
    memset(client_log_buffer, 0, sizeof(client_log_buffer));
    memset(server_log_buffer, 0, sizeof(server_log_buffer));
    client_log_buffer_index = 0;
    server_log_buffer_index = 0;
    error_writing_log = 0;

    if (!TEST_ptr(sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method()))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_ptr(sctx->quic_method)
            || !TEST_ptr(serverssl = SSL_new(sctx))
            || !TEST_true(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, NULL))
            || !TEST_false(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, &quic_method))
            || !TEST_true(SSL_IS_QUIC(serverssl)))
        goto end;

    if (!TEST_int_eq(test_quic_api_version(clnt_params[tst], srvr_params[tst]), results[tst]))
        goto end;

    testresult = 1;

end:
    SSL_CTX_free(sctx);
    sctx = NULL;
    SSL_free(serverssl);
    serverssl = NULL;
    return testresult;
}

# ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Helper method to setup objects for QUIC early data test. Caller
 * frees objects on error.
 */
static int quic_setupearly_data_test(SSL_CTX **cctx, SSL_CTX **sctx,
                                     SSL **clientssl, SSL **serverssl,
                                     SSL_SESSION **sess, int idx)
{
    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";

    if (*sctx == NULL
            && (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                               TLS_client_method(),
                                               TLS1_3_VERSION, 0,
                                               sctx, cctx, cert, privkey))
                || !TEST_true(SSL_CTX_set_quic_method(*sctx, &quic_method))
                || !TEST_true(SSL_CTX_set_quic_method(*cctx, &quic_method))
                || !TEST_true(SSL_CTX_set_max_early_data(*sctx, 0xffffffffu))))
        return 0;

    if (idx == 1) {
        /* When idx == 1 we repeat the tests with read_ahead set */
        SSL_CTX_set_read_ahead(*cctx, 1);
        SSL_CTX_set_read_ahead(*sctx, 1);
    } else if (idx == 2) {
        /* When idx == 2 we are doing early_data with a PSK. Set up callbacks */
        SSL_CTX_set_psk_use_session_callback(*cctx, use_session_cb);
        SSL_CTX_set_psk_find_session_callback(*sctx, find_session_cb);
        use_session_cb_cnt = 0;
        find_session_cb_cnt = 0;
        srvid = pskid;
    }

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl, clientssl,
                                      NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    /*
     * For one of the run throughs (doesn't matter which one), we'll try sending
     * some SNI data in the initial ClientHello. This will be ignored (because
     * there is no SNI cb set up by the server), so it should not impact
     * early_data.
     */
    if (idx == 1
            && !TEST_true(SSL_set_tlsext_host_name(*clientssl, "localhost")))
        return 0;

    if (idx == 2) {
        clientpsk = create_a_psk(*clientssl);
        if (!TEST_ptr(clientpsk)
                || !TEST_true(SSL_SESSION_set_max_early_data(clientpsk,
                                                             0xffffffffu))
                || !TEST_true(SSL_SESSION_up_ref(clientpsk))) {
            SSL_SESSION_free(clientpsk);
            clientpsk = NULL;
            return 0;
        }
        serverpsk = clientpsk;

        if (sess != NULL) {
            if (!TEST_true(SSL_SESSION_up_ref(clientpsk))) {
                SSL_SESSION_free(clientpsk);
                SSL_SESSION_free(serverpsk);
                clientpsk = serverpsk = NULL;
                return 0;
            }
            *sess = clientpsk;
        }

        SSL_set_quic_early_data_enabled(*serverssl, 1);
        SSL_set_quic_early_data_enabled(*clientssl, 1);

        return 1;
    }

    if (sess == NULL)
        return 1;

    if (!TEST_true(create_bare_ssl_connection(*serverssl, *clientssl,
                                              SSL_ERROR_NONE, 0)))
        return 0;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(*clientssl)))
        return 0;

    *sess = SSL_get1_session(*clientssl);
    SSL_shutdown(*clientssl);
    SSL_shutdown(*serverssl);
    SSL_free(*serverssl);
    SSL_free(*clientssl);
    *serverssl = *clientssl = NULL;

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl,
                                      clientssl, NULL, NULL))
            || !TEST_true(SSL_set_session(*clientssl, *sess))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    SSL_set_quic_early_data_enabled(*serverssl, 1);
    SSL_set_quic_early_data_enabled(*clientssl, 1);

    return 1;
}

static int test_quic_early_data(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;

    if (!TEST_true(quic_setupearly_data_test(&cctx, &sctx, &clientssl,
                                             &serverssl, &sess, tst)))
        goto end;

    if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_get_early_data_status(serverssl)))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_SESSION_free(clientpsk);
    SSL_SESSION_free(serverpsk);
    clientpsk = serverpsk = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}
# endif /* OSSL_NO_USABLE_TLS1_3 */
#endif /* OPENSSL_NO_QUIC */

OPT_TEST_DECLARE_USAGE("certfile privkeyfile srpvfile tmpfile provider config dhfile\n")

int setup_tests(void)
{
    char *modulename;
    char *configfile;

    libctx = OSSL_LIB_CTX_new();
    if (!TEST_ptr(libctx))
        return 0;

    defctxnull = OSSL_PROVIDER_load(NULL, "null");

    /*
     * Verify that the default and fips providers in the default libctx are not
     * available
     */
    if (!TEST_false(OSSL_PROVIDER_available(NULL, "default"))
            || !TEST_false(OSSL_PROVIDER_available(NULL, "fips")))
        return 0;

    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(certsdir = test_get_argument(0))
            || !TEST_ptr(srpvfile = test_get_argument(1))
            || !TEST_ptr(tmpfilename = test_get_argument(2))
            || !TEST_ptr(modulename = test_get_argument(3))
            || !TEST_ptr(configfile = test_get_argument(4))
            || !TEST_ptr(dhfile = test_get_argument(5)))
        return 0;

    if (!TEST_true(OSSL_LIB_CTX_load_config(libctx, configfile)))
        return 0;

    /* Check we have the expected provider available */
    if (!TEST_true(OSSL_PROVIDER_available(libctx, modulename)))
        return 0;

    /* Check the default provider is not available */
    if (strcmp(modulename, "default") != 0
            && !TEST_false(OSSL_PROVIDER_available(libctx, "default")))
        return 0;

    if (strcmp(modulename, "fips") == 0)
        is_fips = 1;

    /*
     * We add, but don't load the test "tls-provider". We'll load it when we
     * need it.
     */
    if (!TEST_true(OSSL_PROVIDER_add_builtin(libctx, "tls-provider",
                                             tls_provider_init)))
        return 0;


    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
    } else if (tst >= 8) {
        if (!TEST_true(SSL_CTX_set_tlsext_ticket_key_cb(sctx, tick_key_cb)))
            goto end;
#endif
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * The decrypt ticket key callback in TLSv1.2 should be called even though
     * we have no ticket yet, because it gets called with a status of
     * SSL_TICKET_EMPTY (the client indicates support for tickets but does not
     * actually send any ticket data). This does not happen in TLSv1.3 because
     * it is not valid to send empty ticket data in TLSv1.3.
     */
    if (!TEST_int_eq(gen_tick_called, 1)
            || !TEST_int_eq(dec_tick_called, ((tst % 2) == 0) ? 1 : 0))
        goto end;

    gen_tick_called = dec_tick_called = 0;

    clntsess = SSL_get1_session(clientssl);
    SSL_shutdown(clientssl);
    SSL_shutdown(serverssl);
    SSL_free(serverssl);
    SSL_free(clientssl);
    serverssl = clientssl = NULL;

    /* Now do a resumption */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL))
            || !TEST_true(SSL_set_session(clientssl, clntsess))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    if (tick_dec_ret == SSL_TICKET_RETURN_IGNORE
            || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
            || tick_key_renew == -1) {
        if (!TEST_false(SSL_session_reused(clientssl)))
            goto end;
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
            goto end;
    }

    if (!TEST_int_eq(gen_tick_called,
                     (tick_key_renew
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(clntsess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
    } else {
        if (!TEST_true(SSL_session_reused(clientssl)))
            goto end;
    }

    if (!TEST_int_eq(gen_tick_called,
                     (tick_key_renew
                      || tick_dec_ret == SSL_TICKET_RETURN_IGNORE_RENEW
                      || tick_dec_ret == SSL_TICKET_RETURN_USE_RENEW)
                     ? 1 : 0)
               /* There is no ticket to decrypt in tests 13 and 19 */
            || !TEST_int_eq(dec_tick_called, (tst == 13 || tst == 19) ? 0 : 1))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(clntsess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test incorrect shutdown.
 * Test 0: client does not shutdown properly,
 *         server does not set SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_SSL
 * Test 1: client does not shutdown properly,
 *         server sets SSL_OP_IGNORE_UNEXPECTED_EOF,
 *         server should get SSL_ERROR_ZERO_RETURN
 */
static int test_incorrect_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char buf[80];
    BIO *c2s;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), 0, 0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 1)
        SSL_CTX_set_options(sctx, SSL_OP_IGNORE_UNEXPECTED_EOF);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                            NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE)))
        goto end;

    c2s = SSL_get_rbio(serverssl);
    BIO_set_mem_eof_return(c2s, 0);

    if (!TEST_false(SSL_read(serverssl, buf, sizeof(buf))))
        goto end;

    if (tst == 0 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_SSL) )
        goto end;
    if (tst == 1 && !TEST_int_eq(SSL_get_error(serverssl, 0), SSL_ERROR_ZERO_RETURN) )
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

/*
 * Test bi-directional shutdown.
 * Test 0: TLSv1.2
 * Test 1: TLSv1.2, server continues to read/write after client shutdown
 * Test 2: TLSv1.3, no pending NewSessionTicket messages
 * Test 3: TLSv1.3, pending NewSessionTicket messages
 * Test 4: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends key update, client reads it
 * Test 5: TLSv1.3, server continues to read/write after client shutdown, server
 *                  sends CertificateRequest, client reads and ignores it
 * Test 6: TLSv1.3, server continues to read/write after client shutdown, client
 *                  doesn't read it
 */
static int test_shutdown(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    char msg[] = "A test message";
    char buf[80];
    size_t written, readbytes;
    SSL_SESSION *sess;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 1)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 2)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 1) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (tst == 5)
        SSL_CTX_set_post_handshake_auth(cctx, 1);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst == 3) {
        if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                  SSL_ERROR_NONE, 1))
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_false(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else if (!TEST_true(create_ssl_connection(serverssl, clientssl,
                                              SSL_ERROR_NONE))
            || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))) {
        goto end;
    }

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0))
        goto end;

    if (tst >= 4) {
        /*
         * Reading on the server after the client has sent close_notify should
         * fail and provide SSL_ERROR_ZERO_RETURN
         */
        if (!TEST_false(SSL_read_ex(serverssl, buf, sizeof(buf), &readbytes))
                || !TEST_int_eq(SSL_get_error(serverssl, 0),
                                SSL_ERROR_ZERO_RETURN)
                || !TEST_int_eq(SSL_get_shutdown(serverssl),
                                SSL_RECEIVED_SHUTDOWN)
                   /*
                    * Even though we're shutdown on receive we should still be
                    * able to write.
                    */
                || !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (tst == 4
                && !TEST_true(SSL_key_update(serverssl,
                                             SSL_KEY_UPDATE_REQUESTED)))
            goto end;
        if (tst == 5) {
            SSL_set_verify(serverssl, SSL_VERIFY_PEER, NULL);
            if (!TEST_true(SSL_verify_client_post_handshake(serverssl)))
                goto end;
        }
        if ((tst == 4 || tst == 5)
                && !TEST_true(SSL_write(serverssl, msg, sizeof(msg))))
            goto end;
        if (!TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
        if (tst == 4 || tst == 5) {
            /* Should still be able to read data from server */
            if (!TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                       &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0)
                    || !TEST_true(SSL_read_ex(clientssl, buf, sizeof(buf),
                                              &readbytes))
                    || !TEST_size_t_eq(readbytes, sizeof(msg))
                    || !TEST_int_eq(memcmp(msg, buf, readbytes), 0))
                goto end;
        }
    }

    /* Writing on the client after sending close_notify shouldn't be possible */
    if (!TEST_false(SSL_write_ex(clientssl, msg, sizeof(msg), &written)))
        goto end;

    if (tst < 4) {
        /*
         * For these tests the client has sent close_notify but it has not yet
         * been received by the server. The server has not sent close_notify
         * yet.
         */
        if (!TEST_int_eq(SSL_shutdown(serverssl), 0)
                   /*
                    * Writing on the server after sending close_notify shouldn't
                    * be possible.
                    */
                || !TEST_false(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
                || !TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess))
                || !TEST_int_eq(SSL_shutdown(serverssl), 1))
            goto end;
    } else if (tst == 4 || tst == 5) {
        /*
         * In this test the client has sent close_notify and it has been
         * received by the server which has responded with a close_notify. The
         * client needs to read the close_notify sent by the server.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), 1)
                || !TEST_ptr_ne(sess = SSL_get_session(clientssl), NULL)
                || !TEST_true(SSL_SESSION_is_resumable(sess)))
            goto end;
    } else {
        /*
         * tst == 6
         *
         * The client has sent close_notify and is expecting a close_notify
         * back, but instead there is application data first. The shutdown
         * should fail with a fatal error.
         */
        if (!TEST_int_eq(SSL_shutdown(clientssl), -1)
                || !TEST_int_eq(SSL_get_error(clientssl, -1), SSL_ERROR_SSL))
            goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
static int cert_cb_cnt;

static int cert_cb(SSL *s, void *arg)
{
    SSL_CTX *ctx = (SSL_CTX *)arg;
    BIO *in = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *x509 = NULL, *rootx = NULL;
    STACK_OF(X509) *chain = NULL;
    char *rootfile = NULL, *ecdsacert = NULL, *ecdsakey = NULL;
    int ret = 0;

    if (cert_cb_cnt == 0) {
        /* Suspend the handshake */
        cert_cb_cnt++;
        return -1;
    } else if (cert_cb_cnt == 1) {
        /*
         * Update the SSL_CTX, set the certificate and private key and then
         * continue the handshake normally.
         */
        if (ctx != NULL && !TEST_ptr(SSL_set_SSL_CTX(s, ctx)))
            return 0;

        if (!TEST_true(SSL_use_certificate_file(s, cert, SSL_FILETYPE_PEM))
                || !TEST_true(SSL_use_PrivateKey_file(s, privkey,
                                                      SSL_FILETYPE_PEM))
                || !TEST_true(SSL_check_private_key(s)))
            return 0;
        cert_cb_cnt++;
        return 1;
    } else if (cert_cb_cnt == 3) {
        int rv;

        rootfile = test_mk_file_path(certsdir, "rootcert.pem");
        ecdsacert = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
        ecdsakey = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
        if (!TEST_ptr(rootfile) || !TEST_ptr(ecdsacert) || !TEST_ptr(ecdsakey))
            goto out;
        chain = sk_X509_new_null();
        if (!TEST_ptr(chain))
            goto out;
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, rootfile), 0)
                || !TEST_ptr(rootx = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &rootx, NULL, NULL))
                || !TEST_true(sk_X509_push(chain, rootx)))
            goto out;
        rootx = NULL;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsacert), 0)
                || !TEST_ptr(x509 = X509_new_ex(libctx, NULL))
                || !TEST_ptr(PEM_read_bio_X509(in, &x509, NULL, NULL)))
            goto out;
        BIO_free(in);
        if (!TEST_ptr(in = BIO_new(BIO_s_file()))
                || !TEST_int_gt(BIO_read_filename(in, ecdsakey), 0)
                || !TEST_ptr(pkey = PEM_read_bio_PrivateKey_ex(in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
            goto out;
        rv = SSL_check_chain(s, x509, pkey, chain);
        /*
         * If the cert doesn't show as valid here (e.g., because we don't
         * have any shared sigalgs), then we will not set it, and there will
         * be no certificate at all on the SSL or SSL_CTX.  This, in turn,
         * will cause tls_choose_sigalgs() to fail the connection.
         */
        if ((rv & (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE))
                == (CERT_PKEY_VALID | CERT_PKEY_CA_SIGNATURE)) {
            if (!SSL_use_cert_and_key(s, x509, pkey, NULL, 1))
                goto out;
        }

        ret = 1;
    }

    /* Abort the handshake */
 out:
    OPENSSL_free(ecdsacert);
    OPENSSL_free(ecdsakey);
    OPENSSL_free(rootfile);
    BIO_free(in);
    EVP_PKEY_free(pkey);
    X509_free(x509);
    X509_free(rootx);
    sk_X509_pop_free(chain, X509_free);
    return ret;
}

/*
 * Test the certificate callback.
 * Test 0: Callback fails
 * Test 1: Success - no SSL_set_SSL_CTX() in the callback
 * Test 2: Success - SSL_set_SSL_CTX() in the callback
 * Test 3: Success - Call SSL_check_chain from the callback
 * Test 4: Failure - SSL_check_chain fails from callback due to bad cert in the
 *                   chain
 * Test 5: Failure - SSL_check_chain fails from callback due to bad ee cert
 */
static int test_cert_cb_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *snictx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0, ret;

#ifdef OPENSSL_NO_EC
    /* We use an EC cert in these tests, so we skip in a no-ec build */
    if (tst >= 3)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, NULL, NULL)))
        goto end;

    if (tst == 0)
        cert_cb_cnt = -1;
    else if (tst >= 3)
        cert_cb_cnt = 3;
    else
        cert_cb_cnt = 0;

    if (tst == 2) {
        snictx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
        if (!TEST_ptr(snictx))
            goto end;
    }

    SSL_CTX_set_cert_cb(sctx, cert_cb, snictx);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (tst == 4) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the chain doesn't meet (the root uses an RSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                                             "ecdsa_secp256r1_sha256")))
            goto end;
    } else if (tst == 5) {
        /*
         * We cause SSL_check_chain() to fail by specifying sig_algs that
         * the ee cert doesn't meet (the ee uses an ECDSA cert)
         */
        if (!TEST_true(SSL_set1_sigalgs_list(clientssl,
                           "rsa_pss_rsae_sha256:rsa_pkcs1_sha256")))
            goto end;
    }

    ret = create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE);
    if (!TEST_true(tst == 0 || tst == 4 || tst == 5 ? !ret : ret)
            || (tst > 0
                && !TEST_int_eq((cert_cb_cnt - 2) * (cert_cb_cnt - 3), 0))) {
        goto end;
    }

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    SSL_CTX_free(snictx);

    return testresult;
}
#endif

static int test_cert_cb(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_cert_cb_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_cert_cb_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

static int client_cert_cb(SSL *ssl, X509 **x509, EVP_PKEY **pkey)
{
    X509 *xcert;
    EVP_PKEY *privpkey;
    BIO *in = NULL;
    BIO *priv_in = NULL;

    /* Check that SSL_get0_peer_certificate() returns something sensible */
    if (!TEST_ptr(SSL_get0_peer_certificate(ssl)))
        return 0;

    in = BIO_new_file(cert, "r");
    if (!TEST_ptr(in))
        return 0;

    if (!TEST_ptr(xcert = X509_new_ex(libctx, NULL))
            || !TEST_ptr(PEM_read_bio_X509(in, &xcert, NULL, NULL))
            || !TEST_ptr(priv_in = BIO_new_file(privkey, "r"))
            || !TEST_ptr(privpkey = PEM_read_bio_PrivateKey_ex(priv_in, NULL,
                                                               NULL, NULL,
                                                               libctx, NULL)))
        goto err;

    *x509 = xcert;
    *pkey = privpkey;

    BIO_free(in);
    BIO_free(priv_in);
    return 1;
err:
    X509_free(xcert);
    BIO_free(in);
    BIO_free(priv_in);
    return 0;
}

static int test_client_cert_cb(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

#ifdef OPENSSL_NO_TLS1_2
    if (tst == 0)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst == 1)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       tst == 0 ? TLS1_2_VERSION
                                                : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    /*
     * Test that setting a client_cert_cb results in a client certificate being
     * sent.
     */
    SSL_CTX_set_client_cert_cb(cctx, client_cert_cb);
    SSL_CTX_set_verify(sctx,
                       SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT,
                       verify_cb);

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
/*
 * Test setting certificate authorities on both client and server.
 *
 * Test 0: SSL_CTX_set0_CA_list() only
 * Test 1: Both SSL_CTX_set0_CA_list() and SSL_CTX_set_client_CA_list()
 * Test 2: Only SSL_CTX_set_client_CA_list()
 */
static int test_ca_names_int(int prot, int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    size_t i;
    X509_NAME *name[] = { NULL, NULL, NULL, NULL };
    char *strnames[] = { "Jack", "Jill", "John", "Joanne" };
    STACK_OF(X509_NAME) *sk1 = NULL, *sk2 = NULL;
    const STACK_OF(X509_NAME) *sktmp = NULL;

    for (i = 0; i < OSSL_NELEM(name); i++) {
        name[i] = X509_NAME_new();
        if (!TEST_ptr(name[i])
                || !TEST_true(X509_NAME_add_entry_by_txt(name[i], "CN",
                                                         MBSTRING_ASC,
                                                         (unsigned char *)
                                                         strnames[i],
                                                         -1, -1, 0)))
            goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       prot,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    SSL_CTX_set_verify(sctx, SSL_VERIFY_PEER, NULL);

    if (tst == 0 || tst == 1) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[1])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[0])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[1]))))
            goto end;

        SSL_CTX_set0_CA_list(sctx, sk1);
        SSL_CTX_set0_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }
    if (tst == 1 || tst == 2) {
        if (!TEST_ptr(sk1 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk1, X509_NAME_dup(name[3])))
                || !TEST_ptr(sk2 = sk_X509_NAME_new_null())
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[2])))
                || !TEST_true(sk_X509_NAME_push(sk2, X509_NAME_dup(name[3]))))
            goto end;

        SSL_CTX_set_client_CA_list(sctx, sk1);
        SSL_CTX_set_client_CA_list(cctx, sk2);
        sk1 = sk2 = NULL;
    }

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /*
     * We only expect certificate authorities to have been sent to the server
     * if we are using TLSv1.3 and SSL_set0_CA_list() was used
     */
    sktmp = SSL_get0_peer_CA_list(serverssl);
    if (prot == TLS1_3_VERSION
            && (tst == 0 || tst == 1)) {
        if (!TEST_ptr(sktmp)
                || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                              name[0]), 0)
                || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                              name[1]), 0))
            goto end;
    } else if (!TEST_ptr_null(sktmp)) {
        goto end;
    }

    /*
     * In all tests we expect certificate authorities to have been sent to the
     * client. However, SSL_set_client_CA_list() should override
     * SSL_set0_CA_list()
     */
    sktmp = SSL_get0_peer_CA_list(clientssl);
    if (!TEST_ptr(sktmp)
            || !TEST_int_eq(sk_X509_NAME_num(sktmp), 2)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 0),
                                          name[tst == 0 ? 0 : 2]), 0)
            || !TEST_int_eq(X509_NAME_cmp(sk_X509_NAME_value(sktmp, 1),
                                          name[tst == 0 ? 1 : 3]), 0))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    for (i = 0; i < OSSL_NELEM(name); i++)
        X509_NAME_free(name[i]);
    sk_X509_NAME_pop_free(sk1, X509_NAME_free);
    sk_X509_NAME_pop_free(sk2, X509_NAME_free);

    return testresult;
}
#endif

static int test_ca_names(int tst)
{
    int testresult = 1;

#ifndef OPENSSL_NO_TLS1_2
    testresult &= test_ca_names_int(TLS1_2_VERSION, tst);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    testresult &= test_ca_names_int(TLS1_3_VERSION, tst);
#endif

    return testresult;
}

#ifndef OPENSSL_NO_TLS1_2
static const char *multiblock_cipherlist_data[]=
{
    "AES128-SHA",
    "AES128-SHA256",
    "AES256-SHA",
    "AES256-SHA256",
};

/* Reduce the fragment size - so the multiblock test buffer can be small */
# define MULTIBLOCK_FRAGSIZE 512

static int test_multiblock_write(int test_index)
{
    static const char *fetchable_ciphers[]=
    {
        "AES-128-CBC-HMAC-SHA1",
        "AES-128-CBC-HMAC-SHA256",
        "AES-256-CBC-HMAC-SHA1",
        "AES-256-CBC-HMAC-SHA256"
    };
    const char *cipherlist = multiblock_cipherlist_data[test_index];
    const SSL_METHOD *smeth = TLS_server_method();
    const SSL_METHOD *cmeth = TLS_client_method();
    int min_version = TLS1_VERSION;
    int max_version = TLS1_2_VERSION; /* Don't select TLS1_3 */
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /*
     * Choose a buffer large enough to perform a multi-block operation
     * i.e: write_len >= 4 * frag_size
     * 9 * is chosen so that multiple multiblocks are used + some leftover.
     */
    unsigned char msg[MULTIBLOCK_FRAGSIZE * 9];
    unsigned char buf[sizeof(msg)], *p = buf;
    size_t readbytes, written, len;
    EVP_CIPHER *ciph = NULL;

    /*
     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
    EVP_CIPHER_free(ciph);

    /* Set up a buffer with some data that will be sent to the client */
    RAND_bytes(msg, sizeof(msg));

    if (!TEST_true(create_ssl_ctx_pair(libctx, smeth, cmeth, min_version,
                                       max_version, &sctx, &cctx, cert,
                                       privkey)))
        goto end;

    if (!TEST_true(SSL_CTX_set_max_send_fragment(sctx, MULTIBLOCK_FRAGSIZE)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
            goto end;

    /* settings to force it to use AES-CBC-HMAC_SHA */
    SSL_set_options(serverssl, SSL_OP_NO_ENCRYPT_THEN_MAC);
    if (!TEST_true(SSL_CTX_set_cipher_list(cctx, cipherlist)))
       goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_write_ex(serverssl, msg, sizeof(msg), &written))
        || !TEST_size_t_eq(written, sizeof(msg)))
        goto end;

    len = written;
    while (len > 0) {
        if (!TEST_true(SSL_read_ex(clientssl, p, MULTIBLOCK_FRAGSIZE, &readbytes)))
            goto end;
        p += readbytes;
        len -= readbytes;
    }
    if (!TEST_mem_eq(msg, sizeof(msg), buf, sizeof(buf)))
        goto end;

    testresult = 1;
end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}
#endif /* OPENSSL_NO_TLS1_2 */

static int test_session_timeout(int test)
{
    /*
     * Test session ordering and timeout
     * Can't explicitly test performance of the new code,
     * but can test to see if the ordering of the sessions
     * are correct, and they they are removed as expected
     */
    SSL_SESSION *early = NULL;
    SSL_SESSION *middle = NULL;
    SSL_SESSION *late = NULL;
    SSL_CTX *ctx;
    int testresult = 0;
    long now = (long)time(NULL);
#define TIMEOUT 10

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_method()))
        || !TEST_ptr(early = SSL_SESSION_new())
        || !TEST_ptr(middle = SSL_SESSION_new())
        || !TEST_ptr(late = SSL_SESSION_new()))
        goto end;

    /* assign unique session ids */
    early->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(early->session_id, 1, SSL3_SSL_SESSION_ID_LENGTH);
    middle->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(middle->session_id, 2, SSL3_SSL_SESSION_ID_LENGTH);
    late->session_id_length = SSL3_SSL_SESSION_ID_LENGTH;
    memset(late->session_id, 3, SSL3_SSL_SESSION_ID_LENGTH);

    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_time(early, now - 10), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(middle, now), 0)
        || !TEST_int_ne(SSL_SESSION_set_time(late, now + 10), 0))
        goto end;

    if (!TEST_int_ne(SSL_SESSION_set_timeout(early, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(middle, TIMEOUT), 0)
        || !TEST_int_ne(SSL_SESSION_set_timeout(late, TIMEOUT), 0))
        goto end;

    /* Make sure they are all still there */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* Make sure they are in the expected order */
    if (!TEST_ptr_eq(late->next, middle)
        || !TEST_ptr_eq(middle->next, early)
        || !TEST_ptr_eq(early->prev, middle)
        || !TEST_ptr_eq(middle->prev, late))
        goto end;

    /* This should remove "early" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT - 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "middle" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 1);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove "late" */
    SSL_CTX_flush_sessions(ctx, now + TIMEOUT + 11);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    /* Add them back in again */
    if (!TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, middle), 1)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, late), 1))
        goto end;

    /* Make sure they are all added */
    if (!TEST_ptr(early->prev)
        || !TEST_ptr(middle->prev)
        || !TEST_ptr(late->prev))
        goto end;

    /* This should remove all of them */
    SSL_CTX_flush_sessions(ctx, 0);
    if (!TEST_ptr_null(early->prev)
        || !TEST_ptr_null(middle->prev)
        || !TEST_ptr_null(late->prev))
        goto end;

    (void)SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_UPDATE_TIME
                                         | SSL_CTX_get_session_cache_mode(ctx));

    /* make sure |now| is NOT  equal to the current time */
    now -= 10;
    if (!TEST_int_ne(SSL_SESSION_set_time(early, now), 0)
        || !TEST_int_eq(SSL_CTX_add_session(ctx, early), 1)
        || !TEST_long_ne(SSL_SESSION_get_time(early), now))
        goto end;

    testresult = 1;
 end:
    SSL_CTX_free(ctx);
    SSL_SESSION_free(early);
    SSL_SESSION_free(middle);
    SSL_SESSION_free(late);
    return testresult;
}

/*
 * Test 0: Client sets servername and server acknowledges it (TLSv1.2)
 * Test 1: Client sets servername and server does not acknowledge it (TLSv1.2)
 * Test 2: Client sets inconsistent servername on resumption (TLSv1.2)
 * Test 3: Client does not set servername on initial handshake (TLSv1.2)
 * Test 4: Client does not set servername on resumption handshake (TLSv1.2)
 * Test 5: Client sets servername and server acknowledges it (TLSv1.3)
 * Test 6: Client sets servername and server does not acknowledge it (TLSv1.3)
 * Test 7: Client sets inconsistent servername on resumption (TLSv1.3)
 * Test 8: Client does not set servername on initial handshake(TLSv1.3)
 * Test 9: Client does not set servername on resumption handshake (TLSv1.3)
 */
static int test_servername(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;
    const char *sexpectedhost = NULL, *cexpectedhost = NULL;

#ifdef OPENSSL_NO_TLS1_2
    if (tst <= 4)
        return 1;
#endif
#ifdef OSSL_NO_USABLE_TLS1_3
    if (tst >= 5)
        return 1;
#endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION,
                                       (tst <= 4) ? TLS1_2_VERSION
                                                  : TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (tst != 1 && tst != 6) {
        if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx,
                                                              hostname_cb)))
            goto end;
    }

    if (tst != 3 && tst != 8) {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        sexpectedhost = cexpectedhost = "goodhost";
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(SSL_get_servername(clientssl, TLSEXT_NAMETYPE_host_name),
                     cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    /* Now repeat with a resumption handshake */

    if (!TEST_int_eq(SSL_shutdown(clientssl), 0)
            || !TEST_ptr_ne(sess = SSL_get1_session(clientssl), NULL)
            || !TEST_true(SSL_SESSION_is_resumable(sess))
            || !TEST_int_eq(SSL_shutdown(serverssl), 0))
        goto end;

    SSL_free(clientssl);
    SSL_free(serverssl);
    clientssl = serverssl = NULL;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl, NULL,
                                      NULL)))
        goto end;

    if (!TEST_true(SSL_set_session(clientssl, sess)))
        goto end;

    sexpectedhost = cexpectedhost = "goodhost";
    if (tst == 2 || tst == 7) {
        /* Set an inconsistent hostname */
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "altgoodhost")))
            goto end;
        /*
         * In TLSv1.2 we expect the hostname from the original handshake, in
         * TLSv1.3 we expect the hostname from this handshake
         */
        if (tst == 7)
            sexpectedhost = cexpectedhost = "altgoodhost";

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "altgoodhost"))
            goto end;
    } else if (tst == 4 || tst == 9) {
        /*
         * A TLSv1.3 session does not associate a session with a servername,
         * but a TLSv1.2 session does.
         */
        if (tst == 9)
            sexpectedhost = cexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         cexpectedhost))
            goto end;
    } else {
        if (!TEST_true(SSL_set_tlsext_host_name(clientssl, "goodhost")))
            goto end;
        /*
         * In a TLSv1.2 resumption where the hostname was not acknowledged
         * we expect the hostname on the server to be empty. On the client we
         * return what was requested in this case.
         *
         * Similarly if the client didn't set a hostname on an original TLSv1.2
         * session but is now, the server hostname will be empty, but the client
         * is as we set it.
         */
        if (tst == 1 || tst == 3)
            sexpectedhost = NULL;

        if (!TEST_str_eq(SSL_get_servername(clientssl,
                                            TLSEXT_NAMETYPE_host_name),
                         "goodhost"))
            goto end;
    }

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_true(SSL_session_reused(clientssl))
            || !TEST_true(SSL_session_reused(serverssl))
            || !TEST_str_eq(SSL_get_servername(clientssl,
                                               TLSEXT_NAMETYPE_host_name),
                            cexpectedhost)
            || !TEST_str_eq(SSL_get_servername(serverssl,
                                               TLSEXT_NAMETYPE_host_name),
                            sexpectedhost))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
/*
 * Test that if signature algorithms are not available, then we do not offer or
 * accept them.
 * Test 0: Two RSA sig algs available: both RSA sig algs shared
 * Test 1: The client only has SHA2-256: only SHA2-256 algorithms shared
 * Test 2: The server only has SHA2-256: only SHA2-256 algorithms shared
 * Test 3: An RSA and an ECDSA sig alg available: both sig algs shared
 * Test 4: The client only has an ECDSA sig alg: only ECDSA algorithms shared
 * Test 5: The server only has an ECDSA sig alg: only ECDSA algorithms shared
 */
static int test_sigalgs_available(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_LIB_CTX *tmpctx = OSSL_LIB_CTX_new();
    OSSL_LIB_CTX *clientctx = libctx, *serverctx = libctx;
    OSSL_PROVIDER *filterprov = NULL;
    int sig, hash;

    if (!TEST_ptr(tmpctx))
        goto end;

    if (idx != 0 && idx != 3) {
        if (!TEST_true(OSSL_PROVIDER_add_builtin(tmpctx, "filter",
                                                 filter_provider_init)))
            goto end;

        filterprov = OSSL_PROVIDER_load(tmpctx, "filter");
        if (!TEST_ptr(filterprov))
            goto end;

        if (idx < 3) {
            /*
             * Only enable SHA2-256 so rsa_pss_rsae_sha384 should not be offered
             * or accepted for the peer that uses this libctx. Note that libssl
             * *requires* SHA2-256 to be available so we cannot disable that. We
             * also need SHA1 for our certificate.
             */
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_DIGEST,
                                                      "SHA2-256:SHA1")))
                goto end;
        } else {
            if (!TEST_true(filter_provider_set_filter(OSSL_OP_SIGNATURE,
                                                      "ECDSA"))
                    || !TEST_true(filter_provider_set_filter(OSSL_OP_KEYMGMT,
                                                             "EC:X25519:X448")))
                goto end;
        }

        if (idx == 1 || idx == 4)
            clientctx = tmpctx;
        else
            serverctx = tmpctx;
    }

    cctx = SSL_CTX_new_ex(clientctx, NULL, TLS_client_method());
    sctx = SSL_CTX_new_ex(serverctx, NULL, TLS_server_method());
    if (!TEST_ptr(cctx) || !TEST_ptr(sctx))
        goto end;

    if (idx != 5) {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert, privkey)))
            goto end;
    } else {
        if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                           TLS_client_method(),
                                           TLS1_VERSION,
                                           0,
                                           &sctx, &cctx, cert2, privkey2)))
            goto end;
    }

    /* Ensure we only use TLSv1.2 ciphersuites based on SHA256 */
    if (idx < 4) {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-RSA-AES128-GCM-SHA256")))
            goto end;
    } else {
        if (!TEST_true(SSL_CTX_set_cipher_list(cctx,
                                               "ECDHE-ECDSA-AES128-GCM-SHA256")))
            goto end;
    }

    if (idx < 3) {
        if (!SSL_CTX_set1_sigalgs_list(cctx,
                                       "rsa_pss_rsae_sha384"
                                       ":rsa_pss_rsae_sha256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha384"
                                              ":rsa_pss_rsae_sha256"))
            goto end;
    } else {
        if (!SSL_CTX_set1_sigalgs_list(cctx, "rsa_pss_rsae_sha256:ECDSA+SHA256")
                || !SSL_CTX_set1_sigalgs_list(sctx,
                                              "rsa_pss_rsae_sha256:ECDSA+SHA256"))
            goto end;
    }

    if (idx != 5
        && (!TEST_int_eq(SSL_CTX_use_certificate_file(sctx, cert2,
                                                      SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_use_PrivateKey_file(sctx,
                                                        privkey2,
                                                        SSL_FILETYPE_PEM), 1)
            || !TEST_int_eq(SSL_CTX_check_private_key(sctx), 1)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    /* For tests 0 and 3 we expect 2 shared sigalgs, otherwise exactly 1 */
    if (!TEST_int_eq(SSL_get_shared_sigalgs(serverssl, 0, &sig, &hash, NULL,
                                            NULL, NULL),
                     (idx == 0 || idx == 3) ? 2 : 1))
        goto end;

    if (!TEST_int_eq(hash, idx == 0 ? NID_sha384 : NID_sha256))
        goto end;

    if (!TEST_int_eq(sig, (idx == 4 || idx == 5) ? EVP_PKEY_EC
                                                 : NID_rsassaPss))
        goto end;

    testresult = filter_provider_check_clean_finish();

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(filterprov);
    OSSL_LIB_CTX_free(tmpctx);

    return testresult;
}
#endif /*
        * !defined(OPENSSL_NO_EC) \
        * && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
        */

#ifndef OPENSSL_NO_TLS1_3
/* This test can run in TLSv1.3 even if ec and dh are disabled */
static int test_pluggable_group(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    OSSL_PROVIDER *tlsprov = OSSL_PROVIDER_load(libctx, "tls-provider");
    /* Check that we are not impacted by a provider without any groups */
    OSSL_PROVIDER *legacyprov = OSSL_PROVIDER_load(libctx, "legacy");
    const char *group_name = idx == 0 ? "xorgroup" : "xorkemgroup";

    if (!TEST_ptr(tlsprov))
        goto end;

    if (legacyprov == NULL) {
        /*
         * In this case we assume we've been built with "no-legacy" and skip
         * this test (there is no OPENSSL_NO_LEGACY)
         */
        testresult = 1;
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION,
                                       TLS1_3_VERSION,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set1_groups_list(serverssl, group_name))
            || !TEST_true(SSL_set1_groups_list(clientssl, group_name)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (!TEST_str_eq(group_name,
                     SSL_group_to_name(serverssl, SSL_get_shared_group(serverssl, 0))))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    OSSL_PROVIDER_unload(tlsprov);
    OSSL_PROVIDER_unload(legacyprov);

    return testresult;
}
#endif

#ifndef OPENSSL_NO_TLS1_2
static int test_ssl_dup(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL, *client2ssl = NULL;
    int testresult = 0;
    BIO *rbio = NULL, *wbio = NULL;

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                             NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_min_proto_version(clientssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(clientssl, TLS1_2_VERSION)))
        goto end;

    client2ssl = SSL_dup(clientssl);
    rbio = SSL_get_rbio(clientssl);
    if (!TEST_ptr(rbio)
            || !TEST_true(BIO_up_ref(rbio)))
        goto end;
    SSL_set0_rbio(client2ssl, rbio);
    rbio = NULL;

    wbio = SSL_get_wbio(clientssl);
    if (!TEST_ptr(wbio) || !TEST_true(BIO_up_ref(wbio)))
        goto end;
    SSL_set0_wbio(client2ssl, wbio);
    rbio = NULL;

    if (!TEST_ptr(client2ssl)
               /* Handshake not started so pointers should be different */
            || !TEST_ptr_ne(clientssl, client2ssl))
        goto end;

    if (!TEST_int_eq(SSL_get_min_proto_version(client2ssl), TLS1_2_VERSION)
            || !TEST_int_eq(SSL_get_max_proto_version(client2ssl), TLS1_2_VERSION))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, client2ssl, SSL_ERROR_NONE)))
        goto end;

    SSL_free(clientssl);
    clientssl = SSL_dup(client2ssl);
    if (!TEST_ptr(clientssl)
               /* Handshake has finished so pointers should be the same */
            || !TEST_ptr_eq(clientssl, client2ssl))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_free(client2ssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

# ifndef OPENSSL_NO_DH

static EVP_PKEY *tmp_dh_params = NULL;

/* Helper function for the test_set_tmp_dh() tests */
static EVP_PKEY *get_tmp_dh_params(void)
{
    if (tmp_dh_params == NULL) {
        BIGNUM *p = NULL;
        OSSL_PARAM_BLD *tmpl = NULL;
        EVP_PKEY_CTX *pctx = NULL;
        OSSL_PARAM *params = NULL;
        EVP_PKEY *dhpkey = NULL;

        p = BN_get_rfc3526_prime_2048(NULL);
        if (!TEST_ptr(p))
            goto end;

        pctx = EVP_PKEY_CTX_new_from_name(libctx, "DH", NULL);
        if (!TEST_ptr(pctx)
                || !TEST_int_eq(EVP_PKEY_fromdata_init(pctx), 1))
            goto end;

        tmpl = OSSL_PARAM_BLD_new();
        if (!TEST_ptr(tmpl)
                || !TEST_true(OSSL_PARAM_BLD_push_BN(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_P,
                                                        p))
                || !TEST_true(OSSL_PARAM_BLD_push_uint(tmpl,
                                                        OSSL_PKEY_PARAM_FFC_G,
                                                        2)))
            goto end;

        params = OSSL_PARAM_BLD_to_param(tmpl);
        if (!TEST_ptr(params)
                || !TEST_int_eq(EVP_PKEY_fromdata(pctx, &dhpkey,
                                                  EVP_PKEY_KEY_PARAMETERS,
                                                  params), 1))
            goto end;

        tmp_dh_params = dhpkey;
    end:
        BN_free(p);
        EVP_PKEY_CTX_free(pctx);
        OSSL_PARAM_BLD_free(tmpl);
        OSSL_PARAM_free(params);
    }

    if (tmp_dh_params != NULL && !EVP_PKEY_up_ref(tmp_dh_params))
        return NULL;

    return tmp_dh_params;
}

#  ifndef OPENSSL_NO_DEPRECATED_3_0
/* Callback used by test_set_tmp_dh() */
static DH *tmp_dh_callback(SSL *s, int is_export, int keylen)
{
    EVP_PKEY *dhpkey = get_tmp_dh_params();
    DH *ret = NULL;

    if (!TEST_ptr(dhpkey))
        return NULL;

    /*
     * libssl does not free the returned DH, so we free it now knowing that even
     * after we free dhpkey, there will still be a reference to the owning
     * EVP_PKEY in tmp_dh_params, and so the DH object will live for the length
     * of time we need it for.
     */
    ret = EVP_PKEY_get1_DH(dhpkey);
    DH_free(ret);

    EVP_PKEY_free(dhpkey);

    return ret;
}
#  endif

/*
 * Test the various methods for setting temporary DH parameters
 *
 * Test  0: Default (no auto) setting
 * Test  1: Explicit SSL_CTX auto off
 * Test  2: Explicit SSL auto off
 * Test  3: Explicit SSL_CTX auto on
 * Test  4: Explicit SSL auto on
 * Test  5: Explicit SSL_CTX auto off, custom DH params via EVP_PKEY
 * Test  6: Explicit SSL auto off, custom DH params via EVP_PKEY
 *
 * The following are testing deprecated APIs, so we only run them if available
 * Test  7: Explicit SSL_CTX auto off, custom DH params via DH
 * Test  8: Explicit SSL auto off, custom DH params via DH
 * Test  9: Explicit SSL_CTX auto off, custom DH params via callback
 * Test 10: Explicit SSL auto off, custom DH params via callback
 */
static int test_set_tmp_dh(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int dhauto = (idx == 3 || idx == 4) ? 1 : 0;
    int expected = (idx <= 2) ? 0 : 1;
    EVP_PKEY *dhpkey = NULL;
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH *dh = NULL;
#  else

    if (idx >= 7)
        return 1;
#  endif

    if (idx >= 5 && idx <= 8) {
        dhpkey = get_tmp_dh_params();
        if (!TEST_ptr(dhpkey))
            goto end;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    if (idx == 7 || idx == 8) {
        dh = EVP_PKEY_get1_DH(dhpkey);
        if (!TEST_ptr(dh))
            goto end;
    }
#  endif

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if ((idx & 1) == 1) {
        if (!TEST_true(SSL_CTX_set_dh_auto(sctx, dhauto)))
            goto end;
    }

    if (idx == 5) {
        if (!TEST_true(SSL_CTX_set0_tmp_dh_pkey(sctx, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 7) {
        if (!TEST_true(SSL_CTX_set_tmp_dh(sctx, dh)))
            goto end;
    } else if (idx == 9) {
        SSL_CTX_set_tmp_dh_callback(sctx, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if ((idx & 1) == 0 && idx != 0) {
        if (!TEST_true(SSL_set_dh_auto(serverssl, dhauto)))
            goto end;
    }
    if (idx == 6) {
        if (!TEST_true(SSL_set0_tmp_dh_pkey(serverssl, dhpkey)))
            goto end;
        dhpkey = NULL;
    }
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    else if (idx == 8) {
        if (!TEST_true(SSL_set_tmp_dh(serverssl, dh)))
            goto end;
    } else if (idx == 10) {
        SSL_set_tmp_dh_callback(serverssl, tmp_dh_callback);
    }
#  endif

    if (!TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, "DHE-RSA-AES128-SHA")))
        goto end;

    /*
     * If autoon then we should succeed. Otherwise we expect failure because
     * there are no parameters
     */
    if (!TEST_int_eq(create_ssl_connection(serverssl, clientssl,
                                           SSL_ERROR_NONE), expected))
        goto end;

    testresult = 1;

 end:
#  ifndef OPENSSL_NO_DEPRECATED_3_0
    DH_free(dh);
#  endif
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(dhpkey);

    return testresult;
}

/*
 * Test the auto DH keys are appropriately sized
 */
static int test_dh_auto(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    EVP_PKEY *tmpkey = NULL;
    char *thiscert = NULL, *thiskey = NULL;
    size_t expdhsize = 0;
    const char *ciphersuite = "DHE-RSA-AES128-SHA";

    switch (idx) {
    case 0:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        thiscert = cert1024;
        thiskey = privkey1024;
        expdhsize = 1024;
        break;
    case 1:
        /* 2048 bit prime */
        thiscert = cert;
        thiskey = privkey;
        expdhsize = 2048;
        break;
    case 2:
        thiscert = cert3072;
        thiskey = privkey3072;
        expdhsize = 3072;
        break;
    case 3:
        thiscert = cert4096;
        thiskey = privkey4096;
        expdhsize = 4096;
        break;
    case 4:
        thiscert = cert8192;
        thiskey = privkey8192;
        expdhsize = 8192;
        break;
    /* No certificate cases */
    case 5:
        /* The FIPS provider doesn't support this DH size - so we ignore it */
        if (is_fips)
            return 1;
        ciphersuite = "ADH-AES128-SHA256:@SECLEVEL=0";
        expdhsize = 1024;
        break;
    case 6:
        ciphersuite = "ADH-AES256-SHA256:@SECLEVEL=0";
        expdhsize = 3072;
        break;
    default:
        TEST_error("Invalid text index");
        goto end;
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(),
                                       0,
                                       0,
                                       &sctx, &cctx, thiscert, thiskey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl, &clientssl,
                                      NULL, NULL)))
        goto end;

    if (!TEST_true(SSL_set_dh_auto(serverssl, 1))
            || !TEST_true(SSL_set_min_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_max_proto_version(serverssl, TLS1_2_VERSION))
            || !TEST_true(SSL_set_cipher_list(serverssl, ciphersuite))
            || !TEST_true(SSL_set_cipher_list(clientssl, ciphersuite)))
        goto end;

    /*
     * Send the server's first flight. At this point the server has created the
     * temporary DH key but hasn't finished using it yet. Once used it is
     * removed, so we cannot test it.
     */
    if (!TEST_int_le(SSL_connect(clientssl), 0)
            || !TEST_int_le(SSL_accept(serverssl), 0))
        goto end;

    if (!TEST_int_gt(SSL_get_tmp_key(serverssl, &tmpkey), 0))
        goto end;
    if (!TEST_size_t_eq(EVP_PKEY_get_bits(tmpkey), expdhsize))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    EVP_PKEY_free(tmpkey);

    return testresult;

}
# endif /* OPENSSL_NO_DH */
#endif /* OPENSSL_NO_TLS1_2 */

#ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Test that setting an SNI callback works with TLSv1.3. Specifically we check
 * that it works even without a certificate configured for the original
 * SSL_CTX
 */
static int test_sni_tls13(void)
{
    SSL_CTX *cctx = NULL, *sctx = NULL, *sctx2 = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /* Reset callback counter */
    snicb = 0;

    /* Create an initial SSL_CTX with no certificate configured */
    sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(sctx))
        goto end;
    /* Require TLSv1.3 as a minimum */
    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), TLS1_3_VERSION, 0,
                                       &sctx2, &cctx, cert, privkey)))
        goto end;

    /* Set up SNI */
    if (!TEST_true(SSL_CTX_set_tlsext_servername_callback(sctx, sni_cb))
            || !TEST_true(SSL_CTX_set_tlsext_servername_arg(sctx, sctx2)))
        goto end;

    /*
     * Connection should still succeed because the final SSL_CTX has the right
     * certificates configured.
     */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL))
            || !TEST_true(create_ssl_connection(serverssl, clientssl,
                                                SSL_ERROR_NONE)))
        goto end;

    /* We should have had the SNI callback called exactly once */
    if (!TEST_int_eq(snicb, 1))
        goto end;

    testresult = 1;

end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx2);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}

/*
 * Test that the lifetime hint of a TLSv1.3 ticket is no more than 1 week
 * 0 = TLSv1.2
 * 1 = TLSv1.3
 */
static int test_ticket_lifetime(int idx)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    int version = TLS1_3_VERSION;

#define ONE_WEEK_SEC (7 * 24 * 60 * 60)
#define TWO_WEEK_SEC (2 * ONE_WEEK_SEC)

    if (idx == 0) {
#ifdef OPENSSL_NO_TLS1_2
        return TEST_skip("TLS 1.2 is disabled.");
#else
        version = TLS1_2_VERSION;
#endif
    }

    if (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                       TLS_client_method(), version, version,
                                       &sctx, &cctx, cert, privkey)))
        goto end;

    if (!TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                      &clientssl, NULL, NULL)))
        goto end;

    /*
     * Set the timeout to be more than 1 week
     * make sure the returned value is the default
     */
    if (!TEST_long_eq(SSL_CTX_set_timeout(sctx, TWO_WEEK_SEC),
                      SSL_get_default_timeout(serverssl)))
        goto end;

    if (!TEST_true(create_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE)))
        goto end;

    if (idx == 0) {
        /* TLSv1.2 uses the set value */
        if (!TEST_ulong_eq(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), TWO_WEEK_SEC))
            goto end;
    } else {
        /* TLSv1.3 uses the limited value */
        if (!TEST_ulong_le(SSL_SESSION_get_ticket_lifetime_hint(SSL_get_session(clientssl)), ONE_WEEK_SEC))
            goto end;
    }
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
 * Test that setting an ALPN does not violate RFC
 */
static int test_set_alpn(void)
{
    SSL_CTX *ctx = NULL;
    SSL *ssl = NULL;
    int testresult = 0;

    unsigned char bad0[] = { 0x00, 'b', 'a', 'd' };
    unsigned char good[] = { 0x04, 'g', 'o', 'o', 'd' };
    unsigned char bad1[] = { 0x01, 'b', 'a', 'd' };
    unsigned char bad2[] = { 0x03, 'b', 'a', 'd', 0x00};
    unsigned char bad3[] = { 0x03, 'b', 'a', 'd', 0x01, 'b', 'a', 'd'};
    unsigned char bad4[] = { 0x03, 'b', 'a', 'd', 0x06, 'b', 'a', 'd'};

    /* Create an initial SSL_CTX with no certificate configured */
    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    /* the set_alpn functions return 0 (false) on success, non-zero (true) on failure */
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, 0)))
        goto end;
    if (!TEST_false(SSL_CTX_set_alpn_protos(ctx, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, good, 1)))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_CTX_set_alpn_protos(ctx, bad4, sizeof(bad4))))
        goto end;

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    if (!TEST_false(SSL_set_alpn_protos(ssl, NULL, 2)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, 0)))
        goto end;
    if (!TEST_false(SSL_set_alpn_protos(ssl, good, sizeof(good))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, good, 1)))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad0, sizeof(bad0))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad1, sizeof(bad1))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad2, sizeof(bad2))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad3, sizeof(bad3))))
        goto end;
    if (!TEST_true(SSL_set_alpn_protos(ssl, bad4, sizeof(bad4))))
        goto end;

    testresult = 1;

end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return testresult;
}

/*
 * Test SSL_CTX_set1_verify/chain_cert_store and SSL_CTX_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl_ctx(void)
{
   SSL_CTX *ctx = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, new_store)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_CTX_set1_verify_cert_store(ctx, NULL)))
       goto end;

   if (!TEST_true(SSL_CTX_set1_chain_cert_store(ctx, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_CTX_get0_verify_cert_store(ctx, &store)))
       goto end;

   if (!TEST_true(SSL_CTX_get0_chain_cert_store(ctx, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_CTX_free(ctx);
   return testresult;
}

/*
 * Test SSL_set1_verify/chain_cert_store and SSL_get_verify/chain_cert_store.
 */
static int test_set_verify_cert_store_ssl(void)
{
   SSL_CTX *ctx = NULL;
   SSL *ssl = NULL;
   int testresult = 0;
   X509_STORE *store = NULL, *new_store = NULL,
              *cstore = NULL, *new_cstore = NULL;

   /* Create an initial SSL_CTX. */
   ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
   if (!TEST_ptr(ctx))
       goto end;

   /* Create an SSL object. */
   ssl = SSL_new(ctx);
   if (!TEST_ptr(ssl))
       goto end;

   /* Retrieve verify store pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   /* Retrieve chain store pointer. */
   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   /* We haven't set any yet, so this should be NULL. */
   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   /* Create stores. We use separate stores so pointers are different. */
   new_store = X509_STORE_new();
   if (!TEST_ptr(new_store))
       goto end;

   new_cstore = X509_STORE_new();
   if (!TEST_ptr(new_cstore))
       goto end;

   /* Set stores. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, new_store)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, new_cstore)))
       goto end;

   /* Should be able to retrieve the same pointer. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_eq(store, new_store) || !TEST_ptr_eq(cstore, new_cstore))
       goto end;

   /* Should be able to unset again. */
   if (!TEST_true(SSL_set1_verify_cert_store(ssl, NULL)))
       goto end;

   if (!TEST_true(SSL_set1_chain_cert_store(ssl, NULL)))
       goto end;

   /* Should now be NULL. */
   if (!TEST_true(SSL_get0_verify_cert_store(ssl, &store)))
       goto end;

   if (!TEST_true(SSL_get0_chain_cert_store(ssl, &cstore)))
       goto end;

   if (!TEST_ptr_null(store) || !TEST_ptr_null(cstore))
       goto end;

   testresult = 1;

end:
   X509_STORE_free(new_store);
   X509_STORE_free(new_cstore);
   SSL_free(ssl);
   SSL_CTX_free(ctx);
   return testresult;
}


static int test_inherit_verify_param(void)
{
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    X509_VERIFY_PARAM *cp = NULL;
    SSL *ssl = NULL;
    X509_VERIFY_PARAM *sp = NULL;
    int hostflags = X509_CHECK_FLAG_NEVER_CHECK_SUBJECT;

    ctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method());
    if (!TEST_ptr(ctx))
        goto end;

    cp = SSL_CTX_get0_param(ctx);
    if (!TEST_ptr(cp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(cp), 0))
        goto end;

    X509_VERIFY_PARAM_set_hostflags(cp, hostflags);

    ssl = SSL_new(ctx);
    if (!TEST_ptr(ssl))
        goto end;

    sp = SSL_get0_param(ssl);
    if (!TEST_ptr(sp))
        goto end;
    if (!TEST_int_eq(X509_VERIFY_PARAM_get_hostflags(sp), hostflags))
        goto end;

    testresult = 1;

 end:
    SSL_free(ssl);
    SSL_CTX_free(ctx);

    return testresult;
}


static int test_load_dhfile(void)
{
#ifndef OPENSSL_NO_DH
    int testresult = 0;

    SSL_CTX *ctx = NULL;
    SSL_CONF_CTX *cctx = NULL;

    if (dhfile == NULL)
        return 1;

    if (!TEST_ptr(ctx = SSL_CTX_new_ex(libctx, NULL, TLS_client_method()))
        || !TEST_ptr(cctx = SSL_CONF_CTX_new()))
        goto end;

    SSL_CONF_CTX_set_ssl_ctx(cctx, ctx);
    SSL_CONF_CTX_set_flags(cctx,
                           SSL_CONF_FLAG_CERTIFICATE
                           | SSL_CONF_FLAG_SERVER
                           | SSL_CONF_FLAG_FILE);

    if (!TEST_int_eq(SSL_CONF_cmd(cctx, "DHParameters", dhfile), 2))
        goto end;

    testresult = 1;
end:
    SSL_CONF_CTX_free(cctx);
    SSL_CTX_free(ctx);

    return testresult;
#else
    return TEST_skip("DH not supported by this build");
#endif
}

#ifndef OPENSSL_NO_QUIC
static int test_quic_set_encryption_secrets(SSL *ssl,
                                            OSSL_ENCRYPTION_LEVEL level,
                                            const uint8_t *read_secret,
                                            const uint8_t *write_secret,
                                            size_t secret_len)
{
    test_printf_stderr("quic_set_encryption_secrets() %s, lvl=%d, len=%zd\n",
                       ssl->server ? "server" : "client", level, secret_len);
    return 1;
}

static int test_quic_add_handshake_data(SSL *ssl, OSSL_ENCRYPTION_LEVEL level,
                                        const uint8_t *data, size_t len)
{
    SSL *peer = (SSL*)SSL_get_app_data(ssl);

    TEST_info("quic_add_handshake_data() %s, lvl=%d, *data=0x%02X, len=%zd\n",
              ssl->server ? "server" : "client", level, (int)*data, len);
    if (!TEST_ptr(peer))
        return 0;

    /* We're called with what is locally written; this gives it to the peer */
    if (!TEST_true(SSL_provide_quic_data(peer, level, data, len))) {
        ERR_print_errors_fp(stderr);
        return 0;
    }

    return 1;
}

static int test_quic_flush_flight(SSL *ssl)
{
    test_printf_stderr("quic_flush_flight() %s\n", ssl->server ? "server" : "client");
    return 1;
}

static int test_quic_send_alert(SSL *ssl, enum ssl_encryption_level_t level, uint8_t alert)
{
    test_printf_stderr("quic_send_alert() %s, lvl=%d, alert=%d\n",
                       ssl->server ? "server" : "client", level, alert);
    return 1;
}

static SSL_QUIC_METHOD quic_method = {
    test_quic_set_encryption_secrets,
    test_quic_add_handshake_data,
    test_quic_flush_flight,
    test_quic_send_alert,
};

static int test_quic_api_set_versions(SSL *ssl, int ver)
{
    SSL_set_quic_transport_version(ssl, ver);
    return 1;
}

static int test_quic_api_version(int clnt, int srvr)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";
    const uint8_t *peer_str;
    size_t peer_str_len;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);

    if (!TEST_true(create_ssl_ctx_pair(libctx,
                                       TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_3_VERSION, 0,
                                       &sctx, &cctx, cert, privkey))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_true(SSL_CTX_set_quic_method(cctx, &quic_method))
            || !TEST_true(create_ssl_objects(sctx, cctx, &serverssl,
                                             &clientssl, NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(serverssl, clientssl))
            || !TEST_true(SSL_set_app_data(clientssl, serverssl))
            || !TEST_true(test_quic_api_set_versions(clientssl, clnt))
            || !TEST_true(test_quic_api_set_versions(serverssl, srvr))
            || !TEST_true(create_bare_ssl_connection(serverssl, clientssl,
                                                     SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_version(serverssl) == TLS1_3_VERSION)
            || !TEST_true(SSL_version(clientssl) == TLS1_3_VERSION)
            || !(TEST_int_eq(SSL_quic_read_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_read_level(serverssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(clientssl), ssl_encryption_application))
            || !(TEST_int_eq(SSL_quic_write_level(serverssl), ssl_encryption_application)))
        goto end;

    SSL_get_peer_quic_transport_params(serverssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, client_str, strlen(client_str)+1))
        goto end;
    SSL_get_peer_quic_transport_params(clientssl, &peer_str, &peer_str_len);
    if (!TEST_mem_eq(peer_str, peer_str_len, server_str, strlen(server_str)+1))
        goto end;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(clientssl)))
        goto end;

    /* Dummy handshake call should succeed */
    if (!TEST_true(SSL_do_handshake(clientssl)))
        goto end;
    /* Test that we (correctly) fail to send KeyUpdate */
    if (!TEST_true(SSL_key_update(clientssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(clientssl), 0))
        goto end;
    if (!TEST_true(SSL_key_update(serverssl, SSL_KEY_UPDATE_NOT_REQUESTED))
            || !TEST_int_le(SSL_do_handshake(serverssl), 0))
        goto end;

    TEST_info("original clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (srvr == 0 && clnt == 0)
        srvr = clnt = TLSEXT_TYPE_quic_transport_parameters;
    else if (srvr == 0)
        srvr = clnt;
    else if (clnt == 0)
        clnt = srvr;
    TEST_info("expected clnt=0x%X, srvr=0x%X\n", clnt, srvr);
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(serverssl), clnt))
        goto end;
    if (!TEST_int_eq(SSL_get_peer_quic_transport_version(clientssl), srvr))
        goto end;

    testresult = 1;

 end:
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);

    return testresult;
}

static int test_quic_api(int tst)
{
    SSL_CTX *sctx = NULL;
    SSL *serverssl = NULL;
    int testresult = 0;
    static int clnt_params[] = { 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int srvr_params[] = { 0,
                                 0,
                                 0,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters_draft,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters,
                                 TLSEXT_TYPE_quic_transport_parameters };
    static int results[] = { 1, 1, 1, 1, 1, 0, 1, 0, 1 };

    /* Failure cases:
     * test 6/[5] clnt = parameters, srvr = draft
     * test 8/[7] clnt = draft, srvr = parameters
     */

    /* Clean up logging space */
    memset(client_log_buffer, 0, sizeof(client_log_buffer));
    memset(server_log_buffer, 0, sizeof(server_log_buffer));
    client_log_buffer_index = 0;
    server_log_buffer_index = 0;
    error_writing_log = 0;

    if (!TEST_ptr(sctx = SSL_CTX_new_ex(libctx, NULL, TLS_server_method()))
            || !TEST_true(SSL_CTX_set_quic_method(sctx, &quic_method))
            || !TEST_ptr(sctx->quic_method)
            || !TEST_ptr(serverssl = SSL_new(sctx))
            || !TEST_true(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, NULL))
            || !TEST_false(SSL_IS_QUIC(serverssl))
            || !TEST_true(SSL_set_quic_method(serverssl, &quic_method))
            || !TEST_true(SSL_IS_QUIC(serverssl)))
        goto end;

    if (!TEST_int_eq(test_quic_api_version(clnt_params[tst], srvr_params[tst]), results[tst]))
        goto end;

    testresult = 1;

end:
    SSL_CTX_free(sctx);
    sctx = NULL;
    SSL_free(serverssl);
    serverssl = NULL;
    return testresult;
}

# ifndef OSSL_NO_USABLE_TLS1_3
/*
 * Helper method to setup objects for QUIC early data test. Caller
 * frees objects on error.
 */
static int quic_setupearly_data_test(SSL_CTX **cctx, SSL_CTX **sctx,
                                     SSL **clientssl, SSL **serverssl,
                                     SSL_SESSION **sess, int idx)
{
    static const char *server_str = "SERVER";
    static const char *client_str = "CLIENT";

    if (*sctx == NULL
            && (!TEST_true(create_ssl_ctx_pair(libctx, TLS_server_method(),
                                               TLS_client_method(),
                                               TLS1_3_VERSION, 0,
                                               sctx, cctx, cert, privkey))
                || !TEST_true(SSL_CTX_set_quic_method(*sctx, &quic_method))
                || !TEST_true(SSL_CTX_set_quic_method(*cctx, &quic_method))
                || !TEST_true(SSL_CTX_set_max_early_data(*sctx, 0xffffffffu))))
        return 0;

    if (idx == 1) {
        /* When idx == 1 we repeat the tests with read_ahead set */
        SSL_CTX_set_read_ahead(*cctx, 1);
        SSL_CTX_set_read_ahead(*sctx, 1);
    } else if (idx == 2) {
        /* When idx == 2 we are doing early_data with a PSK. Set up callbacks */
        SSL_CTX_set_psk_use_session_callback(*cctx, use_session_cb);
        SSL_CTX_set_psk_find_session_callback(*sctx, find_session_cb);
        use_session_cb_cnt = 0;
        find_session_cb_cnt = 0;
        srvid = pskid;
    }

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl, clientssl,
                                      NULL, NULL))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    /*
     * For one of the run throughs (doesn't matter which one), we'll try sending
     * some SNI data in the initial ClientHello. This will be ignored (because
     * there is no SNI cb set up by the server), so it should not impact
     * early_data.
     */
    if (idx == 1
            && !TEST_true(SSL_set_tlsext_host_name(*clientssl, "localhost")))
        return 0;

    if (idx == 2) {
        clientpsk = create_a_psk(*clientssl);
        if (!TEST_ptr(clientpsk)
                || !TEST_true(SSL_SESSION_set_max_early_data(clientpsk,
                                                             0xffffffffu))
                || !TEST_true(SSL_SESSION_up_ref(clientpsk))) {
            SSL_SESSION_free(clientpsk);
            clientpsk = NULL;
            return 0;
        }
        serverpsk = clientpsk;

        if (sess != NULL) {
            if (!TEST_true(SSL_SESSION_up_ref(clientpsk))) {
                SSL_SESSION_free(clientpsk);
                SSL_SESSION_free(serverpsk);
                clientpsk = serverpsk = NULL;
                return 0;
            }
            *sess = clientpsk;
        }

        SSL_set_quic_early_data_enabled(*serverssl, 1);
        SSL_set_quic_early_data_enabled(*clientssl, 1);

        return 1;
    }

    if (sess == NULL)
        return 1;

    if (!TEST_true(create_bare_ssl_connection(*serverssl, *clientssl,
                                              SSL_ERROR_NONE, 0)))
        return 0;

    /* Deal with two NewSessionTickets */
    if (!TEST_true(SSL_process_quic_post_handshake(*clientssl)))
        return 0;

    *sess = SSL_get1_session(*clientssl);
    SSL_shutdown(*clientssl);
    SSL_shutdown(*serverssl);
    SSL_free(*serverssl);
    SSL_free(*clientssl);
    *serverssl = *clientssl = NULL;

    if (!TEST_true(create_ssl_objects(*sctx, *cctx, serverssl,
                                      clientssl, NULL, NULL))
            || !TEST_true(SSL_set_session(*clientssl, *sess))
            || !TEST_true(SSL_set_quic_transport_params(*serverssl,
                                                        (unsigned char*)server_str,
                                                        strlen(server_str)+1))
            || !TEST_true(SSL_set_quic_transport_params(*clientssl,
                                                        (unsigned char*)client_str,
                                                        strlen(client_str)+1))
            || !TEST_true(SSL_set_app_data(*serverssl, *clientssl))
            || !TEST_true(SSL_set_app_data(*clientssl, *serverssl)))
        return 0;

    SSL_set_quic_early_data_enabled(*serverssl, 1);
    SSL_set_quic_early_data_enabled(*clientssl, 1);

    return 1;
}

static int test_quic_early_data(int tst)
{
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;
    SSL_SESSION *sess = NULL;

    if (!TEST_true(quic_setupearly_data_test(&cctx, &sctx, &clientssl,
                                             &serverssl, &sess, tst)))
        goto end;

    if (!TEST_true(create_bare_ssl_connection(serverssl, clientssl, SSL_ERROR_NONE, 0))
            || !TEST_true(SSL_get_early_data_status(serverssl)))
        goto end;

    testresult = 1;

 end:
    SSL_SESSION_free(sess);
    SSL_SESSION_free(clientpsk);
    SSL_SESSION_free(serverpsk);
    clientpsk = serverpsk = NULL;
    SSL_free(serverssl);
    SSL_free(clientssl);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}
# endif /* OSSL_NO_USABLE_TLS1_3 */
#endif /* OPENSSL_NO_QUIC */

OPT_TEST_DECLARE_USAGE("certfile privkeyfile srpvfile tmpfile provider config dhfile\n")

int setup_tests(void)
{
    char *modulename;
    char *configfile;

    libctx = OSSL_LIB_CTX_new();
    if (!TEST_ptr(libctx))
        return 0;

    defctxnull = OSSL_PROVIDER_load(NULL, "null");

    /*
     * Verify that the default and fips providers in the default libctx are not
     * available
     */
    if (!TEST_false(OSSL_PROVIDER_available(NULL, "default"))
            || !TEST_false(OSSL_PROVIDER_available(NULL, "fips")))
        return 0;

    if (!test_skip_common_options()) {
        TEST_error("Error parsing test options\n");
        return 0;
    }

    if (!TEST_ptr(certsdir = test_get_argument(0))
            || !TEST_ptr(srpvfile = test_get_argument(1))
            || !TEST_ptr(tmpfilename = test_get_argument(2))
            || !TEST_ptr(modulename = test_get_argument(3))
            || !TEST_ptr(configfile = test_get_argument(4))
            || !TEST_ptr(dhfile = test_get_argument(5)))
        return 0;

    if (!TEST_true(OSSL_LIB_CTX_load_config(libctx, configfile)))
        return 0;

    /* Check we have the expected provider available */
    if (!TEST_true(OSSL_PROVIDER_available(libctx, modulename)))
        return 0;

    /* Check the default provider is not available */
    if (strcmp(modulename, "default") != 0
            && !TEST_false(OSSL_PROVIDER_available(libctx, "default")))
        return 0;

    if (strcmp(modulename, "fips") == 0)
        is_fips = 1;

    /*
     * We add, but don't load the test "tls-provider". We'll load it when we
     * need it.
     */
    if (!TEST_true(OSSL_PROVIDER_add_builtin(libctx, "tls-provider",
                                             tls_provider_init)))
        return 0;


    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
    {
        "AES-128-CBC-HMAC-SHA1",
        "AES-128-CBC-HMAC-SHA256",
        "AES-256-CBC-HMAC-SHA1",
        "AES-256-CBC-HMAC-SHA256"
    };
    const char *cipherlist = multiblock_cipherlist_data[test_index];
    const SSL_METHOD *smeth = TLS_server_method();
    const SSL_METHOD *cmeth = TLS_client_method();
    int min_version = TLS1_VERSION;
    int max_version = TLS1_2_VERSION; /* Don't select TLS1_3 */
    SSL_CTX *cctx = NULL, *sctx = NULL;
    SSL *clientssl = NULL, *serverssl = NULL;
    int testresult = 0;

    /*
     * Choose a buffer large enough to perform a multi-block operation
     * i.e: write_len >= 4 * frag_size
     * 9 * is chosen so that multiple multiblocks are used + some leftover.
     */
    unsigned char msg[MULTIBLOCK_FRAGSIZE * 9];
    unsigned char buf[sizeof(msg)], *p = buf;
    size_t readbytes, written, len;
    EVP_CIPHER *ciph = NULL;

    /*
     * Check if the cipher exists before attempting to use it since it only has
     * a hardware specific implementation.
     */
    ciph = EVP_CIPHER_fetch(libctx, fetchable_ciphers[test_index], "");
    if (ciph == NULL) {
        TEST_skip("Multiblock cipher is not available for %s", cipherlist);
        return 1;
    }
    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}
    if (getenv("OPENSSL_TEST_GETCOUNTS") != NULL) {
#ifdef OPENSSL_NO_CRYPTO_MDEBUG
        TEST_error("not supported in this build");
        return 0;
#else
        int i, mcount, rcount, fcount;

        for (i = 0; i < 4; i++)
            test_export_key_mat(i);
        CRYPTO_get_alloc_counts(&mcount, &rcount, &fcount);
        test_printf_stdout("malloc %d realloc %d free %d\n",
                mcount, rcount, fcount);
        return 1;
#endif
    }

    cert = test_mk_file_path(certsdir, "servercert.pem");
    if (cert == NULL)
        goto err;

    privkey = test_mk_file_path(certsdir, "serverkey.pem");
    if (privkey == NULL)
        goto err;

    cert2 = test_mk_file_path(certsdir, "server-ecdsa-cert.pem");
    if (cert2 == NULL)
        goto err;

    privkey2 = test_mk_file_path(certsdir, "server-ecdsa-key.pem");
    if (privkey2 == NULL)
        goto err;

    cert1024 = test_mk_file_path(certsdir, "ee-cert-1024.pem");
    if (cert1024 == NULL)
        goto err;

    privkey1024 = test_mk_file_path(certsdir, "ee-key-1024.pem");
    if (privkey1024 == NULL)
        goto err;

    cert3072 = test_mk_file_path(certsdir, "ee-cert-3072.pem");
    if (cert3072 == NULL)
        goto err;

    privkey3072 = test_mk_file_path(certsdir, "ee-key-3072.pem");
    if (privkey3072 == NULL)
        goto err;

    cert4096 = test_mk_file_path(certsdir, "ee-cert-4096.pem");
    if (cert4096 == NULL)
        goto err;

    privkey4096 = test_mk_file_path(certsdir, "ee-key-4096.pem");
    if (privkey4096 == NULL)
        goto err;

    cert8192 = test_mk_file_path(certsdir, "ee-cert-8192.pem");
    if (cert8192 == NULL)
        goto err;

    privkey8192 = test_mk_file_path(certsdir, "ee-key-8192.pem");
    if (privkey8192 == NULL)
        goto err;

#if !defined(OPENSSL_NO_KTLS) && !defined(OPENSSL_NO_SOCK)
# if !defined(OPENSSL_NO_TLS1_2) || !defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_ktls, NUM_KTLS_TEST_CIPHERS * 4);
    ADD_ALL_TESTS(test_ktls_sendfile, NUM_KTLS_TEST_CIPHERS);
# endif
#endif
    ADD_TEST(test_large_message_tls);
    ADD_TEST(test_large_message_tls_read_ahead);
#ifndef OPENSSL_NO_DTLS
    ADD_TEST(test_large_message_dtls);
#endif
    ADD_TEST(test_cleanse_plaintext);
#ifndef OPENSSL_NO_OCSP
    ADD_TEST(test_tlsext_status_type);
#endif
    ADD_TEST(test_session_with_only_int_cache);
    ADD_TEST(test_session_with_only_ext_cache);
    ADD_TEST(test_session_with_both_cache);
    ADD_TEST(test_session_wo_ca_names);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_stateful_tickets, 3);
    ADD_ALL_TESTS(test_stateless_tickets, 3);
    ADD_TEST(test_psk_tickets);
    ADD_ALL_TESTS(test_extra_tickets, 6);
#endif
    ADD_ALL_TESTS(test_ssl_set_bio, TOTAL_SSL_SET_BIO_TESTS);
    ADD_TEST(test_ssl_bio_pop_next_bio);
    ADD_TEST(test_ssl_bio_pop_ssl_bio);
    ADD_TEST(test_ssl_bio_change_rbio);
    ADD_TEST(test_ssl_bio_change_wbio);
#if !defined(OPENSSL_NO_TLS1_2) || defined(OSSL_NO_USABLE_TLS1_3)
    ADD_ALL_TESTS(test_set_sigalgs, OSSL_NELEM(testsigalgs) * 2);
    ADD_TEST(test_keylog);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_keylog_no_master_key);
#endif
    ADD_TEST(test_client_cert_verify_cb);
    ADD_TEST(test_ssl_build_cert_chain);
    ADD_TEST(test_ssl_ctx_build_cert_chain);
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_client_hello_cb);
    ADD_TEST(test_no_ems);
    ADD_TEST(test_ccs_change_cipher);
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_early_data_read_write, 3);
    /*
     * We don't do replay tests for external PSK. Replay protection isn't used
     * in that scenario.
     */
    ADD_ALL_TESTS(test_early_data_replay, 2);
    ADD_ALL_TESTS(test_early_data_skip, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr, 3);
    ADD_ALL_TESTS(test_early_data_skip_hrr_fail, 3);
    ADD_ALL_TESTS(test_early_data_skip_abort, 3);
    ADD_ALL_TESTS(test_early_data_not_sent, 3);
    ADD_ALL_TESTS(test_early_data_psk, 8);
    ADD_ALL_TESTS(test_early_data_psk_with_all_ciphers, 5);
    ADD_ALL_TESTS(test_early_data_not_expected, 3);
# ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_early_data_tls1_2, 3);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_set_ciphersuite, 10);
    ADD_TEST(test_ciphersuite_change);
    ADD_ALL_TESTS(test_tls13_ciphersuite, 4);
# ifdef OPENSSL_NO_PSK
    ADD_ALL_TESTS(test_tls13_psk, 1);
# else
    ADD_ALL_TESTS(test_tls13_psk, 4);
# endif  /* OPENSSL_NO_PSK */
# ifndef OPENSSL_NO_TLS1_2
    /* Test with both TLSv1.3 and 1.2 versions */
    ADD_ALL_TESTS(test_key_exchange, 14);
#  if !defined(OPENSSL_NO_EC) && !defined(OPENSSL_NO_DH)
    ADD_ALL_TESTS(test_negotiated_group,
                  4 * (OSSL_NELEM(ecdhe_kexch_groups)
                       + OSSL_NELEM(ffdhe_kexch_groups)));
#  endif
# else
    /* Test with only TLSv1.3 versions */
    ADD_ALL_TESTS(test_key_exchange, 12);
# endif
    ADD_ALL_TESTS(test_custom_exts, 6);
    ADD_TEST(test_stateless);
    ADD_TEST(test_pha_key_update);
#else
    ADD_ALL_TESTS(test_custom_exts, 3);
#endif
    ADD_ALL_TESTS(test_export_key_mat, 6);
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_ALL_TESTS(test_export_key_mat_early, 3);
    ADD_TEST(test_key_update);
    ADD_ALL_TESTS(test_key_update_peer_in_write, 2);
    ADD_ALL_TESTS(test_key_update_peer_in_read, 2);
    ADD_ALL_TESTS(test_key_update_local_in_write, 2);
    ADD_ALL_TESTS(test_key_update_local_in_read, 2);
#endif
    ADD_ALL_TESTS(test_ssl_clear, 2);
    ADD_ALL_TESTS(test_max_fragment_len_ext, OSSL_NELEM(max_fragment_len_test));
#if !defined(OPENSSL_NO_SRP) && !defined(OPENSSL_NO_TLS1_2)
    ADD_ALL_TESTS(test_srp, 6);
#endif
    ADD_ALL_TESTS(test_info_callback, 6);
    ADD_ALL_TESTS(test_ssl_pending, 2);
    ADD_ALL_TESTS(test_ssl_get_shared_ciphers, OSSL_NELEM(shared_ciphers_data));
    ADD_ALL_TESTS(test_ticket_callbacks, 20);
    ADD_ALL_TESTS(test_shutdown, 7);
    ADD_ALL_TESTS(test_incorrect_shutdown, 2);
    ADD_ALL_TESTS(test_cert_cb, 6);
    ADD_ALL_TESTS(test_client_cert_cb, 2);
    ADD_ALL_TESTS(test_ca_names, 3);
#ifndef OPENSSL_NO_TLS1_2
    ADD_ALL_TESTS(test_multiblock_write, OSSL_NELEM(multiblock_cipherlist_data));
#endif
    ADD_ALL_TESTS(test_servername, 10);
#if !defined(OPENSSL_NO_EC) \
    && (!defined(OSSL_NO_USABLE_TLS1_3) || !defined(OPENSSL_NO_TLS1_2))
    ADD_ALL_TESTS(test_sigalgs_available, 6);
#endif
#ifndef OPENSSL_NO_TLS1_3
    ADD_ALL_TESTS(test_pluggable_group, 2);
#endif
#ifndef OPENSSL_NO_TLS1_2
    ADD_TEST(test_ssl_dup);
# ifndef OPENSSL_NO_DH
    ADD_ALL_TESTS(test_set_tmp_dh, 11);
    ADD_ALL_TESTS(test_dh_auto, 7);
# endif
#endif
#ifndef OSSL_NO_USABLE_TLS1_3
    ADD_TEST(test_sni_tls13);
    ADD_ALL_TESTS(test_ticket_lifetime, 2);
#endif
    ADD_TEST(test_inherit_verify_param);
    ADD_TEST(test_set_alpn);
    ADD_TEST(test_set_verify_cert_store_ssl_ctx);
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
# endif
#endif
    return 1;

 err:
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    return 0;
}

void cleanup_tests(void)
{
# if !defined(OPENSSL_NO_TLS1_2) && !defined(OPENSSL_NO_DH)
    EVP_PKEY_free(tmp_dh_params);
#endif
    OPENSSL_free(cert);
    OPENSSL_free(privkey);
    OPENSSL_free(cert2);
    OPENSSL_free(privkey2);
    OPENSSL_free(cert1024);
    OPENSSL_free(privkey1024);
    OPENSSL_free(cert3072);
    OPENSSL_free(privkey3072);
    OPENSSL_free(cert4096);
    OPENSSL_free(privkey4096);
    OPENSSL_free(cert8192);
    OPENSSL_free(privkey8192);
    bio_s_mempacket_test_free();
    bio_s_always_retry_free();
    OSSL_PROVIDER_unload(defctxnull);
    OSSL_LIB_CTX_free(libctx);
}