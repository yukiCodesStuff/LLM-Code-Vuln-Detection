IMPL_GET_RSA_PAYLOAD_COEFFICIENT(8)
IMPL_GET_RSA_PAYLOAD_COEFFICIENT(9)

/*-
 * The translation table itself
 * ============================
 */
    { GET, -1, -1, EVP_PKEY_OP_TYPE_SIG,
      EVP_PKEY_CTRL_GET_MD, NULL, NULL,
      OSSL_SIGNATURE_PARAM_DIGEST, OSSL_PARAM_UTF8_STRING, fix_md },
};

static const struct translation_st evp_pkey_translations[] = {
    /*

        ret = fixup(PRE_PARAMS_TO_CTRL, translation, &ctx);

        if (ret > 0 && action_type != NONE)
            ret = EVP_PKEY_CTX_ctrl(pctx, keytype, optype,
                                    ctx.ctrl_cmd, ctx.p1, ctx.p2);

        /*