/*
 * Copyright 2016-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include "helpers/ssltestlib.h"
#include "testutil.h"

static int docorrupt = 0;

static void copy_flags(BIO *bio)
{
    int flags;
    BIO *next = BIO_next(bio);

    flags = BIO_test_flags(next, BIO_FLAGS_SHOULD_RETRY | BIO_FLAGS_RWS);
    BIO_clear_flags(bio, BIO_FLAGS_SHOULD_RETRY | BIO_FLAGS_RWS);
    BIO_set_flags(bio, flags);
}
{
    static unsigned char junk[16000] = { 0 };
    SSL_CTX *sctx = NULL, *cctx = NULL;
    SSL *server = NULL, *client = NULL;
    BIO *c_to_s_fbio;
    int testresult = 0;
    STACK_OF(SSL_CIPHER) *ciphers;
    const SSL_CIPHER *currcipher;
    int err;

    docorrupt = 0;

    ERR_clear_error();

    TEST_info("Starting #%d, %s", testidx, cipher_list[testidx]);

    if (!TEST_true(create_ssl_ctx_pair(NULL, TLS_server_method(),
                                       TLS_client_method(),
                                       TLS1_VERSION, 0,
                                       &sctx, &cctx, cert, privkey)))
        return 0;

    if (!TEST_true(SSL_CTX_set_dh_auto(sctx, 1))
            || !TEST_true(SSL_CTX_set_cipher_list(cctx, cipher_list[testidx]))
            || !TEST_true(SSL_CTX_set_ciphersuites(cctx, ""))
            || !TEST_ptr(ciphers = SSL_CTX_get_ciphers(cctx))
            || !TEST_int_eq(sk_SSL_CIPHER_num(ciphers), 1)
            || !TEST_ptr(currcipher = sk_SSL_CIPHER_value(ciphers, 0)))
        goto end;

    /*
     * No ciphers we are using are TLSv1.3 compatible so we should not attempt
     * to negotiate TLSv1.3
     */
    if (!TEST_true(SSL_CTX_set_max_proto_version(cctx, TLS1_2_VERSION)))
        goto end;

    if (!TEST_ptr(c_to_s_fbio = BIO_new(bio_f_tls_corrupt_filter())))
        goto end;

    /* BIO is freed by create_ssl_connection on error */
    if (!TEST_true(create_ssl_objects(sctx, cctx, &server, &client, NULL,
                                      c_to_s_fbio)))
        goto end;

    if (!TEST_true(create_ssl_connection(server, client, SSL_ERROR_NONE)))
        goto end;

    docorrupt = 1;

    if (!TEST_int_ge(SSL_write(client, junk, sizeof(junk)), 0))
        goto end;

    if (!TEST_int_lt(SSL_read(server, junk, sizeof(junk)), 0))
        goto end;

    do {
        err = ERR_get_error();

        if (err == 0) {
            TEST_error("Decryption failed or bad record MAC not seen");
            goto end;
        }
    } while (ERR_GET_REASON(err) != SSL_R_DECRYPTION_FAILED_OR_BAD_RECORD_MAC);

    testresult = 1;
 end:
    SSL_free(server);
    SSL_free(client);
    SSL_CTX_free(sctx);
    SSL_CTX_free(cctx);
    return testresult;
}