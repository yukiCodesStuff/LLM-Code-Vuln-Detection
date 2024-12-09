    const BIGNUM *priv_key = NULL, *pub_key = NULL;
    const FFC_PARAMS *params = NULL;
    const BIGNUM *p = NULL;

    if (out == NULL || dh == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    if (params != NULL
        && !ffc_params_to_text(out, params))
        return 0;

    return 1;
}
