{
    int fixed_top;
    const BIGNUM *order = NULL;
    BIGNUM *tmp_key = NULL;

    if (key->group == NULL || key->group->meth == NULL)
        return 0;

    /*
     * Not only should key->group be set, but it should also be in a valid
     * fully initialized state.
     *
     * Specifically, to operate in constant time, we need that the group order
     * is set, as we use its length as the fixed public size of any scalar used
     * as an EC private key.
     */
    order = EC_GROUP_get0_order(key->group);
    if (order == NULL || BN_is_zero(order))
        return 0; /* This should never happen */

    if (key->group->meth->set_private != NULL
        && key->group->meth->set_private(key, priv_key) == 0)
        return 0;
    if (key->meth->set_private != NULL
        && key->meth->set_private(key, priv_key) == 0)
        return 0;

    /*
     * Return `0` to comply with legacy behavior for this function, see
     * https://github.com/openssl/openssl/issues/18744#issuecomment-1195175696
     */
    if (priv_key == NULL) {
        BN_clear_free(key->priv_key);
        key->priv_key = NULL;
        return 0; /* intentional for legacy compatibility */
    }