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
        } else {
            /*
             * Prior to TLSv1.3 resuming a session always meant using the same
             * ciphersuite.
             */
            SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER,
                     SSL_R_OLD_SESSION_CIPHER_NOT_RETURNED);
            return 0;
        }
    }
    s->s3.tmp.new_cipher = c;

    return 1;
}