            if (ERR_peek_last_error() != 0) {
                _setSSLError(NULL, ret, __FILE__, __LINE__);
                goto fail;
            }
        }
    }

    /* ssl compatibility */
    SSL_CTX_set_options(self->ctx,
                        SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);

    verification_mode = SSL_VERIFY_NONE;
    if (certreq == PY_SSL_CERT_OPTIONAL)
        verification_mode = SSL_VERIFY_PEER;
    else if (certreq == PY_SSL_CERT_REQUIRED)
        verification_mode = (SSL_VERIFY_PEER |
                             SSL_VERIFY_FAIL_IF_NO_PEER_CERT);
    SSL_CTX_set_verify(self->ctx, verification_mode,
                       NULL); /* set verify lvl */

    PySSL_BEGIN_ALLOW_THREADS
    self->ssl = SSL_new(self->ctx); /* New ssl struct */
    PySSL_END_ALLOW_THREADS
    SSL_set_fd(self->ssl, Sock->sock_fd);       /* Set the socket for SSL */
#ifdef SSL_MODE_AUTO_RETRY
    SSL_set_mode(self->ssl, SSL_MODE_AUTO_RETRY);
#endif

    /* If the socket is in non-blocking mode or timeout mode, set the BIO
     * to non-blocking mode (blocking is the default)
     */
    if (Sock->sock_timeout >= 0.0) {
        /* Set both the read and write BIO's to non-blocking mode */
        BIO_set_nbio(SSL_get_rbio(self->ssl), 1);
        BIO_set_nbio(SSL_get_wbio(self->ssl), 1);
    }