        if (mode == EVP_CIPH_CBC_MODE) {
            eivlen = EVP_CIPHER_CTX_get_iv_length(s->enc_write_ctx);
            if (eivlen < 0) {
                SSLfatal(s, SSL_AD_INTERNAL_ERROR, SSL_R_LIBRARY_BUG);
                return -1;
            }
            if (eivlen <= 1)
                eivlen = 0;
        }