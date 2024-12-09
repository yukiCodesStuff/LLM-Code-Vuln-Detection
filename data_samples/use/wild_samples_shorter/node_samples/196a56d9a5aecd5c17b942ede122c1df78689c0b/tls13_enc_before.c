size_t tls13_final_finish_mac(SSL *s, const char *str, size_t slen,
                             unsigned char *out)
{
    const char *mdname = EVP_MD_get0_name(ssl_handshake_md(s));
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned char finsecret[EVP_MAX_MD_SIZE];
    unsigned char *key = NULL;
    size_t len = 0, hashlen;
    OSSL_PARAM params[2], *p = params;

    /* Safe to cast away const here since we're not "getting" any data */
    if (s->ctx->propq != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ALG_PARAM_PROPERTIES,
                                                (char *)s->ctx->propq,
    } else if (SSL_IS_FIRST_HANDSHAKE(s)) {
        key = s->client_finished_secret;
    } else {
        if (!tls13_derive_finishedkey(s, ssl_handshake_md(s),
                                      s->client_app_traffic_secret,
                                      finsecret, hashlen))
            goto err;
        key = finsecret;
  static const unsigned char application_traffic[] = "traffic upd";
#endif
    const EVP_MD *md = ssl_handshake_md(s);
    size_t hashlen = EVP_MD_get_size(md);
    unsigned char key[EVP_MAX_KEY_LENGTH];
    unsigned char *insecret, *iv;
    unsigned char secret[EVP_MAX_MD_SIZE];
    EVP_CIPHER_CTX *ciph_ctx;
    int ret = 0;

    if (s->server == sending)
        insecret = s->server_app_traffic_secret;
    else
        RECORD_LAYER_reset_read_sequence(&s->rlayer);
    }

    if (!derive_secret_key_and_iv(s, sending, ssl_handshake_md(s),
                                  s->s3.tmp.new_sym_enc, insecret, NULL,
                                  application_traffic,
                                  sizeof(application_traffic) - 1, secret, key,
                                  iv, ciph_ctx)) {

    memcpy(insecret, secret, hashlen);

    s->statem.enc_write_state = ENC_WRITE_STATE_VALID;
    ret = 1;
 err:
    OPENSSL_cleanse(key, sizeof(key));
    unsigned int hashsize, datalen;
    int ret = 0;

    if (ctx == NULL || !ossl_statem_export_allowed(s))
        goto err;

    if (!use_context)
        contextlen = 0;
     *
     * Here Transcript-Hash is the cipher suite hash algorithm.
     */
    if (EVP_DigestInit_ex(ctx, md, NULL) <= 0
            || EVP_DigestUpdate(ctx, context, contextlen) <= 0
            || EVP_DigestFinal_ex(ctx, hash, &hashsize) <= 0
            || EVP_DigestInit_ex(ctx, md, NULL) <= 0
            || EVP_DigestFinal_ex(ctx, data, &datalen) <= 0