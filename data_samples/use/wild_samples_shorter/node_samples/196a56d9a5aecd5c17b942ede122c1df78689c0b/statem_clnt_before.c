        s->session->cipher_id = s->session->cipher->id;
    if (s->hit && (s->session->cipher_id != c->id)) {
        if (SSL_IS_TLS13(s)) {
            /*
             * In TLSv1.3 it is valid for the server to select a different
             * ciphersuite as long as the hash is the same.
             */
            if (ssl_md(s->ctx, c->algorithm2)
                    != ssl_md(s->ctx, s->session->cipher->algorithm2)) {
                SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER,
                         SSL_R_CIPHERSUITE_DIGEST_HAS_CHANGED);
                return 0;
            }