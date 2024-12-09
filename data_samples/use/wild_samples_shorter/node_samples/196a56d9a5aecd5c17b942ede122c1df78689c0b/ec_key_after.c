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

    /*
     * We should never leak the bit length of the secret scalar in the key,
     * so we always set the `BN_FLG_CONSTTIME` flag on the internal `BIGNUM`
     * holding the secret scalar.