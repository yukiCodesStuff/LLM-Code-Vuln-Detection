{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey))
        return 0;

    if (key != NULL)
        return s390x_x25519_mul(key, pubkey, privkey);

    *keylen = X25519_KEYLEN;
    return 1;
}

static int s390x_pkey_ecx_derive448(EVP_PKEY_CTX *ctx, unsigned char *key,
                                      size_t *keylen)
{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey))
        return 0;

    if (key != NULL)
        return s390x_x448_mul(key, pubkey, privkey);

    *keylen = X448_KEYLEN;
    return 1;
}

static int s390x_pkey_ecd_digestsign25519(EVP_MD_CTX *ctx,
                                          unsigned char *sig, size_t *siglen,
                                          const unsigned char *tbs,
                                          size_t tbslen)
{
    union {
        struct {
            unsigned char sig[64];
            unsigned char priv[32];
        } ed25519;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);
    int rc;

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (sig == NULL) {
        *siglen = ED25519_SIGSIZE;
        return 1;
    }

    if (*siglen < ED25519_SIGSIZE) {
        ERR_raise(ERR_LIB_EC, EC_R_BUFFER_TOO_SMALL);
        return 0;
    }

    memset(&param, 0, sizeof(param));
    memcpy(param.ed25519.priv, edkey->privkey, sizeof(param.ed25519.priv));

    rc = s390x_kdsa(S390X_EDDSA_SIGN_ED25519, &param.ed25519, tbs, tbslen);
    OPENSSL_cleanse(param.ed25519.priv, sizeof(param.ed25519.priv));
    if (rc != 0)
        return 0;

    s390x_flip_endian32(sig, param.ed25519.sig);
    s390x_flip_endian32(sig + 32, param.ed25519.sig + 32);

    *siglen = ED25519_SIGSIZE;
    return 1;
}

static int s390x_pkey_ecd_digestsign448(EVP_MD_CTX *ctx,
                                        unsigned char *sig, size_t *siglen,
                                        const unsigned char *tbs,
                                        size_t tbslen)
{
    union {
        struct {
            unsigned char sig[128];
            unsigned char priv[64];
        } ed448;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);
    int rc;

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (sig == NULL) {
        *siglen = ED448_SIGSIZE;
        return 1;
    }

    if (*siglen < ED448_SIGSIZE) {
        ERR_raise(ERR_LIB_EC, EC_R_BUFFER_TOO_SMALL);
        return 0;
    }

    memset(&param, 0, sizeof(param));
    memcpy(param.ed448.priv + 64 - 57, edkey->privkey, 57);

    rc = s390x_kdsa(S390X_EDDSA_SIGN_ED448, &param.ed448, tbs, tbslen);
    OPENSSL_cleanse(param.ed448.priv, sizeof(param.ed448.priv));
    if (rc != 0)
        return 0;

    s390x_flip_endian64(param.ed448.sig, param.ed448.sig);
    s390x_flip_endian64(param.ed448.sig + 64, param.ed448.sig + 64);
    memcpy(sig, param.ed448.sig, 57);
    memcpy(sig + 57, param.ed448.sig + 64, 57);

    *siglen = ED448_SIGSIZE;
    return 1;
}

static int s390x_pkey_ecd_digestverify25519(EVP_MD_CTX *ctx,
                                            const unsigned char *sig,
                                            size_t siglen,
                                            const unsigned char *tbs,
                                            size_t tbslen)
{
    union {
        struct {
            unsigned char sig[64];
            unsigned char pub[32];
        } ed25519;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (siglen != ED25519_SIGSIZE)
        return 0;

    memset(&param, 0, sizeof(param));
    s390x_flip_endian32(param.ed25519.sig, sig);
    s390x_flip_endian32(param.ed25519.sig + 32, sig + 32);
    s390x_flip_endian32(param.ed25519.pub, edkey->pubkey);

    return s390x_kdsa(S390X_EDDSA_VERIFY_ED25519,
                      &param.ed25519, tbs, tbslen) == 0 ? 1 : 0;
}

static int s390x_pkey_ecd_digestverify448(EVP_MD_CTX *ctx,
                                          const unsigned char *sig,
                                          size_t siglen,
                                          const unsigned char *tbs,
                                          size_t tbslen)
{
    union {
        struct {
            unsigned char sig[128];
            unsigned char pub[64];
        } ed448;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (siglen != ED448_SIGSIZE)
        return 0;

    memset(&param, 0, sizeof(param));
    memcpy(param.ed448.sig, sig, 57);
    s390x_flip_endian64(param.ed448.sig, param.ed448.sig);
    memcpy(param.ed448.sig + 64, sig + 57, 57);
    s390x_flip_endian64(param.ed448.sig + 64, param.ed448.sig + 64);
    memcpy(param.ed448.pub, edkey->pubkey, 57);
    s390x_flip_endian64(param.ed448.pub, param.ed448.pub);

    return s390x_kdsa(S390X_EDDSA_VERIFY_ED448,
                      &param.ed448, tbs, tbslen) == 0 ? 1 : 0;
}

static const EVP_PKEY_METHOD ecx25519_s390x_pkey_meth = {
    EVP_PKEY_X25519,
    0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_keygen25519,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_derive25519,
    pkey_ecx_ctrl,
    0
};

static const EVP_PKEY_METHOD ecx448_s390x_pkey_meth = {
    EVP_PKEY_X448,
    0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_keygen448,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_derive448,
    pkey_ecx_ctrl,
    0
};
static const EVP_PKEY_METHOD ed25519_s390x_pkey_meth = {
    EVP_PKEY_ED25519, EVP_PKEY_FLAG_SIGCTX_CUSTOM,
    0, 0, 0, 0, 0, 0,
    s390x_pkey_ecd_keygen25519,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    pkey_ecd_ctrl,
    0,
    s390x_pkey_ecd_digestsign25519,
    s390x_pkey_ecd_digestverify25519
};

static const EVP_PKEY_METHOD ed448_s390x_pkey_meth = {
    EVP_PKEY_ED448, EVP_PKEY_FLAG_SIGCTX_CUSTOM,
    0, 0, 0, 0, 0, 0,
    s390x_pkey_ecd_keygen448,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    pkey_ecd_ctrl,
    0,
    s390x_pkey_ecd_digestsign448,
    s390x_pkey_ecd_digestverify448
};
#endif

const EVP_PKEY_METHOD *ossl_ecx25519_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_X25519))
        return &ecx25519_s390x_pkey_meth;
#endif
    return &ecx25519_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ecx448_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_X448))
        return &ecx448_s390x_pkey_meth;
#endif
    return &ecx448_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ed25519_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_ED25519)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_SIGN_ED25519)
        && OPENSSL_s390xcap_P.kdsa[0]
            & S390X_CAPBIT(S390X_EDDSA_VERIFY_ED25519))
        return &ed25519_s390x_pkey_meth;
#endif
    return &ed25519_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ed448_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_ED448)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_SIGN_ED448)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_VERIFY_ED448))
        return &ed448_s390x_pkey_meth;
#endif
    return &ed448_pkey_meth;
}
{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey))
        return 0;

    if (key != NULL)
        return s390x_x448_mul(key, pubkey, privkey);

    *keylen = X448_KEYLEN;
    return 1;
}

static int s390x_pkey_ecd_digestsign25519(EVP_MD_CTX *ctx,
                                          unsigned char *sig, size_t *siglen,
                                          const unsigned char *tbs,
                                          size_t tbslen)
{
    union {
        struct {
            unsigned char sig[64];
            unsigned char priv[32];
        } ed25519;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);
    int rc;

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (sig == NULL) {
        *siglen = ED25519_SIGSIZE;
        return 1;
    }

    if (*siglen < ED25519_SIGSIZE) {
        ERR_raise(ERR_LIB_EC, EC_R_BUFFER_TOO_SMALL);
        return 0;
    }

    memset(&param, 0, sizeof(param));
    memcpy(param.ed25519.priv, edkey->privkey, sizeof(param.ed25519.priv));

    rc = s390x_kdsa(S390X_EDDSA_SIGN_ED25519, &param.ed25519, tbs, tbslen);
    OPENSSL_cleanse(param.ed25519.priv, sizeof(param.ed25519.priv));
    if (rc != 0)
        return 0;

    s390x_flip_endian32(sig, param.ed25519.sig);
    s390x_flip_endian32(sig + 32, param.ed25519.sig + 32);

    *siglen = ED25519_SIGSIZE;
    return 1;
}

static int s390x_pkey_ecd_digestsign448(EVP_MD_CTX *ctx,
                                        unsigned char *sig, size_t *siglen,
                                        const unsigned char *tbs,
                                        size_t tbslen)
{
    union {
        struct {
            unsigned char sig[128];
            unsigned char priv[64];
        } ed448;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);
    int rc;

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (sig == NULL) {
        *siglen = ED448_SIGSIZE;
        return 1;
    }

    if (*siglen < ED448_SIGSIZE) {
        ERR_raise(ERR_LIB_EC, EC_R_BUFFER_TOO_SMALL);
        return 0;
    }

    memset(&param, 0, sizeof(param));
    memcpy(param.ed448.priv + 64 - 57, edkey->privkey, 57);

    rc = s390x_kdsa(S390X_EDDSA_SIGN_ED448, &param.ed448, tbs, tbslen);
    OPENSSL_cleanse(param.ed448.priv, sizeof(param.ed448.priv));
    if (rc != 0)
        return 0;

    s390x_flip_endian64(param.ed448.sig, param.ed448.sig);
    s390x_flip_endian64(param.ed448.sig + 64, param.ed448.sig + 64);
    memcpy(sig, param.ed448.sig, 57);
    memcpy(sig + 57, param.ed448.sig + 64, 57);

    *siglen = ED448_SIGSIZE;
    return 1;
}

static int s390x_pkey_ecd_digestverify25519(EVP_MD_CTX *ctx,
                                            const unsigned char *sig,
                                            size_t siglen,
                                            const unsigned char *tbs,
                                            size_t tbslen)
{
    union {
        struct {
            unsigned char sig[64];
            unsigned char pub[32];
        } ed25519;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (siglen != ED25519_SIGSIZE)
        return 0;

    memset(&param, 0, sizeof(param));
    s390x_flip_endian32(param.ed25519.sig, sig);
    s390x_flip_endian32(param.ed25519.sig + 32, sig + 32);
    s390x_flip_endian32(param.ed25519.pub, edkey->pubkey);

    return s390x_kdsa(S390X_EDDSA_VERIFY_ED25519,
                      &param.ed25519, tbs, tbslen) == 0 ? 1 : 0;
}

static int s390x_pkey_ecd_digestverify448(EVP_MD_CTX *ctx,
                                          const unsigned char *sig,
                                          size_t siglen,
                                          const unsigned char *tbs,
                                          size_t tbslen)
{
    union {
        struct {
            unsigned char sig[128];
            unsigned char pub[64];
        } ed448;
        unsigned long long buff[512];
    } param;
    const ECX_KEY *edkey = evp_pkey_get_legacy(EVP_MD_CTX_get_pkey_ctx(ctx)->pkey);

    if (edkey == NULL) {
        ERR_raise(ERR_LIB_EC, EC_R_INVALID_KEY);
        return 0;
    }

    if (siglen != ED448_SIGSIZE)
        return 0;

    memset(&param, 0, sizeof(param));
    memcpy(param.ed448.sig, sig, 57);
    s390x_flip_endian64(param.ed448.sig, param.ed448.sig);
    memcpy(param.ed448.sig + 64, sig + 57, 57);
    s390x_flip_endian64(param.ed448.sig + 64, param.ed448.sig + 64);
    memcpy(param.ed448.pub, edkey->pubkey, 57);
    s390x_flip_endian64(param.ed448.pub, param.ed448.pub);

    return s390x_kdsa(S390X_EDDSA_VERIFY_ED448,
                      &param.ed448, tbs, tbslen) == 0 ? 1 : 0;
}

static const EVP_PKEY_METHOD ecx25519_s390x_pkey_meth = {
    EVP_PKEY_X25519,
    0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_keygen25519,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_derive25519,
    pkey_ecx_ctrl,
    0
};

static const EVP_PKEY_METHOD ecx448_s390x_pkey_meth = {
    EVP_PKEY_X448,
    0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_keygen448,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    s390x_pkey_ecx_derive448,
    pkey_ecx_ctrl,
    0
};
static const EVP_PKEY_METHOD ed25519_s390x_pkey_meth = {
    EVP_PKEY_ED25519, EVP_PKEY_FLAG_SIGCTX_CUSTOM,
    0, 0, 0, 0, 0, 0,
    s390x_pkey_ecd_keygen25519,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    pkey_ecd_ctrl,
    0,
    s390x_pkey_ecd_digestsign25519,
    s390x_pkey_ecd_digestverify25519
};

static const EVP_PKEY_METHOD ed448_s390x_pkey_meth = {
    EVP_PKEY_ED448, EVP_PKEY_FLAG_SIGCTX_CUSTOM,
    0, 0, 0, 0, 0, 0,
    s390x_pkey_ecd_keygen448,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    pkey_ecd_ctrl,
    0,
    s390x_pkey_ecd_digestsign448,
    s390x_pkey_ecd_digestverify448
};
#endif

const EVP_PKEY_METHOD *ossl_ecx25519_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_X25519))
        return &ecx25519_s390x_pkey_meth;
#endif
    return &ecx25519_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ecx448_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_X448))
        return &ecx448_s390x_pkey_meth;
#endif
    return &ecx448_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ed25519_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_ED25519)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_SIGN_ED25519)
        && OPENSSL_s390xcap_P.kdsa[0]
            & S390X_CAPBIT(S390X_EDDSA_VERIFY_ED25519))
        return &ed25519_s390x_pkey_meth;
#endif
    return &ed25519_pkey_meth;
}

const EVP_PKEY_METHOD *ossl_ed448_pkey_method(void)
{
#ifdef S390X_EC_ASM
    if (OPENSSL_s390xcap_P.pcc[1] & S390X_CAPBIT(S390X_SCALAR_MULTIPLY_ED448)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_SIGN_ED448)
        && OPENSSL_s390xcap_P.kdsa[0] & S390X_CAPBIT(S390X_EDDSA_VERIFY_ED448))
        return &ed448_s390x_pkey_meth;
#endif
    return &ed448_pkey_meth;
}