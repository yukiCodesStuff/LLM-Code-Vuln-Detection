    return 1;
}

static int construct_stateless_ticket(SSL *s, WPACKET *pkt, uint32_t age_add,
                                      unsigned char *tick_nonce)
{
    unsigned char *senc = NULL;
    SSL_CTX *tctx = s->session_ctx;
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned char key_name[TLSEXT_KEYNAME_LENGTH];
    int iv_len, ok = 0;
    size_t macoffset, macendoffset;

    /* get session encoding length */
    slen_full = i2d_SSL_SESSION(s->session, NULL);
#endif

        if (ret == 0) {

            /* Put timeout and length */
            if (!WPACKET_put_bytes_u32(pkt, 0)
                    || !WPACKET_put_bytes_u16(pkt, 0)) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
            goto err;
        }
        iv_len = EVP_CIPHER_CTX_get_iv_length(ctx);
    } else {
        EVP_CIPHER *cipher = EVP_CIPHER_fetch(s->ctx->libctx, "AES-256-CBC",
                                              s->ctx->propq);

    return 1;
}

int tls_construct_new_session_ticket(SSL *s, WPACKET *pkt)
{
    SSL_CTX *tctx = s->session_ctx;
    unsigned char tick_nonce[TICKET_NONCE_SIZE];
        unsigned char age_add_c[sizeof(uint32_t)];
        uint32_t age_add;
    } age_add_u;

    age_add_u.age_add = 0;

    if (SSL_IS_TLS13(s)) {
            /* SSLfatal() already called */
            goto err;
        }
    } else if (!construct_stateless_ticket(s, pkt, age_add_u.age_add,
                                           tick_nonce)) {
        /* SSLfatal() already called */
        goto err;
    }

    if (SSL_IS_TLS13(s)) {
        if (!tls_construct_extensions(s, pkt,
            /* SSLfatal() already called */
            goto err;
        }
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
        ssl_update_cache(s, SSL_SESS_CACHE_SERVER);
    }

    return 1;
 err:
    return 0;
}

/*
 * In TLSv1.3 this is called from the extensions code, otherwise it is used to