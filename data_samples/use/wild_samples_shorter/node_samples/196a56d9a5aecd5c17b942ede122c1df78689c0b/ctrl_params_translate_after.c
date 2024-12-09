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

/*-
 * The translation table itself
 * ============================
 */
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

        ret = fixup(PRE_PARAMS_TO_CTRL, translation, &ctx);

        if (ret > 0 && ctx.action_type != NONE)
            ret = EVP_PKEY_CTX_ctrl(pctx, keytype, optype,
                                    ctx.ctrl_cmd, ctx.p1, ctx.p2);

        /*