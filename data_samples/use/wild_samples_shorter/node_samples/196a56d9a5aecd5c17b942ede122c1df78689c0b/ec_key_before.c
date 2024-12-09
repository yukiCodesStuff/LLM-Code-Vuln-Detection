        && key->meth->set_private(key, priv_key) == 0)
        return 0;

    /*
     * We should never leak the bit length of the secret scalar in the key,
     * so we always set the `BN_FLG_CONSTTIME` flag on the internal `BIGNUM`
     * holding the secret scalar.