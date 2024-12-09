{
    SSL_SESSION *sess = NULL;
    unsigned char *sdec;
    const unsigned char *p;
    int slen, renew_ticket = 0, declen;
    SSL_TICKET_STATUS ret = SSL_TICKET_FATAL_ERR_OTHER;
    size_t mlen;
    unsigned char tick_hmac[EVP_MAX_MD_SIZE];
    SSL_HMAC *hctx = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    SSL_CTX *tctx = s->session_ctx;

    if (eticklen == 0) {
        /*
         * The client will accept a ticket but doesn't currently have
         * one (TLSv1.2 and below), or treated as a fatal error in TLSv1.3
         */
        ret = SSL_TICKET_EMPTY;
        goto end;
    }
    if (mlen == 0) {
        ret = SSL_TICKET_FATAL_ERR_OTHER;
        goto end;
    }

    /* Sanity check ticket length: must exceed keyname + IV + HMAC */
    if (eticklen <=
        TLSEXT_KEYNAME_LENGTH + EVP_CIPHER_CTX_get_iv_length(ctx) + mlen) {
        ret = SSL_TICKET_NO_DECRYPT;
        goto end;
    }
    eticklen -= mlen;
    /* Check HMAC of encrypted ticket */
    if (ssl_hmac_update(hctx, etick, eticklen) <= 0
        || ssl_hmac_final(hctx, tick_hmac, NULL, sizeof(tick_hmac)) <= 0) {
        ret = SSL_TICKET_FATAL_ERR_OTHER;
        goto end;
    }

    if (CRYPTO_memcmp(tick_hmac, etick + eticklen, mlen)) {
        ret = SSL_TICKET_NO_DECRYPT;
        goto end;
    }
    /* Attempt to decrypt session data */
    /* Move p after IV to start of encrypted ticket, update length */
    p = etick + TLSEXT_KEYNAME_LENGTH + EVP_CIPHER_CTX_get_iv_length(ctx);
    eticklen -= TLSEXT_KEYNAME_LENGTH + EVP_CIPHER_CTX_get_iv_length(ctx);
    sdec = OPENSSL_malloc(eticklen);
    if (sdec == NULL || EVP_DecryptUpdate(ctx, sdec, &slen, p,
                                          (int)eticklen) <= 0) {
        OPENSSL_free(sdec);
        ret = SSL_TICKET_FATAL_ERR_OTHER;
        goto end;
    }
    if (EVP_DecryptFinal(ctx, sdec + slen, &declen) <= 0) {
        OPENSSL_free(sdec);
        ret = SSL_TICKET_NO_DECRYPT;
        goto end;
    }
    slen += declen;
    p = sdec;

    sess = d2i_SSL_SESSION(NULL, &p, slen);
    slen -= p - sdec;
    OPENSSL_free(sdec);
    if (sess) {
        /* Some additional consistency checks */
        if (slen != 0) {
            SSL_SESSION_free(sess);
            sess = NULL;
            ret = SSL_TICKET_NO_DECRYPT;
            goto end;
        }
    if (CRYPTO_memcmp(tick_hmac, etick + eticklen, mlen)) {
        ret = SSL_TICKET_NO_DECRYPT;
        goto end;
    }
    /* Attempt to decrypt session data */
    /* Move p after IV to start of encrypted ticket, update length */
    p = etick + TLSEXT_KEYNAME_LENGTH + EVP_CIPHER_CTX_get_iv_length(ctx);
    eticklen -= TLSEXT_KEYNAME_LENGTH + EVP_CIPHER_CTX_get_iv_length(ctx);
    sdec = OPENSSL_malloc(eticklen);
    if (sdec == NULL || EVP_DecryptUpdate(ctx, sdec, &slen, p,
                                          (int)eticklen) <= 0) {
        OPENSSL_free(sdec);
        ret = SSL_TICKET_FATAL_ERR_OTHER;
        goto end;
    }