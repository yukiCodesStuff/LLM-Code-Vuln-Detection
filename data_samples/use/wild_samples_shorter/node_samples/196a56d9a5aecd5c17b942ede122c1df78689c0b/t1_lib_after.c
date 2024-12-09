    SSL_SESSION *sess = NULL;
    unsigned char *sdec;
    const unsigned char *p;
    int slen, ivlen, renew_ticket = 0, declen;
    SSL_TICKET_STATUS ret = SSL_TICKET_FATAL_ERR_OTHER;
    size_t mlen;
    unsigned char tick_hmac[EVP_MAX_MD_SIZE];
    SSL_HMAC *hctx = NULL;
        goto end;
    }

    ivlen = EVP_CIPHER_CTX_get_iv_length(ctx);
    if (ivlen < 0) {
        ret = SSL_TICKET_FATAL_ERR_OTHER;
        goto end;
    }

    /* Sanity check ticket length: must exceed keyname + IV + HMAC */
    if (eticklen <= TLSEXT_KEYNAME_LENGTH + ivlen + mlen) {
        ret = SSL_TICKET_NO_DECRYPT;
        goto end;
    }
    eticklen -= mlen;
    }
    /* Attempt to decrypt session data */
    /* Move p after IV to start of encrypted ticket, update length */
    p = etick + TLSEXT_KEYNAME_LENGTH + ivlen;
    eticklen -= TLSEXT_KEYNAME_LENGTH + ivlen;
    sdec = OPENSSL_malloc(eticklen);
    if (sdec == NULL || EVP_DecryptUpdate(ctx, sdec, &slen, p,
                                          (int)eticklen) <= 0) {
        OPENSSL_free(sdec);