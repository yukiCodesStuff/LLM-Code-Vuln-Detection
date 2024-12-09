{
    const DH *dh = key;
    const char *type_label = NULL;
    const BIGNUM *priv_key = NULL, *pub_key = NULL;
    const FFC_PARAMS *params = NULL;
    const BIGNUM *p = NULL;

    if (out == NULL || dh == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }
    if (p == NULL) {
        ERR_raise(ERR_LIB_PROV, PROV_R_INVALID_KEY);
        return 0;
    }

    if (BIO_printf(out, "%s: (%d bit)\n", type_label, BN_num_bits(p)) <= 0)
        return 0;
    if (priv_key != NULL
        && !print_labeled_bignum(out, "private-key:", priv_key))
        return 0;
    if (pub_key != NULL
        && !print_labeled_bignum(out, "public-key:", pub_key))
        return 0;
    if (params != NULL
        && !ffc_params_to_text(out, params))
        return 0;

    return 1;
}

# define dh_input_type          "DH"
# define dhx_input_type         "DHX"
#endif

/* ---------------------------------------------------------------------- */

#ifndef OPENSSL_NO_DSA
static int dsa_to_text(BIO *out, const void *key, int selection)
{
    const DSA *dsa = key;
    const char *type_label = NULL;
    const BIGNUM *priv_key = NULL, *pub_key = NULL;
    const FFC_PARAMS *params = NULL;
    const BIGNUM *p = NULL;

    if (out == NULL || dsa == NULL) {
        ERR_raise(ERR_LIB_PROV, ERR_R_PASSED_NULL_PARAMETER);
        return 0;
    }