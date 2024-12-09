        {
            size_t pnum = RSA_get_multi_prime_extra_count(r);
            const BIGNUM *exps[10], *coeffs[10];

            if (coefficientnum - 1 < pnum
                && RSA_get0_multi_prime_crt_params(r, exps, coeffs))
                bn = coeffs[coefficientnum - 1];
        }
        break;
    }

    return get_payload_bn(state, translation, ctx, bn);
}

#define IMPL_GET_RSA_PAYLOAD_FACTOR(n)                                  \
    static int                                                          \
    get_rsa_payload_f##n(enum state state,                              \
                         const struct translation_st *translation,      \
                         struct translation_ctx_st *ctx)                \
    {                                                                   \
        if (EVP_PKEY_get_base_id(ctx->p2) != EVP_PKEY_RSA)              \
            return 0;                                                   \
        return get_rsa_payload_factor(state, translation, ctx, n - 1);  \
    }

#define IMPL_GET_RSA_PAYLOAD_EXPONENT(n)                                \
    static int                                                          \
    get_rsa_payload_e##n(enum state state,                              \
                         const struct translation_st *translation,      \
                         struct translation_ctx_st *ctx)                \
    {                                                                   \
        if (EVP_PKEY_get_base_id(ctx->p2) != EVP_PKEY_RSA)              \
            return 0;                                                   \
        return get_rsa_payload_exponent(state, translation, ctx,        \
                                        n - 1);                         \
    }

#define IMPL_GET_RSA_PAYLOAD_COEFFICIENT(n)                             \
    static int                                                          \
    get_rsa_payload_c##n(enum state state,                              \
                         const struct translation_st *translation,      \
                         struct translation_ctx_st *ctx)                \
    {                                                                   \
        if (EVP_PKEY_get_base_id(ctx->p2) != EVP_PKEY_RSA)              \
            return 0;                                                   \
        return get_rsa_payload_coefficient(state, translation, ctx,     \
                                           n - 1);                      \
    }

IMPL_GET_RSA_PAYLOAD_FACTOR(1)
IMPL_GET_RSA_PAYLOAD_FACTOR(2)
IMPL_GET_RSA_PAYLOAD_FACTOR(3)
IMPL_GET_RSA_PAYLOAD_FACTOR(4)
IMPL_GET_RSA_PAYLOAD_FACTOR(5)
IMPL_GET_RSA_PAYLOAD_FACTOR(6)
IMPL_GET_RSA_PAYLOAD_FACTOR(7)
IMPL_GET_RSA_PAYLOAD_FACTOR(8)
IMPL_GET_RSA_PAYLOAD_FACTOR(9)
IMPL_GET_RSA_PAYLOAD_FACTOR(10)
IMPL_GET_RSA_PAYLOAD_EXPONENT(1)
IMPL_GET_RSA_PAYLOAD_EXPONENT(2)
IMPL_GET_RSA_PAYLOAD_EXPONENT(3)
IMPL_GET_RSA_PAYLOAD_EXPONENT(4)
IMPL_GET_RSA_PAYLOAD_EXPONENT(5)
IMPL_GET_RSA_PAYLOAD_EXPONENT(6)
IMPL_GET_RSA_PAYLOAD_EXPONENT(7)
IMPL_GET_RSA_PAYLOAD_EXPONENT(8)
IMPL_GET_RSA_PAYLOAD_EXPONENT(9)
IMPL_GET_RSA_PAYLOAD_EXPONENT(10)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(1)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(2)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(3)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(4)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(5)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(6)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(7)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(8)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(9)

static int fix_group_ecx(enum state state,
                         const struct translation_st *translation,
                         struct translation_ctx_st *ctx)
{
    const char *value = NULL;

    switch (state) {
    case PRE_PARAMS_TO_CTRL:
        if (!EVP_PKEY_CTX_IS_GEN_OP(ctx->pctx))
            return 0;
        ctx->action_type = NONE;
        return 1;
    case POST_PARAMS_TO_CTRL:
        if (OSSL_PARAM_get_utf8_string_ptr(ctx->params, &value) == 0 ||
            OPENSSL_strcasecmp(ctx->pctx->keytype, value) != 0) {
            ERR_raise(ERR_LIB_EVP, ERR_R_PASSED_INVALID_ARGUMENT);
            ctx->p1 = 0;
            return 0;
        }
        ctx->p1 = 1;
        return 1;
    default:
        return 0;
    }
}
static const struct translation_st evp_pkey_ctx_translations[] = {
    /*
     * DistID: we pass it to the backend as an octet string,
     * but get it back as a pointer to an octet string.
     *
     * Note that the EVP_PKEY_CTRL_GET1_ID_LEN is purely for legacy purposes
     * that has no separate counterpart in OSSL_PARAM terms, since we get
     * the length of the DistID automatically when getting the DistID itself.
     */
    { SET, -1, -1, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_SET1_ID, "distid", "hexdistid",
      OSSL_PKEY_PARAM_DIST_ID, OSSL_PARAM_OCTET_STRING, NULL },
    { GET, -1, -1, -1,
      EVP_PKEY_CTRL_GET1_ID, "distid", "hexdistid",
      OSSL_PKEY_PARAM_DIST_ID, OSSL_PARAM_OCTET_PTR, NULL },
    { GET, -1, -1, -1,
      EVP_PKEY_CTRL_GET1_ID_LEN, NULL, NULL,
      OSSL_PKEY_PARAM_DIST_ID, OSSL_PARAM_OCTET_PTR, fix_distid_len },

    /*-
     * DH & DHX
     * ========
     */

    /*
     * EVP_PKEY_CTRL_DH_KDF_TYPE is used both for setting and getting.  The
     * fixup function has to handle this...
     */
    { NONE, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_KDF_TYPE, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_TYPE, OSSL_PARAM_UTF8_STRING,
      fix_dh_kdf_type },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_KDF_MD, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { GET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_DH_KDF_MD, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_KDF_OUTLEN, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_OUTLEN, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { GET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_DH_KDF_OUTLEN, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_OUTLEN, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_KDF_UKM, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_UKM, OSSL_PARAM_OCTET_STRING, NULL },
    { GET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_DH_KDF_UKM, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_UKM, OSSL_PARAM_OCTET_PTR, NULL },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_KDF_OID, NULL, NULL,
      OSSL_KDF_PARAM_CEK_ALG, OSSL_PARAM_UTF8_STRING, fix_oid },
    { GET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_DH_KDF_OID, NULL, NULL,
      OSSL_KDF_PARAM_CEK_ALG, OSSL_PARAM_UTF8_STRING, fix_oid },

    /* DHX Keygen Parameters that are shared with DH */
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_TYPE, "dh_paramgen_type", NULL,
      OSSL_PKEY_PARAM_FFC_TYPE, OSSL_PARAM_UTF8_STRING, fix_dh_paramgen_type },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_PRIME_LEN, "dh_paramgen_prime_len", NULL,
      OSSL_PKEY_PARAM_FFC_PBITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_PARAMGEN  | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_DH_NID, "dh_param", NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, NULL },
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_PARAMGEN  | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_DH_RFC5114, "dh_rfc5114", NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, fix_dh_nid5114 },

    /* DH Keygen Parameters that are shared with DHX */
    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_TYPE, "dh_paramgen_type", NULL,
      OSSL_PKEY_PARAM_FFC_TYPE, OSSL_PARAM_UTF8_STRING, fix_dh_paramgen_type },
    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_PRIME_LEN, "dh_paramgen_prime_len", NULL,
      OSSL_PKEY_PARAM_FFC_PBITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_PARAMGEN | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_DH_NID, "dh_param", NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, fix_dh_nid },
    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_PARAMGEN  | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_DH_RFC5114, "dh_rfc5114", NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, fix_dh_nid5114 },

    /* DH specific Keygen Parameters */
    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_GENERATOR, "dh_paramgen_generator", NULL,
      OSSL_PKEY_PARAM_DH_GENERATOR, OSSL_PARAM_INTEGER, NULL },

    /* DHX specific Keygen Parameters */
    { SET, EVP_PKEY_DHX, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DH_PARAMGEN_SUBPRIME_LEN, "dh_paramgen_subprime_len", NULL,
      OSSL_PKEY_PARAM_FFC_QBITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },

    { SET, EVP_PKEY_DH, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_DH_PAD, "dh_pad", NULL,
      OSSL_EXCHANGE_PARAM_PAD, OSSL_PARAM_UNSIGNED_INTEGER, NULL },

    /*-
     * DSA
     * ===
     */
    { SET, EVP_PKEY_DSA, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DSA_PARAMGEN_BITS, "dsa_paramgen_bits", NULL,
      OSSL_PKEY_PARAM_FFC_PBITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_DSA, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DSA_PARAMGEN_Q_BITS, "dsa_paramgen_q_bits", NULL,
      OSSL_PKEY_PARAM_FFC_QBITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_DSA, 0, EVP_PKEY_OP_PARAMGEN,
      EVP_PKEY_CTRL_DSA_PARAMGEN_MD, "dsa_paramgen_md", NULL,
      OSSL_PKEY_PARAM_FFC_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },

    /*-
     * EC
     * ==
     */
    { SET, EVP_PKEY_EC, 0, EVP_PKEY_OP_PARAMGEN | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_EC_PARAM_ENC, "ec_param_enc", NULL,
      OSSL_PKEY_PARAM_EC_ENCODING, OSSL_PARAM_UTF8_STRING, fix_ec_param_enc },
    { SET, EVP_PKEY_EC, 0, EVP_PKEY_OP_PARAMGEN | EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_EC_PARAMGEN_CURVE_NID, "ec_paramgen_curve", NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING,
      fix_ec_paramgen_curve_nid },
    /*
     * EVP_PKEY_CTRL_EC_ECDH_COFACTOR and EVP_PKEY_CTRL_EC_KDF_TYPE are used
     * both for setting and getting.  The fixup function has to handle this...
     */
    { NONE, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_EC_ECDH_COFACTOR, "ecdh_cofactor_mode", NULL,
      OSSL_EXCHANGE_PARAM_EC_ECDH_COFACTOR_MODE, OSSL_PARAM_INTEGER,
      fix_ecdh_cofactor },
    { NONE, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_EC_KDF_TYPE, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_TYPE, OSSL_PARAM_UTF8_STRING, fix_ec_kdf_type },
    { SET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_EC_KDF_MD, "ecdh_kdf_md", NULL,
      OSSL_EXCHANGE_PARAM_KDF_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { GET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_EC_KDF_MD, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_EC_KDF_OUTLEN, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_OUTLEN, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { GET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_EC_KDF_OUTLEN, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_OUTLEN, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_EC_KDF_UKM, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_UKM, OSSL_PARAM_OCTET_STRING, NULL },
    { GET, EVP_PKEY_EC, 0, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_GET_EC_KDF_UKM, NULL, NULL,
      OSSL_EXCHANGE_PARAM_KDF_UKM, OSSL_PARAM_OCTET_PTR, NULL },

    /*-
     * RSA
     * ===
     */

    /*
     * RSA padding modes are numeric with ctrls, strings with ctrl_strs,
     * and can be both with OSSL_PARAM.  We standardise on strings here,
     * fix_rsa_padding_mode() does the work when the caller has a different
     * idea.
     */
    { SET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS,
      EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_RSA_PADDING, "rsa_padding_mode", NULL,
      OSSL_PKEY_PARAM_PAD_MODE, OSSL_PARAM_UTF8_STRING, fix_rsa_padding_mode },
    { GET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS,
      EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_GET_RSA_PADDING, NULL, NULL,
      OSSL_PKEY_PARAM_PAD_MODE, OSSL_PARAM_UTF8_STRING, fix_rsa_padding_mode },

    { SET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS,
      EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_RSA_MGF1_MD, "rsa_mgf1_md", NULL,
      OSSL_PKEY_PARAM_MGF1_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { GET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS,
      EVP_PKEY_OP_TYPE_CRYPT | EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_GET_RSA_MGF1_MD, NULL, NULL,
      OSSL_PKEY_PARAM_MGF1_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },

    /*
     * RSA-PSS saltlen is essentially numeric, but certain values can be
     * expressed as keywords (strings) with ctrl_str.  The corresponding
     * OSSL_PARAM allows both forms.
     * fix_rsa_pss_saltlen() takes care of the distinction.
     */
    { SET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_RSA_PSS_SALTLEN, "rsa_pss_saltlen", NULL,
      OSSL_PKEY_PARAM_RSA_PSS_SALTLEN, OSSL_PARAM_UTF8_STRING,
      fix_rsa_pss_saltlen },
    { GET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_GET_RSA_PSS_SALTLEN, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_PSS_SALTLEN, OSSL_PARAM_UTF8_STRING,
      fix_rsa_pss_saltlen },

    { SET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_TYPE_CRYPT,
      EVP_PKEY_CTRL_RSA_OAEP_MD, "rsa_oaep_md", NULL,
      OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { GET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_TYPE_CRYPT,
      EVP_PKEY_CTRL_GET_RSA_OAEP_MD, NULL, NULL,
      OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    /*
     * The "rsa_oaep_label" ctrl_str expects the value to always be hex.
     * This is accomodated by default_fixup_args() above, which mimics that
     * expectation for any translation item where |ctrl_str| is NULL and
     * |ctrl_hexstr| is non-NULL.
     */
    { SET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_TYPE_CRYPT,
      EVP_PKEY_CTRL_RSA_OAEP_LABEL, NULL, "rsa_oaep_label",
      OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL, OSSL_PARAM_OCTET_STRING, NULL },
    { GET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_TYPE_CRYPT,
      EVP_PKEY_CTRL_GET_RSA_OAEP_LABEL, NULL, NULL,
      OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL, OSSL_PARAM_OCTET_STRING, NULL },

    { SET, EVP_PKEY_RSA_PSS, 0, EVP_PKEY_OP_TYPE_GEN,
      EVP_PKEY_CTRL_MD, "rsa_pss_keygen_md", NULL,
      OSSL_ALG_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, EVP_PKEY_RSA_PSS, 0, EVP_PKEY_OP_TYPE_GEN,
      EVP_PKEY_CTRL_RSA_MGF1_MD, "rsa_pss_keygen_mgf1_md", NULL,
      OSSL_PKEY_PARAM_MGF1_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, EVP_PKEY_RSA_PSS, 0, EVP_PKEY_OP_TYPE_GEN,
      EVP_PKEY_CTRL_RSA_PSS_SALTLEN, "rsa_pss_keygen_saltlen", NULL,
      OSSL_SIGNATURE_PARAM_PSS_SALTLEN, OSSL_PARAM_INTEGER, NULL },
    { SET, EVP_PKEY_RSA, EVP_PKEY_RSA_PSS, EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_RSA_KEYGEN_BITS, "rsa_keygen_bits", NULL,
      OSSL_PKEY_PARAM_RSA_BITS, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_RSA_KEYGEN_PUBEXP, "rsa_keygen_pubexp", NULL,
      OSSL_PKEY_PARAM_RSA_E, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, EVP_PKEY_RSA, 0, EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_RSA_KEYGEN_PRIMES, "rsa_keygen_primes", NULL,
      OSSL_PKEY_PARAM_RSA_PRIMES, OSSL_PARAM_UNSIGNED_INTEGER, NULL },

    /*-
     * SipHash
     * ======
     */
    { SET, -1, -1, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_SET_DIGEST_SIZE, "digestsize", NULL,
      OSSL_MAC_PARAM_SIZE, OSSL_PARAM_UNSIGNED_INTEGER, NULL },

    /*-
     * TLS1-PRF
     * ========
     */
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_TLS_MD, "md", NULL,
      OSSL_KDF_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_TLS_SECRET, "secret", "hexsecret",
      OSSL_KDF_PARAM_SECRET, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_TLS_SEED, "seed", "hexseed",
      OSSL_KDF_PARAM_SEED, OSSL_PARAM_OCTET_STRING, NULL },

    /*-
     * HKDF
     * ====
     */
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_HKDF_MD, "md", NULL,
      OSSL_KDF_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_HKDF_SALT, "salt", "hexsalt",
      OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_HKDF_KEY, "key", "hexkey",
      OSSL_KDF_PARAM_KEY, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_HKDF_INFO, "info", "hexinfo",
      OSSL_KDF_PARAM_INFO, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_HKDF_MODE, "mode", NULL,
      OSSL_KDF_PARAM_MODE, OSSL_PARAM_INTEGER, fix_hkdf_mode },

    /*-
     * Scrypt
     * ======
     */
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_PASS, "pass", "hexpass",
      OSSL_KDF_PARAM_PASSWORD, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_SCRYPT_SALT, "salt", "hexsalt",
      OSSL_KDF_PARAM_SALT, OSSL_PARAM_OCTET_STRING, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_SCRYPT_N, "N", NULL,
      OSSL_KDF_PARAM_SCRYPT_N, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_SCRYPT_R, "r", NULL,
      OSSL_KDF_PARAM_SCRYPT_R, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_SCRYPT_P, "p", NULL,
      OSSL_KDF_PARAM_SCRYPT_P, OSSL_PARAM_UNSIGNED_INTEGER, NULL },
    { SET, -1, -1, EVP_PKEY_OP_DERIVE,
      EVP_PKEY_CTRL_SCRYPT_MAXMEM_BYTES, "maxmem_bytes", NULL,
      OSSL_KDF_PARAM_SCRYPT_MAXMEM, OSSL_PARAM_UNSIGNED_INTEGER, NULL },

    { SET, -1, -1, EVP_PKEY_OP_KEYGEN | EVP_PKEY_OP_TYPE_CRYPT,
      EVP_PKEY_CTRL_CIPHER, NULL, NULL,
      OSSL_PKEY_PARAM_CIPHER, OSSL_PARAM_UTF8_STRING, fix_cipher },
    { SET, -1, -1, EVP_PKEY_OP_KEYGEN,
      EVP_PKEY_CTRL_SET_MAC_KEY, "key", "hexkey",
      OSSL_PKEY_PARAM_PRIV_KEY, OSSL_PARAM_OCTET_STRING, NULL },

    { SET, -1, -1, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_MD, NULL, NULL,
      OSSL_SIGNATURE_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
    { GET, -1, -1, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_GET_MD, NULL, NULL,
      OSSL_SIGNATURE_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },

    /*-
     * ECX
     * ===
     */
    { SET, EVP_PKEY_X25519, EVP_PKEY_X25519, EVP_PKEY_OP_KEYGEN, -1, NULL, NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, fix_group_ecx },
    { SET, EVP_PKEY_X448, EVP_PKEY_X448, EVP_PKEY_OP_KEYGEN, -1, NULL, NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING, fix_group_ecx },
};

static const struct translation_st evp_pkey_translations[] = {
    /*
     * The following contain no ctrls, they are exclusively here to extract
     * key payloads from legacy keys, using OSSL_PARAMs, and rely entirely
     * on |fixup_args| to pass the actual data.  The |fixup_args| should
     * expect to get the EVP_PKEY pointer through |ctx->p2|.
     */

    /* DH, DSA & EC */
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_GROUP_NAME, OSSL_PARAM_UTF8_STRING,
      get_payload_group_name },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_PRIV_KEY, OSSL_PARAM_UNSIGNED_INTEGER,
      get_payload_private_key },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_PUB_KEY,
      0 /* no data type, let get_payload_public_key() handle that */,
      get_payload_public_key },

    /* DH and DSA */
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_FFC_P, OSSL_PARAM_UNSIGNED_INTEGER,
      get_dh_dsa_payload_p },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_FFC_G, OSSL_PARAM_UNSIGNED_INTEGER,
      get_dh_dsa_payload_g },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_FFC_Q, OSSL_PARAM_UNSIGNED_INTEGER,
      get_dh_dsa_payload_q },

    /* RSA */
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_N, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_n },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_E, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_D, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_d },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR1, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f1 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR2, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f2 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR3, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f3 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR4, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f4 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR5, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f5 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR6, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f6 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR7, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f7 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR8, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f8 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR9, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f9 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_FACTOR10, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_f10 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT1, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e1 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT2, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e2 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT3, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e3 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT4, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e4 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT5, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e5 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT6, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e6 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT7, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e7 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT8, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e8 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT9, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e9 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_EXPONENT10, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_e10 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT1, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c1 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT2, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c2 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT3, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c3 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT4, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c4 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT5, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c5 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT6, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c6 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT7, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c7 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT8, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c8 },
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_RSA_COEFFICIENT9, OSSL_PARAM_UNSIGNED_INTEGER,
      get_rsa_payload_c9 },

    /* EC */
    { GET, -1, -1, -1, 0, NULL, NULL,
      OSSL_PKEY_PARAM_EC_DECODED_FROM_EXPLICIT_PARAMS, OSSL_PARAM_INTEGER,
      get_ec_decoded_from_explicit_params },
};

static const struct translation_st *
lookup_translation(struct translation_st *tmpl,
                   const struct translation_st *translations,
                   size_t translations_num)
{
    size_t i;

    for (i = 0; i < translations_num; i++) {
        const struct translation_st *item = &translations[i];

        /*
         * Sanity check the translation table item.
         *
         * 1.  Either both keytypes are -1, or neither of them are.
         * 2.  TBA...
         */
        if (!ossl_assert((item->keytype1 == -1) == (item->keytype2 == -1)))
            continue;


        /*
         * Base search criteria: check that the optype and keytypes match,
         * if relevant.  All callers must synthesise these bits somehow.
         */
        if (item->optype != -1 && (tmpl->optype & item->optype) == 0)
            continue;
        /*
         * This expression is stunningly simple thanks to the sanity check
         * above.
         */
        if (item->keytype1 != -1
            && tmpl->keytype1 != item->keytype1
            && tmpl->keytype2 != item->keytype2)
            continue;

        /*
         * Done with the base search criteria, now we check the criteria for
         * the individual types of translations:
         * ctrl->params, ctrl_str->params, and params->ctrl
         */
        if (tmpl->ctrl_num != 0) {
            if (tmpl->ctrl_num != item->ctrl_num)
                continue;
        } else if (tmpl->ctrl_str != NULL) {
            const char *ctrl_str = NULL;
            const char *ctrl_hexstr = NULL;

            /*
             * Search criteria that originates from a ctrl_str is only used
             * for setting, never for getting.  Therefore, we only look at
             * the setter items.
             */
            if (item->action_type != NONE
                && item->action_type != SET)
                continue;
            /*
             * At least one of the ctrl cmd names must be match the ctrl
             * cmd name in the template.
             */
            if (item->ctrl_str != NULL
                && OPENSSL_strcasecmp(tmpl->ctrl_str, item->ctrl_str) == 0)
                ctrl_str = tmpl->ctrl_str;
            else if (item->ctrl_hexstr != NULL
                     && OPENSSL_strcasecmp(tmpl->ctrl_hexstr,
                                           item->ctrl_hexstr) == 0)
                ctrl_hexstr = tmpl->ctrl_hexstr;
            else
                continue;

            /* Modify the template to signal which string matched */
            tmpl->ctrl_str = ctrl_str;
            tmpl->ctrl_hexstr = ctrl_hexstr;
        } else if (tmpl->param_key != NULL) {
            /*
             * Search criteria that originates from a OSSL_PARAM setter or
             * getter.
             *
             * Ctrls were fundamentally bidirectional, with only the ctrl
             * command macro name implying direction (if you're lucky).
             * A few ctrl commands were even taking advantage of the
             * bidirectional nature, making the direction depend in the
             * value of the numeric argument.
             *
             * OSSL_PARAM functions are fundamentally different, in that
             * setters and getters are separated, so the data direction is
             * implied by the function that's used.  The same OSSL_PARAM
             * key name can therefore be used in both directions.  We must
             * therefore take the action type into account in this case.
             */
            if ((item->action_type != NONE
                 && tmpl->action_type != item->action_type)
                || (item->param_key != NULL
                    && OPENSSL_strcasecmp(tmpl->param_key,
                                          item->param_key) != 0))
                continue;
        } else {
            return NULL;
        }

        return item;
    }

    return NULL;
}

static const struct translation_st *
lookup_evp_pkey_ctx_translation(struct translation_st *tmpl)
{
    return lookup_translation(tmpl, evp_pkey_ctx_translations,
                              OSSL_NELEM(evp_pkey_ctx_translations));
}

static const struct translation_st *
lookup_evp_pkey_translation(struct translation_st *tmpl)
{
    return lookup_translation(tmpl, evp_pkey_translations,
                              OSSL_NELEM(evp_pkey_translations));
}

/* This must ONLY be called for provider side operations */
int evp_pkey_ctx_ctrl_to_param(EVP_PKEY_CTX *pctx,
                               int keytype, int optype,
                               int cmd, int p1, void *p2)
{
    struct translation_ctx_st ctx = { 0, };
    struct translation_st tmpl = { 0, };
    const struct translation_st *translation = NULL;
    OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
    int ret;
    fixup_args_fn *fixup = default_fixup_args;

    if (keytype == -1)
        keytype = pctx->legacy_keytype;
    tmpl.ctrl_num = cmd;
    tmpl.keytype1 = tmpl.keytype2 = keytype;
    tmpl.optype = optype;
    translation = lookup_evp_pkey_ctx_translation(&tmpl);

    if (translation == NULL) {
        ERR_raise(ERR_LIB_EVP, EVP_R_COMMAND_NOT_SUPPORTED);
        return -2;
    }

    if (pctx->pmeth != NULL
        && pctx->pmeth->pkey_id != translation->keytype1
        && pctx->pmeth->pkey_id != translation->keytype2)
        return -1;

    if (translation->fixup_args != NULL)
        fixup = translation->fixup_args;
    ctx.action_type = translation->action_type;
    ctx.ctrl_cmd = cmd;
    ctx.p1 = p1;
    ctx.p2 = p2;
    ctx.pctx = pctx;
    ctx.params = params;

    ret = fixup(PRE_CTRL_TO_PARAMS, translation, &ctx);

    if (ret > 0) {
        switch (ctx.action_type) {
        default:
            /* fixup_args is expected to make sure this is dead code */
            break;
        case GET:
            ret = evp_pkey_ctx_get_params_strict(pctx, ctx.params);
            break;
        case SET:
            ret = evp_pkey_ctx_set_params_strict(pctx, ctx.params);
            break;
        }
    }

    /*
     * In POST, we pass the return value as p1, allowing the fixup_args
     * function to affect it by changing its value.
     */
    if (ret > 0) {
        ctx.p1 = ret;
        fixup(POST_CTRL_TO_PARAMS, translation, &ctx);
        ret = ctx.p1;
    }

    cleanup_translation_ctx(POST_CTRL_TO_PARAMS, translation, &ctx);

    return ret;
}

/* This must ONLY be called for provider side operations */
int evp_pkey_ctx_ctrl_str_to_param(EVP_PKEY_CTX *pctx,
                                   const char *name, const char *value)
{
    struct translation_ctx_st ctx = { 0, };
    struct translation_st tmpl = { 0, };
    const struct translation_st *translation = NULL;
    OSSL_PARAM params[2] = { OSSL_PARAM_END, OSSL_PARAM_END };
    int keytype = pctx->legacy_keytype;
    int optype = pctx->operation == 0 ? -1 : pctx->operation;
    int ret;
    fixup_args_fn *fixup = default_fixup_args;

    tmpl.action_type = SET;
    tmpl.keytype1 = tmpl.keytype2 = keytype;
    tmpl.optype = optype;
    tmpl.ctrl_str = name;
    tmpl.ctrl_hexstr = name;
    translation = lookup_evp_pkey_ctx_translation(&tmpl);

    if (translation != NULL) {
        if (translation->fixup_args != NULL)
            fixup = translation->fixup_args;
        ctx.action_type = translation->action_type;
        ctx.ishex = (tmpl.ctrl_hexstr != NULL);
    } else {
        /* String controls really only support setting */
        ctx.action_type = SET;
    }
    ctx.ctrl_str = name;
    ctx.p1 = (int)strlen(value);
    ctx.p2 = (char *)value;
    ctx.pctx = pctx;
    ctx.params = params;

    ret = fixup(PRE_CTRL_STR_TO_PARAMS, translation, &ctx);

    if (ret > 0) {
        switch (ctx.action_type) {
        default:
            /* fixup_args is expected to make sure this is dead code */
            break;
        case GET:
            /*
             * this is dead code, but must be present, or some compilers
             * will complain
             */
            break;
        case SET:
            ret = evp_pkey_ctx_set_params_strict(pctx, ctx.params);
            break;
        }
    }

    if (ret > 0)
        ret = fixup(POST_CTRL_STR_TO_PARAMS, translation, &ctx);

    cleanup_translation_ctx(CLEANUP_CTRL_STR_TO_PARAMS, translation, &ctx);

    return ret;
}

/* This must ONLY be called for legacy operations */
static int evp_pkey_ctx_setget_params_to_ctrl(EVP_PKEY_CTX *pctx,
                                              enum action action_type,
                                              OSSL_PARAM *params)
{
    int keytype = pctx->legacy_keytype;
    int optype = pctx->operation == 0 ? -1 : pctx->operation;

    for (; params != NULL && params->key != NULL; params++) {
        struct translation_ctx_st ctx = { 0, };
        struct translation_st tmpl = { 0, };
        const struct translation_st *translation = NULL;
        fixup_args_fn *fixup = default_fixup_args;
        int ret;

        tmpl.action_type = action_type;
        tmpl.keytype1 = tmpl.keytype2 = keytype;
        tmpl.optype = optype;
        tmpl.param_key = params->key;
        translation = lookup_evp_pkey_ctx_translation(&tmpl);

        if (translation != NULL) {
            if (translation->fixup_args != NULL)
                fixup = translation->fixup_args;
            ctx.action_type = translation->action_type;
        }
        ctx.pctx = pctx;
        ctx.params = params;

        ret = fixup(PRE_PARAMS_TO_CTRL, translation, &ctx);

        if (ret > 0 && ctx.action_type != NONE)
            ret = EVP_PKEY_CTX_ctrl(pctx, keytype, optype,
                                    ctx.ctrl_cmd, ctx.p1, ctx.p2);

        /*
         * In POST, we pass the return value as p1, allowing the fixup_args
         * function to put it to good use, or maybe affect it.
         */
        if (ret > 0) {
            ctx.p1 = ret;
            fixup(POST_PARAMS_TO_CTRL, translation, &ctx);
            ret = ctx.p1;
        }

        cleanup_translation_ctx(CLEANUP_PARAMS_TO_CTRL, translation, &ctx);

        if (ret <= 0)
            return 0;
    }
    return 1;
}

int evp_pkey_ctx_set_params_to_ctrl(EVP_PKEY_CTX *ctx, const OSSL_PARAM *params)
{
    return evp_pkey_ctx_setget_params_to_ctrl(ctx, SET, (OSSL_PARAM *)params);
}

int evp_pkey_ctx_get_params_to_ctrl(EVP_PKEY_CTX *ctx, OSSL_PARAM *params)
{
    return evp_pkey_ctx_setget_params_to_ctrl(ctx, GET, params);
}

/* This must ONLY be called for legacy EVP_PKEYs */
static int evp_pkey_setget_params_to_ctrl(const EVP_PKEY *pkey,
                                          enum action action_type,
                                          OSSL_PARAM *params)
{
    int ret = 1;

    for (; params != NULL && params->key != NULL; params++) {
        struct translation_ctx_st ctx = { 0, };
        struct translation_st tmpl = { 0, };
        const struct translation_st *translation = NULL;
        fixup_args_fn *fixup = default_fixup_args;

        tmpl.action_type = action_type;
        tmpl.param_key = params->key;
        translation = lookup_evp_pkey_translation(&tmpl);

        if (translation != NULL) {
            if (translation->fixup_args != NULL)
                fixup = translation->fixup_args;
            ctx.action_type = translation->action_type;
        }
        ctx.p2 = (void *)pkey;
        ctx.params = params;

        /*
         * EVP_PKEY doesn't have any ctrl function, so we rely completely
         * on fixup_args to do the whole work.  Also, we currently only
         * support getting.
         */
        if (!ossl_assert(translation != NULL)
            || !ossl_assert(translation->action_type == GET)
            || !ossl_assert(translation->fixup_args != NULL)) {
            return -2;
        }

        ret = fixup(PKEY, translation, &ctx);

        cleanup_translation_ctx(PKEY, translation, &ctx);
    }
    return ret;
}

int evp_pkey_get_params_to_ctrl(const EVP_PKEY *pkey, OSSL_PARAM *params)
{
    return evp_pkey_setget_params_to_ctrl(pkey, GET, params);
}
        if (translation != NULL) {
            if (translation->fixup_args != NULL)
                fixup = translation->fixup_args;
            ctx.action_type = translation->action_type;
        }
        ctx.pctx = pctx;
        ctx.params = params;

        ret = fixup(PRE_PARAMS_TO_CTRL, translation, &ctx);

        if (ret > 0 && ctx.action_type != NONE)
            ret = EVP_PKEY_CTX_ctrl(pctx, keytype, optype,
                                    ctx.ctrl_cmd, ctx.p1, ctx.p2);

        /*
         * In POST, we pass the return value as p1, allowing the fixup_args
         * function to put it to good use, or maybe affect it.
         */
        if (ret > 0) {
            ctx.p1 = ret;
            fixup(POST_PARAMS_TO_CTRL, translation, &ctx);
            ret = ctx.p1;
        }