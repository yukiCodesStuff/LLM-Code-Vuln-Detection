    } while (BN_is_zero(k));

    BN_set_flags(k, BN_FLG_CONSTTIME);
    BN_set_flags(l, BN_FLG_CONSTTIME);

    if (dsa->flags & DSA_FLAG_CACHE_MONT_P) {
        if (!BN_MONT_CTX_set_locked(&dsa->method_mont_p,
                                    dsa->lock, dsa->p, ctx))