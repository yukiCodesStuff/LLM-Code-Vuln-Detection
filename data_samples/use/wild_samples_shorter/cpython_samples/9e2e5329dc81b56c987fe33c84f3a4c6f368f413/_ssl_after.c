    self->ctx = ctx;
    /* Defaults */
    SSL_CTX_set_verify(self->ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_options(self->ctx,
                        SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);

#define SID_CTX "Python"
    SSL_CTX_set_session_id_context(self->ctx, (const unsigned char *) SID_CTX,
                                   sizeof(SID_CTX));
                            PY_SSL_VERSION_TLS1);

    /* protocol options */
    PyModule_AddIntConstant(m, "OP_ALL",
                            SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);
    PyModule_AddIntConstant(m, "OP_NO_SSLv2", SSL_OP_NO_SSLv2);
    PyModule_AddIntConstant(m, "OP_NO_SSLv3", SSL_OP_NO_SSLv3);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1", SSL_OP_NO_TLSv1);
    PyModule_AddIntConstant(m, "OP_CIPHER_SERVER_PREFERENCE",