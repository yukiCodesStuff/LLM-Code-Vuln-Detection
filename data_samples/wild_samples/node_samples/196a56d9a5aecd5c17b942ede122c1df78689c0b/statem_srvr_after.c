    if (!WPACKET_start_sub_packet_u16(pkt)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    return 1;
}

/*
 * Returns 1 on success, 0 to abort construction of the ticket (non-fatal), or
 * -1 on fatal error
 */
static int construct_stateless_ticket(SSL *s, WPACKET *pkt, uint32_t age_add,
                                      unsigned char *tick_nonce)
{
    unsigned char *senc = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    SSL_HMAC *hctx = NULL;
    unsigned char *p, *encdata1, *encdata2, *macdata1, *macdata2;
    const unsigned char *const_p;
    int len, slen_full, slen, lenfinal;
    SSL_SESSION *sess;
    size_t hlen;
    SSL_CTX *tctx = s->session_ctx;
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char key_name[TLSEXT_KEYNAME_LENGTH];
    int iv_len, ok = -1;
    size_t macoffset, macendoffset;

    /* get session encoding length */
    slen_full = i2d_SSL_SESSION(s->session, NULL);
    /*
     * Some length values are 16 bits, so forget it if session is too
     * long
     */
    if (slen_full == 0 || slen_full > 0xFF00) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        goto err;
    }
{
    unsigned char *senc = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    SSL_HMAC *hctx = NULL;
    unsigned char *p, *encdata1, *encdata2, *macdata1, *macdata2;
    const unsigned char *const_p;
    int len, slen_full, slen, lenfinal;
    SSL_SESSION *sess;
    size_t hlen;
    SSL_CTX *tctx = s->session_ctx;
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char key_name[TLSEXT_KEYNAME_LENGTH];
    int iv_len, ok = -1;
    size_t macoffset, macendoffset;

    /* get session encoding length */
    slen_full = i2d_SSL_SESSION(s->session, NULL);
    /*
     * Some length values are 16 bits, so forget it if session is too
     * long
     */
    if (slen_full == 0 || slen_full > 0xFF00) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        goto err;
    }
    {
        int ret = 0;

        if (tctx->ext.ticket_key_evp_cb != NULL)
            ret = tctx->ext.ticket_key_evp_cb(s, key_name, iv, ctx,
                                              ssl_hmac_get0_EVP_MAC_CTX(hctx),
                                              1);
#ifndef OPENSSL_NO_DEPRECATED_3_0
        else if (tctx->ext.ticket_key_cb != NULL)
            /* if 0 is returned, write an empty ticket */
            ret = tctx->ext.ticket_key_cb(s, key_name, iv, ctx,
                                          ssl_hmac_get0_HMAC_CTX(hctx), 1);
#endif

        if (ret == 0) {
            /*
             * In TLSv1.2 we construct a 0 length ticket. In TLSv1.3 a 0
             * length ticket is not allowed so we abort construction of the
             * ticket
             */
            if (SSL_IS_TLS13(s)) {
                ok = 0;
                goto err;
            }
            /* Put timeout and length */
            if (!WPACKET_put_bytes_u32(pkt, 0)
                    || !WPACKET_put_bytes_u16(pkt, 0)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
                goto err;
            }
            OPENSSL_free(senc);
            EVP_CIPHER_CTX_free(ctx);
            ssl_hmac_free(hctx);
            return 1;
        }
        if (ret < 0) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_CALLBACK_FAILED);
            goto err;
        }
        iv_len = EVP_CIPHER_CTX_get_iv_length(ctx);
        if (iv_len < 0) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
    } else {
        EVP_CIPHER *cipher = EVP_CIPHER_fetch(s->ctx->libctx, "AES-256-CBC",
                                              s->ctx->propq);

        if (cipher == NULL) {
            /* Error is already recorded */
            SSLfatal_alert(s, SSL_AD_INTERNAL_ERROR);
            goto err;
        }
            || !WPACKET_close(pkt)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    return 1;
}

static void tls_update_ticket_counts(SSL *s)
{
    /*
     * Increment both |sent_tickets| and |next_ticket_nonce|. |sent_tickets|
     * gets reset to 0 if we send more tickets following a post-handshake
     * auth, but |next_ticket_nonce| does not.  If we're sending extra
     * tickets, decrement the count of pending extra tickets.
     */
    s->sent_tickets++;
    s->next_ticket_nonce++;
    if (s->ext.extra_tickets_expected > 0)
        s->ext.extra_tickets_expected--;
}

int tls_construct_new_session_ticket(SSL *s, WPACKET *pkt)
{
    SSL_CTX *tctx = s->session_ctx;
    unsigned char tick_nonce[TICKET_NONCE_SIZE];
    union {
        unsigned char age_add_c[sizeof(uint32_t)];
        uint32_t age_add;
    } age_add_u;
    int ret = 0;

    age_add_u.age_add = 0;

    if (SSL_IS_TLS13(s)) {
        size_t i, hashlen;
        uint64_t nonce;
        static const unsigned char nonce_label[] = "resumption";
        const EVP_MD *md = ssl_handshake_md(s);
        int hashleni = EVP_MD_get_size(md);

        /* Ensure cast to size_t is safe */
        if (!ossl_assert(hashleni >= 0)) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        hashlen = (size_t)hashleni;

        /*
         * If we already sent one NewSessionTicket, or we resumed then
         * s->session may already be in a cache and so we must not modify it.
         * Instead we need to take a copy of it and modify that.
         */
        if (s->sent_tickets != 0 || s->hit) {
            SSL_SESSION *new_sess = ssl_session_dup(s->session, 0);

            if (new_sess == NULL) {
                /* SSLfatal already called */
                goto err;
            }

            SSL_SESSION_free(s->session);
            s->session = new_sess;
        }

        if (!ssl_generate_session_id(s, s->session)) {
            /* SSLfatal() already called */
            goto err;
        }
        if (RAND_bytes_ex(s->ctx->libctx, age_add_u.age_add_c,
                          sizeof(age_add_u), 0) <= 0) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        s->session->ext.tick_age_add = age_add_u.age_add;

        nonce = s->next_ticket_nonce;
        for (i = TICKET_NONCE_SIZE; i > 0; i--) {
            tick_nonce[i - 1] = (unsigned char)(nonce & 0xff);
            nonce >>= 8;
        }

        if (!tls13_hkdf_expand(s, md, s->resumption_master_secret,
                               nonce_label,
                               sizeof(nonce_label) - 1,
                               tick_nonce,
                               TICKET_NONCE_SIZE,
                               s->session->master_key,
                               hashlen, 1)) {
            /* SSLfatal() already called */
            goto err;
        }
        s->session->master_key_length = hashlen;

        s->session->time = time(NULL);
        ssl_session_calculate_timeout(s->session);
        if (s->s3.alpn_selected != NULL) {
            OPENSSL_free(s->session->ext.alpn_selected);
            s->session->ext.alpn_selected =
                OPENSSL_memdup(s->s3.alpn_selected, s->s3.alpn_selected_len);
            if (s->session->ext.alpn_selected == NULL) {
                s->session->ext.alpn_selected_len = 0;
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_MALLOC_FAILURE);
                goto err;
            }
            s->session->ext.alpn_selected_len = s->s3.alpn_selected_len;
        }
        s->session->ext.max_early_data = s->max_early_data;
    }

    if (tctx->generate_ticket_cb != NULL &&
        tctx->generate_ticket_cb(s, tctx->ticket_cb_data) == 0) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        goto err;
    }
    /*
     * If we are using anti-replay protection then we behave as if
     * SSL_OP_NO_TICKET is set - we are caching tickets anyway so there
     * is no point in using full stateless tickets.
     */
    if (SSL_IS_TLS13(s)
            && ((s->options & SSL_OP_NO_TICKET) != 0
                || (s->max_early_data > 0
                    && (s->options & SSL_OP_NO_ANTI_REPLAY) == 0))) {
        if (!construct_stateful_ticket(s, pkt, age_add_u.age_add, tick_nonce)) {
            /* SSLfatal() already called */
            goto err;
        }
    } else {
        int tmpret;

        tmpret = construct_stateless_ticket(s, pkt, age_add_u.age_add,
                                            tick_nonce);
        if (tmpret != 1) {
            if (tmpret == 0) {
                ret = 2; /* Non-fatal. Abort construction but continue */
                /* We count this as a success so update the counts anwyay */
                tls_update_ticket_counts(s);
            }
            /* else SSLfatal() already called */
            goto err;
        }
    }

    if (SSL_IS_TLS13(s)) {
        if (!tls_construct_extensions(s, pkt,
                                      SSL_EXT_TLS1_3_NEW_SESSION_TICKET,
                                      NULL, 0)) {
            /* SSLfatal() already called */
            goto err;
        }
        tls_update_ticket_counts(s);
        ssl_update_cache(s, SSL_SESS_CACHE_SERVER);
    }

    ret = 1;
 err:
    return ret;
}

/*
 * In TLSv1.3 this is called from the extensions code, otherwise it is used to
 * create a separate message. Returns 1 on success or 0 on failure.
 */
int tls_construct_cert_status_body(SSL *s, WPACKET *pkt)
{
    if (!WPACKET_put_bytes_u8(pkt, s->ext.status_type)
            || !WPACKET_sub_memcpy_u24(pkt, s->ext.ocsp.resp,
                                       s->ext.ocsp.resp_len)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    return 1;
}

int tls_construct_cert_status(SSL *s, WPACKET *pkt)
{
    if (!tls_construct_cert_status_body(s, pkt)) {
        /* SSLfatal() already called */
        return 0;
    }

    return 1;
}

#ifndef OPENSSL_NO_NEXTPROTONEG
/*
 * tls_process_next_proto reads a Next Protocol Negotiation handshake message.
 * It sets the next_proto member in s if found
 */
MSG_PROCESS_RETURN tls_process_next_proto(SSL *s, PACKET *pkt)
{
    PACKET next_proto, padding;
    size_t next_proto_len;

    /*-
     * The payload looks like:
     *   uint8 proto_len;
     *   uint8 proto[proto_len];
     *   uint8 padding_len;
     *   uint8 padding[padding_len];
     */
    if (!PACKET_get_length_prefixed_1(pkt, &next_proto)
        || !PACKET_get_length_prefixed_1(pkt, &padding)
        || PACKET_remaining(pkt) > 0) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return MSG_PROCESS_ERROR;
    }

    if (!PACKET_memdup(&next_proto, &s->ext.npn, &next_proto_len)) {
        s->ext.npn_len = 0;
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return MSG_PROCESS_ERROR;
    }

    s->ext.npn_len = (unsigned char)next_proto_len;

    return MSG_PROCESS_CONTINUE_READING;
}
#endif

static int tls_construct_encrypted_extensions(SSL *s, WPACKET *pkt)
{
    if (!tls_construct_extensions(s, pkt, SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS,
                                  NULL, 0)) {
        /* SSLfatal() already called */
        return 0;
    }

    return 1;
}

MSG_PROCESS_RETURN tls_process_end_of_early_data(SSL *s, PACKET *pkt)
{
    if (PACKET_remaining(pkt) != 0) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return MSG_PROCESS_ERROR;
    }

    if (s->early_data_state != SSL_EARLY_DATA_READING
            && s->early_data_state != SSL_EARLY_DATA_READ_RETRY) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return MSG_PROCESS_ERROR;
    }

    /*
     * EndOfEarlyData signals a key change so the end of the message must be on
     * a record boundary.
     */
    if (RECORD_LAYER_processed_read_pending(&s->rlayer)) {
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_NOT_ON_RECORD_BOUNDARY);
        return MSG_PROCESS_ERROR;
    }

    s->early_data_state = SSL_EARLY_DATA_FINISHED_READING;
    if (!s->method->ssl3_enc->change_cipher_state(s,
                SSL3_CC_HANDSHAKE | SSL3_CHANGE_CIPHER_SERVER_READ)) {
        /* SSLfatal() already called */
        return MSG_PROCESS_ERROR;
    }

    return MSG_PROCESS_CONTINUE_READING;
}
    union {
        unsigned char age_add_c[sizeof(uint32_t)];
        uint32_t age_add;
    } age_add_u;
    int ret = 0;

    age_add_u.age_add = 0;

    if (SSL_IS_TLS13(s)) {
        size_t i, hashlen;
        uint64_t nonce;
        static const unsigned char nonce_label[] = "resumption";
        const EVP_MD *md = ssl_handshake_md(s);
        int hashleni = EVP_MD_get_size(md);

        /* Ensure cast to size_t is safe */
        if (!ossl_assert(hashleni >= 0)) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        hashlen = (size_t)hashleni;

        /*
         * If we already sent one NewSessionTicket, or we resumed then
         * s->session may already be in a cache and so we must not modify it.
         * Instead we need to take a copy of it and modify that.
         */
        if (s->sent_tickets != 0 || s->hit) {
            SSL_SESSION *new_sess = ssl_session_dup(s->session, 0);

            if (new_sess == NULL) {
                /* SSLfatal already called */
                goto err;
            }

            SSL_SESSION_free(s->session);
            s->session = new_sess;
        }

        if (!ssl_generate_session_id(s, s->session)) {
            /* SSLfatal() already called */
            goto err;
        }
        if (RAND_bytes_ex(s->ctx->libctx, age_add_u.age_add_c,
                          sizeof(age_add_u), 0) <= 0) {
            SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        s->session->ext.tick_age_add = age_add_u.age_add;

        nonce = s->next_ticket_nonce;
        for (i = TICKET_NONCE_SIZE; i > 0; i--) {
            tick_nonce[i - 1] = (unsigned char)(nonce & 0xff);
            nonce >>= 8;
        }

        if (!tls13_hkdf_expand(s, md, s->resumption_master_secret,
                               nonce_label,
                               sizeof(nonce_label) - 1,
                               tick_nonce,
                               TICKET_NONCE_SIZE,
                               s->session->master_key,
                               hashlen, 1)) {
            /* SSLfatal() already called */
            goto err;
        }
        s->session->master_key_length = hashlen;

        s->session->time = time(NULL);
        ssl_session_calculate_timeout(s->session);
        if (s->s3.alpn_selected != NULL) {
            OPENSSL_free(s->session->ext.alpn_selected);
            s->session->ext.alpn_selected =
                OPENSSL_memdup(s->s3.alpn_selected, s->s3.alpn_selected_len);
            if (s->session->ext.alpn_selected == NULL) {
                s->session->ext.alpn_selected_len = 0;
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_MALLOC_FAILURE);
                goto err;
            }
            s->session->ext.alpn_selected_len = s->s3.alpn_selected_len;
        }
        s->session->ext.max_early_data = s->max_early_data;
    }
        if (!construct_stateful_ticket(s, pkt, age_add_u.age_add, tick_nonce)) {
            /* SSLfatal() already called */
            goto err;
        }
    } else {
        int tmpret;

        tmpret = construct_stateless_ticket(s, pkt, age_add_u.age_add,
                                            tick_nonce);
        if (tmpret != 1) {
            if (tmpret == 0) {
                ret = 2; /* Non-fatal. Abort construction but continue */
                /* We count this as a success so update the counts anwyay */
                tls_update_ticket_counts(s);
            }
            /* else SSLfatal() already called */
            goto err;
        }
                                      NULL, 0)) {
            /* SSLfatal() already called */
            goto err;
        }
        tls_update_ticket_counts(s);
        ssl_update_cache(s, SSL_SESS_CACHE_SERVER);
    }

    ret = 1;
 err:
    return ret;
}

/*
 * In TLSv1.3 this is called from the extensions code, otherwise it is used to
 * create a separate message. Returns 1 on success or 0 on failure.
 */
int tls_construct_cert_status_body(SSL *s, WPACKET *pkt)
{
    if (!WPACKET_put_bytes_u8(pkt, s->ext.status_type)
            || !WPACKET_sub_memcpy_u24(pkt, s->ext.ocsp.resp,
                                       s->ext.ocsp.resp_len)) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    return 1;
}

int tls_construct_cert_status(SSL *s, WPACKET *pkt)
{
    if (!tls_construct_cert_status_body(s, pkt)) {
        /* SSLfatal() already called */
        return 0;
    }

    return 1;
}

#ifndef OPENSSL_NO_NEXTPROTONEG
/*
 * tls_process_next_proto reads a Next Protocol Negotiation handshake message.
 * It sets the next_proto member in s if found
 */
MSG_PROCESS_RETURN tls_process_next_proto(SSL *s, PACKET *pkt)
{
    PACKET next_proto, padding;
    size_t next_proto_len;

    /*-
     * The payload looks like:
     *   uint8 proto_len;
     *   uint8 proto[proto_len];
     *   uint8 padding_len;
     *   uint8 padding[padding_len];
     */
    if (!PACKET_get_length_prefixed_1(pkt, &next_proto)
        || !PACKET_get_length_prefixed_1(pkt, &padding)
        || PACKET_remaining(pkt) > 0) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return MSG_PROCESS_ERROR;
    }

    if (!PACKET_memdup(&next_proto, &s->ext.npn, &next_proto_len)) {
        s->ext.npn_len = 0;
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return MSG_PROCESS_ERROR;
    }

    s->ext.npn_len = (unsigned char)next_proto_len;

    return MSG_PROCESS_CONTINUE_READING;
}
#endif

static int tls_construct_encrypted_extensions(SSL *s, WPACKET *pkt)
{
    if (!tls_construct_extensions(s, pkt, SSL_EXT_TLS1_3_ENCRYPTED_EXTENSIONS,
                                  NULL, 0)) {
        /* SSLfatal() already called */
        return 0;
    }

    return 1;
}

MSG_PROCESS_RETURN tls_process_end_of_early_data(SSL *s, PACKET *pkt)
{
    if (PACKET_remaining(pkt) != 0) {
        SSLfatal(s, SSL_AD_DECODE_ERROR, SSL_R_LENGTH_MISMATCH);
        return MSG_PROCESS_ERROR;
    }

    if (s->early_data_state != SSL_EARLY_DATA_READING
            && s->early_data_state != SSL_EARLY_DATA_READ_RETRY) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return MSG_PROCESS_ERROR;
    }

    /*
     * EndOfEarlyData signals a key change so the end of the message must be on
     * a record boundary.
     */
    if (RECORD_LAYER_processed_read_pending(&s->rlayer)) {
        SSLfatal(s, SSL_AD_UNEXPECTED_MESSAGE, SSL_R_NOT_ON_RECORD_BOUNDARY);
        return MSG_PROCESS_ERROR;
    }

    s->early_data_state = SSL_EARLY_DATA_FINISHED_READING;
    if (!s->method->ssl3_enc->change_cipher_state(s,
                SSL3_CC_HANDSHAKE | SSL3_CHANGE_CIPHER_SERVER_READ)) {
        /* SSLfatal() already called */
        return MSG_PROCESS_ERROR;
    }

    return MSG_PROCESS_CONTINUE_READING;
}