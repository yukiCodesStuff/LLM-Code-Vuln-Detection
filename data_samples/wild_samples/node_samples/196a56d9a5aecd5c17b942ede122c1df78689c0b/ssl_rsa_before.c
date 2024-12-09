        } else {
            if (!SSL_CTX_add_custom_ext(ctx, ext_type, context,
                                        serverinfoex_srv_add_cb,
                                        NULL, NULL,
                                        serverinfoex_srv_parse_cb,
                                        NULL))
                return 0;
        }
    }

    return 1;
}

int SSL_CTX_use_serverinfo_ex(SSL_CTX *ctx, unsigned int version,
                              const unsigned char *serverinfo,
                              size_t serverinfo_length)
{
    unsigned char *new_serverinfo;

    if (ctx == NULL || serverinfo == NULL || serverinfo_length == 0) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
{
    unsigned char *serverinfo = NULL;
    unsigned char *tmp;
    size_t serverinfo_length = 0;
    unsigned char *extension = 0;
    long extension_length = 0;
    char *name = NULL;
    char *header = NULL;
    static const char namePrefix1[] = "SERVERINFO FOR ";
    static const char namePrefix2[] = "SERVERINFOV2 FOR ";
    unsigned int name_len;
    int ret = 0;
    BIO *bin = NULL;
    size_t num_extensions = 0, contextoff = 0;

    if (ctx == NULL || file == NULL) {
        ERR_raise(ERR_LIB_SSL, ERR_R_PASSED_NULL_PARAMETER);
        goto end;
    }
    for (num_extensions = 0;; num_extensions++) {
        unsigned int version;

        if (PEM_read_bio(bin, &name, &header, &extension, &extension_length)
            == 0) {
            /*
             * There must be at least one extension in this file
             */
            if (num_extensions == 0) {
                ERR_raise(ERR_LIB_SSL, SSL_R_NO_PEM_EXTENSIONS);
                goto end;
            } else              /* End of file, we're done */
                break;
        }
        /* Check that PEM name starts with "BEGIN SERVERINFO FOR " */
        name_len = strlen(name);
        if (name_len < sizeof(namePrefix1) - 1) {
            ERR_raise(ERR_LIB_SSL, SSL_R_PEM_NAME_TOO_SHORT);
            goto end;
        }
        if (strncmp(name, namePrefix1, sizeof(namePrefix1) - 1) == 0) {
            version = SSL_SERVERINFOV1;
        } else {
            if (name_len < sizeof(namePrefix2) - 1) {
                ERR_raise(ERR_LIB_SSL, SSL_R_PEM_NAME_TOO_SHORT);
                goto end;
            }
            if (strncmp(name, namePrefix2, sizeof(namePrefix2) - 1) != 0) {
                ERR_raise(ERR_LIB_SSL, SSL_R_PEM_NAME_BAD_PREFIX);
                goto end;
            }
            version = SSL_SERVERINFOV2;
        }
        /*
         * Check that the decoded PEM data is plausible (valid length field)
         */
        if (version == SSL_SERVERINFOV1) {
            /* 4 byte header: 2 bytes type, 2 bytes len */
            if (extension_length < 4
                    || (extension[2] << 8) + extension[3]
                       != extension_length - 4) {
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
            /*
             * File does not have a context value so we must take account of
             * this later.
             */
            contextoff = 4;
        } else {
            /* 8 byte header: 4 bytes context, 2 bytes type, 2 bytes len */
            if (extension_length < 8
                    || (extension[6] << 8) + extension[7]
                       != extension_length - 8) {
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
        }
        /* Append the decoded extension to the serverinfo buffer */
        tmp = OPENSSL_realloc(serverinfo, serverinfo_length + extension_length
                                          + contextoff);
        if (tmp == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto end;
        }
        serverinfo = tmp;
        if (contextoff > 0) {
            unsigned char *sinfo = serverinfo + serverinfo_length;

            /* We know this only uses the last 2 bytes */
            sinfo[0] = 0;
            sinfo[1] = 0;
            sinfo[2] = (SYNTHV1CONTEXT >> 8) & 0xff;
            sinfo[3] = SYNTHV1CONTEXT & 0xff;
        }
        memcpy(serverinfo + serverinfo_length + contextoff,
               extension, extension_length);
        serverinfo_length += extension_length + contextoff;

        OPENSSL_free(name);
        name = NULL;
        OPENSSL_free(header);
        header = NULL;
        OPENSSL_free(extension);
        extension = NULL;
    }

    ret = SSL_CTX_use_serverinfo_ex(ctx, SSL_SERVERINFOV2, serverinfo,
                                    serverinfo_length);
 end:
    /* SSL_CTX_use_serverinfo makes a local copy of the serverinfo. */
    OPENSSL_free(name);
    OPENSSL_free(header);
    OPENSSL_free(extension);
    OPENSSL_free(serverinfo);
    BIO_free(bin);
    return ret;
}

static int ssl_set_cert_and_key(SSL *ssl, SSL_CTX *ctx, X509 *x509, EVP_PKEY *privatekey,
                                STACK_OF(X509) *chain, int override)
{
    int ret = 0;
    size_t i;
    int j;
    int rv;
    CERT *c = ssl != NULL ? ssl->cert : ctx->cert;
    STACK_OF(X509) *dup_chain = NULL;
    EVP_PKEY *pubkey = NULL;

    /* Do all security checks before anything else */
    rv = ssl_security_cert(ssl, ctx, x509, 0, 1);
    if (rv != 1) {
        ERR_raise(ERR_LIB_SSL, rv);
        goto out;
    }
    for (j = 0; j < sk_X509_num(chain); j++) {
        rv = ssl_security_cert(ssl, ctx, sk_X509_value(chain, j), 0, 0);
        if (rv != 1) {
            ERR_raise(ERR_LIB_SSL, rv);
            goto out;
        }
    }

    pubkey = X509_get_pubkey(x509); /* bumps reference */
    if (pubkey == NULL)
        goto out;
    if (privatekey == NULL) {
        privatekey = pubkey;
    } else {
        /* For RSA, which has no parameters, missing returns 0 */
        if (EVP_PKEY_missing_parameters(privatekey)) {
            if (EVP_PKEY_missing_parameters(pubkey)) {
                /* nobody has parameters? - error */
                ERR_raise(ERR_LIB_SSL, SSL_R_MISSING_PARAMETERS);
                goto out;
            } else {
                /* copy to privatekey from pubkey */
                if (!EVP_PKEY_copy_parameters(privatekey, pubkey)) {
                    ERR_raise(ERR_LIB_SSL, SSL_R_COPY_PARAMETERS_FAILED);
                    goto out;
                }
            }
        } else if (EVP_PKEY_missing_parameters(pubkey)) {
            /* copy to pubkey from privatekey */
            if (!EVP_PKEY_copy_parameters(pubkey, privatekey)) {
                ERR_raise(ERR_LIB_SSL, SSL_R_COPY_PARAMETERS_FAILED);
                goto out;
            }
        } /* else both have parameters */

        /* check that key <-> cert match */
        if (EVP_PKEY_eq(pubkey, privatekey) != 1) {
            ERR_raise(ERR_LIB_SSL, SSL_R_PRIVATE_KEY_MISMATCH);
            goto out;
        }
    }
    if (ssl_cert_lookup_by_pkey(pubkey, &i) == NULL) {
        ERR_raise(ERR_LIB_SSL, SSL_R_UNKNOWN_CERTIFICATE_TYPE);
        goto out;
    }

    if (!override && (c->pkeys[i].x509 != NULL
                      || c->pkeys[i].privatekey != NULL
                      || c->pkeys[i].chain != NULL)) {
        /* No override, and something already there */
        ERR_raise(ERR_LIB_SSL, SSL_R_NOT_REPLACING_CERTIFICATE);
        goto out;
    }

    if (chain != NULL) {
        dup_chain = X509_chain_up_ref(chain);
        if  (dup_chain == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto out;
        }
    }

    sk_X509_pop_free(c->pkeys[i].chain, X509_free);
    c->pkeys[i].chain = dup_chain;

    X509_free(c->pkeys[i].x509);
    X509_up_ref(x509);
    c->pkeys[i].x509 = x509;

    EVP_PKEY_free(c->pkeys[i].privatekey);
    EVP_PKEY_up_ref(privatekey);
    c->pkeys[i].privatekey = privatekey;

    c->key = &(c->pkeys[i]);

    ret = 1;
 out:
    EVP_PKEY_free(pubkey);
    return ret;
}

int SSL_use_cert_and_key(SSL *ssl, X509 *x509, EVP_PKEY *privatekey,
                         STACK_OF(X509) *chain, int override)
{
    return ssl_set_cert_and_key(ssl, NULL, x509, privatekey, chain, override);
}

int SSL_CTX_use_cert_and_key(SSL_CTX *ctx, X509 *x509, EVP_PKEY *privatekey,
                             STACK_OF(X509) *chain, int override)
{
    return ssl_set_cert_and_key(NULL, ctx, x509, privatekey, chain, override);
}
                       != extension_length - 4) {
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
            /*
             * File does not have a context value so we must take account of
             * this later.
             */
            contextoff = 4;
        } else {
            /* 8 byte header: 4 bytes context, 2 bytes type, 2 bytes len */
            if (extension_length < 8
                    || (extension[6] << 8) + extension[7]
                       != extension_length - 8) {
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
                       != extension_length - 8) {
                ERR_raise(ERR_LIB_SSL, SSL_R_BAD_DATA);
                goto end;
            }
        }
        /* Append the decoded extension to the serverinfo buffer */
        tmp = OPENSSL_realloc(serverinfo, serverinfo_length + extension_length
                                          + contextoff);
        if (tmp == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto end;
        }
        serverinfo = tmp;
        if (contextoff > 0) {
            unsigned char *sinfo = serverinfo + serverinfo_length;

            /* We know this only uses the last 2 bytes */
            sinfo[0] = 0;
            sinfo[1] = 0;
            sinfo[2] = (SYNTHV1CONTEXT >> 8) & 0xff;
            sinfo[3] = SYNTHV1CONTEXT & 0xff;
        }
        memcpy(serverinfo + serverinfo_length + contextoff,
               extension, extension_length);
        serverinfo_length += extension_length + contextoff;

        OPENSSL_free(name);
        name = NULL;
        OPENSSL_free(header);
        header = NULL;
        OPENSSL_free(extension);
        extension = NULL;
    }

    ret = SSL_CTX_use_serverinfo_ex(ctx, SSL_SERVERINFOV2, serverinfo,
                                    serverinfo_length);
 end:
    /* SSL_CTX_use_serverinfo makes a local copy of the serverinfo. */
    OPENSSL_free(name);
    OPENSSL_free(header);
    OPENSSL_free(extension);
    OPENSSL_free(serverinfo);
    BIO_free(bin);
    return ret;
}

static int ssl_set_cert_and_key(SSL *ssl, SSL_CTX *ctx, X509 *x509, EVP_PKEY *privatekey,
                                STACK_OF(X509) *chain, int override)
{
    int ret = 0;
    size_t i;
    int j;
    int rv;
    CERT *c = ssl != NULL ? ssl->cert : ctx->cert;
    STACK_OF(X509) *dup_chain = NULL;
    EVP_PKEY *pubkey = NULL;

    /* Do all security checks before anything else */
    rv = ssl_security_cert(ssl, ctx, x509, 0, 1);
    if (rv != 1) {
        ERR_raise(ERR_LIB_SSL, rv);
        goto out;
    }
    for (j = 0; j < sk_X509_num(chain); j++) {
        rv = ssl_security_cert(ssl, ctx, sk_X509_value(chain, j), 0, 0);
        if (rv != 1) {
            ERR_raise(ERR_LIB_SSL, rv);
            goto out;
        }
    }

    pubkey = X509_get_pubkey(x509); /* bumps reference */
    if (pubkey == NULL)
        goto out;
    if (privatekey == NULL) {
        privatekey = pubkey;
    } else {
        /* For RSA, which has no parameters, missing returns 0 */
        if (EVP_PKEY_missing_parameters(privatekey)) {
            if (EVP_PKEY_missing_parameters(pubkey)) {
                /* nobody has parameters? - error */
                ERR_raise(ERR_LIB_SSL, SSL_R_MISSING_PARAMETERS);
                goto out;
            } else {
                /* copy to privatekey from pubkey */
                if (!EVP_PKEY_copy_parameters(privatekey, pubkey)) {
                    ERR_raise(ERR_LIB_SSL, SSL_R_COPY_PARAMETERS_FAILED);
                    goto out;
                }
            }
        } else if (EVP_PKEY_missing_parameters(pubkey)) {
            /* copy to pubkey from privatekey */
            if (!EVP_PKEY_copy_parameters(pubkey, privatekey)) {
                ERR_raise(ERR_LIB_SSL, SSL_R_COPY_PARAMETERS_FAILED);
                goto out;
            }
        } /* else both have parameters */

        /* check that key <-> cert match */
        if (EVP_PKEY_eq(pubkey, privatekey) != 1) {
            ERR_raise(ERR_LIB_SSL, SSL_R_PRIVATE_KEY_MISMATCH);
            goto out;
        }
    }
    if (ssl_cert_lookup_by_pkey(pubkey, &i) == NULL) {
        ERR_raise(ERR_LIB_SSL, SSL_R_UNKNOWN_CERTIFICATE_TYPE);
        goto out;
    }

    if (!override && (c->pkeys[i].x509 != NULL
                      || c->pkeys[i].privatekey != NULL
                      || c->pkeys[i].chain != NULL)) {
        /* No override, and something already there */
        ERR_raise(ERR_LIB_SSL, SSL_R_NOT_REPLACING_CERTIFICATE);
        goto out;
    }

    if (chain != NULL) {
        dup_chain = X509_chain_up_ref(chain);
        if  (dup_chain == NULL) {
            ERR_raise(ERR_LIB_SSL, ERR_R_MALLOC_FAILURE);
            goto out;
        }
    }

    sk_X509_pop_free(c->pkeys[i].chain, X509_free);
    c->pkeys[i].chain = dup_chain;

    X509_free(c->pkeys[i].x509);
    X509_up_ref(x509);
    c->pkeys[i].x509 = x509;

    EVP_PKEY_free(c->pkeys[i].privatekey);
    EVP_PKEY_up_ref(privatekey);
    c->pkeys[i].privatekey = privatekey;

    c->key = &(c->pkeys[i]);

    ret = 1;
 out:
    EVP_PKEY_free(pubkey);
    return ret;
}

int SSL_use_cert_and_key(SSL *ssl, X509 *x509, EVP_PKEY *privatekey,
                         STACK_OF(X509) *chain, int override)
{
    return ssl_set_cert_and_key(ssl, NULL, x509, privatekey, chain, override);
}

int SSL_CTX_use_cert_and_key(SSL_CTX *ctx, X509 *x509, EVP_PKEY *privatekey,
                             STACK_OF(X509) *chain, int override)
{
    return ssl_set_cert_and_key(NULL, ctx, x509, privatekey, chain, override);
}