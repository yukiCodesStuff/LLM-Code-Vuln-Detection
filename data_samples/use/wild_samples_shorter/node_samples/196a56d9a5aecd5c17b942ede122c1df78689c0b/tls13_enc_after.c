size_t tls13_final_finish_mac(SSL *s, const char *str, size_t slen,
                             unsigned char *out)
{
    const EVP_MD *md = ssl_handshake_md(s);
    const char *mdname = EVP_MD_get0_name(md);
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned char finsecret[EVP_MAX_MD_SIZE];
    unsigned char *key = NULL;
    size_t len = 0, hashlen;
    OSSL_PARAM params[2], *p = params;

    if (md == NULL)
        return 0;

    /* Safe to cast away const here since we're not "getting" any data */
    if (s->ctx->propq != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ALG_PARAM_PROPERTIES,
                                                (char *)s->ctx->propq,
    } else if (SSL_IS_FIRST_HANDSHAKE(s)) {
        key = s->client_finished_secret;
    } else {
        if (!tls13_derive_finishedkey(s, md,
                                      s->client_app_traffic_secret,
                                      finsecret, hashlen))
            goto err;
        key = finsecret;
  static const unsigned char application_traffic[] = "traffic upd";
#endif
    const EVP_MD *md = ssl_handshake_md(s);
    size_t hashlen;
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char *insecret, *iv;
    unsigned char secret[EVP_MAX_MD_SIZE];
    char *log_label;
    EVP_CIPHER_CTX *ciph_ctx;
    int ret = 0, l;

    if ((l = EVP_MD_get_size(md)) <= 0) {
        SSLfatal(s, SSL_AD_INTERNAL_ERROR, ERR_R_INTERNAL_ERROR);
        return 0;
    }
    hashlen = (size_t)l;

    if (s->server == sending)
        insecret = s->server_app_traffic_secret;
    else
        RECORD_LAYER_reset_read_sequence(&s->rlayer);
    }

    if (!derive_secret_key_and_iv(s, sending, md,
                                  s->s3.tmp.new_sym_enc, insecret, NULL,
                                  application_traffic,
                                  sizeof(application_traffic) - 1, secret, key,
                                  iv, ciph_ctx)) {

    memcpy(insecret, secret, hashlen);

    /* Call Key log on successful traffic secret update */
    log_label = s->server == sending ? SERVER_APPLICATION_N_LABEL : CLIENT_APPLICATION_N_LABEL;
    if (!ssl_log_secret(s, log_label, secret, hashlen)) {
        /* SSLfatal() already called */
        goto err;
    }

    s->statem.enc_write_state = ENC_WRITE_STATE_VALID;
    ret = 1;
 err:
    OPENSSL_cleanse(key, sizeof(key));
    unsigned int hashsize, datalen;
    int ret = 0;

    if (ctx == NULL || md == NULL || !ossl_statem_export_allowed(s))
        goto err;

    if (!use_context)
        contextlen = 0;
     *
     * Here Transcript-Hash is the cipher suite hash algorithm.
     */
    if (md == NULL
            || EVP_DigestInit_ex(ctx, md, NULL) <= 0
            || EVP_DigestUpdate(ctx, context, contextlen) <= 0
            || EVP_DigestFinal_ex(ctx, hash, &hashsize) <= 0
            || EVP_DigestInit_ex(ctx, md, NULL) <= 0
            || EVP_DigestFinal_ex(ctx, data, &datalen) <= 0