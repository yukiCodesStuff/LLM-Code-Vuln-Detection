    if (self == NULL) {
        SSL_CTX_free(ctx);
        return NULL;
    }
    self->ctx = ctx;
    /* Defaults */
    SSL_CTX_set_verify(self->ctx, SSL_VERIFY_NONE, NULL);
    SSL_CTX_set_options(self->ctx,
                        SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);

#define SID_CTX "Python"
    SSL_CTX_set_session_id_context(self->ctx, (const unsigned char *) SID_CTX,
                                   sizeof(SID_CTX));
#undef SID_CTX

    return (PyObject *)self;
}

static void
context_dealloc(PySSLContext *self)
{
    SSL_CTX_free(self->ctx);
    Py_TYPE(self)->tp_free(self);
}

static PyObject *
set_ciphers(PySSLContext *self, PyObject *args)
{
    int ret;
    const char *cipherlist;

    if (!PyArg_ParseTuple(args, "s:set_ciphers", &cipherlist))
        return NULL;
    ret = SSL_CTX_set_cipher_list(self->ctx, cipherlist);
    if (ret == 0) {
        /* Clearing the error queue is necessary on some OpenSSL versions,
           otherwise the error will be reported again when another SSL call
           is done. */
        ERR_clear_error();
        PyErr_SetString(PySSLErrorObject,
                        "No cipher can be selected.");
        return NULL;
    }
    if (!_setup_ssl_threads()) {
        return NULL;
    }
#endif
    OpenSSL_add_all_algorithms();

    /* Add symbols to module dict */
    PySSLErrorObject = PyErr_NewException("ssl.SSLError",
                                          PySocketModule.error,
                                          NULL);
    if (PySSLErrorObject == NULL)
        return NULL;
    if (PyDict_SetItemString(d, "SSLError", PySSLErrorObject) != 0)
        return NULL;
    if (PyDict_SetItemString(d, "_SSLContext",
                             (PyObject *)&PySSLContext_Type) != 0)
        return NULL;
    if (PyDict_SetItemString(d, "_SSLSocket",
                             (PyObject *)&PySSLSocket_Type) != 0)
        return NULL;
    PyModule_AddIntConstant(m, "SSL_ERROR_ZERO_RETURN",
                            PY_SSL_ERROR_ZERO_RETURN);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_READ",
                            PY_SSL_ERROR_WANT_READ);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_WRITE",
                            PY_SSL_ERROR_WANT_WRITE);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_X509_LOOKUP",
                            PY_SSL_ERROR_WANT_X509_LOOKUP);
    PyModule_AddIntConstant(m, "SSL_ERROR_SYSCALL",
                            PY_SSL_ERROR_SYSCALL);
    PyModule_AddIntConstant(m, "SSL_ERROR_SSL",
                            PY_SSL_ERROR_SSL);
    PyModule_AddIntConstant(m, "SSL_ERROR_WANT_CONNECT",
                            PY_SSL_ERROR_WANT_CONNECT);
    /* non ssl.h errorcodes */
    PyModule_AddIntConstant(m, "SSL_ERROR_EOF",
                            PY_SSL_ERROR_EOF);
    PyModule_AddIntConstant(m, "SSL_ERROR_INVALID_ERROR_CODE",
                            PY_SSL_ERROR_INVALID_ERROR_CODE);
    /* cert requirements */
    PyModule_AddIntConstant(m, "CERT_NONE",
                            PY_SSL_CERT_NONE);
    PyModule_AddIntConstant(m, "CERT_OPTIONAL",
                            PY_SSL_CERT_OPTIONAL);
    PyModule_AddIntConstant(m, "CERT_REQUIRED",
                            PY_SSL_CERT_REQUIRED);

    /* protocol versions */
#ifndef OPENSSL_NO_SSL2
    PyModule_AddIntConstant(m, "PROTOCOL_SSLv2",
                            PY_SSL_VERSION_SSL2);
#endif
    PyModule_AddIntConstant(m, "PROTOCOL_SSLv3",
                            PY_SSL_VERSION_SSL3);
    PyModule_AddIntConstant(m, "PROTOCOL_SSLv23",
                            PY_SSL_VERSION_SSL23);
    PyModule_AddIntConstant(m, "PROTOCOL_TLSv1",
                            PY_SSL_VERSION_TLS1);

    /* protocol options */
    PyModule_AddIntConstant(m, "OP_ALL",
                            SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);
    PyModule_AddIntConstant(m, "OP_NO_SSLv2", SSL_OP_NO_SSLv2);
    PyModule_AddIntConstant(m, "OP_NO_SSLv3", SSL_OP_NO_SSLv3);
    PyModule_AddIntConstant(m, "OP_NO_TLSv1", SSL_OP_NO_TLSv1);

#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
    r = Py_True;
#else
    r = Py_False;
#endif
    Py_INCREF(r);
    PyModule_AddObject(m, "HAS_SNI", r);

    /* OpenSSL version */
    /* SSLeay() gives us the version of the library linked against,
       which could be different from the headers version.
    */
    libver = SSLeay();
    r = PyLong_FromUnsignedLong(libver);
    if (r == NULL)
        return NULL;
    if (PyModule_AddObject(m, "OPENSSL_VERSION_NUMBER", r))
        return NULL;
    parse_openssl_version(libver, &major, &minor, &fix, &patch, &status);
    r = Py_BuildValue("IIIII", major, minor, fix, patch, status);
    if (r == NULL || PyModule_AddObject(m, "OPENSSL_VERSION_INFO", r))
        return NULL;
    r = PyUnicode_FromString(SSLeay_version(SSLEAY_VERSION));
    if (r == NULL || PyModule_AddObject(m, "OPENSSL_VERSION", r))
        return NULL;

    libver = OPENSSL_VERSION_NUMBER;
    parse_openssl_version(libver, &major, &minor, &fix, &patch, &status);
    r = Py_BuildValue("IIIII", major, minor, fix, patch, status);
    if (r == NULL || PyModule_AddObject(m, "_OPENSSL_API_VERSION", r))
        return NULL;

    return m;
}