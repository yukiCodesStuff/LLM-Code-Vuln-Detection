    }

    /* ssl compatibility */
    SSL_CTX_set_options(self->ctx,
                        SSL_OP_ALL & ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS);

    verification_mode = SSL_VERIFY_NONE;
    if (certreq == PY_SSL_CERT_OPTIONAL)
        verification_mode = SSL_VERIFY_PEER;