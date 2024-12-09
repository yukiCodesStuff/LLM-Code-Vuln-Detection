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
     * We should never leak the bit length of the secret scalar in the key,
     * so we always set the `BN_FLG_CONSTTIME` flag on the internal `BIGNUM`
     * holding the secret scalar.
     *
     * This is important also because `BN_dup()` (and `BN_copy()`) do not
     * propagate the `BN_FLG_CONSTTIME` flag from the source `BIGNUM`, and
     * this brings an extra risk of inadvertently losing the flag, even when
     * the caller specifically set it.
     *
     * The propagation has been turned on and off a few times in the past
     * years because in some conditions has shown unintended consequences in
     * some code paths, so at the moment we can't fix this in the BN layer.
     *
     * In `EC_KEY_set_private_key()` we can work around the propagation by
     * manually setting the flag after `BN_dup()` as we know for sure that
     * inside the EC module the `BN_FLG_CONSTTIME` is always treated
     * correctly and should not generate unintended consequences.
     *
     * Setting the BN_FLG_CONSTTIME flag alone is never enough, we also have
     * to preallocate the BIGNUM internal buffer to a fixed public size big
     * enough that operations performed during the processing never trigger
     * a realloc which would leak the size of the scalar through memory
     * accesses.
     *
     * Fixed Length
     * ------------
     *
     * The order of the large prime subgroup of the curve is our choice for
     * a fixed public size, as that is generally the upper bound for
     * generating a private key in EC cryptosystems and should fit all valid
     * secret scalars.
     *
     * For preallocating the BIGNUM storage we look at the number of "words"
     * required for the internal representation of the order, and we
     * preallocate 2 extra "words" in case any of the subsequent processing
     * might temporarily overflow the order length.
     */
    tmp_key = BN_dup(priv_key);
    if (tmp_key == NULL)
        return 0;

    BN_set_flags(tmp_key, BN_FLG_CONSTTIME);

    fixed_top = bn_get_top(order) + 2;
    if (bn_wexpand(tmp_key, fixed_top) == NULL) {
        BN_clear_free(tmp_key);
        return 0;
    }