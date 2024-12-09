
#ifndef OPENSSL_NO_DH

# define FFDHE(sz, keylength) {                                             \
        SN_ffdhe##sz, NID_ffdhe##sz,                                        \
        sz,                                                                 \
        keylength,                                                          \
        &ossl_bignum_ffdhe##sz##_p, &ossl_bignum_ffdhe##sz##_q,             \
        &ossl_bignum_const_2,                                               \
    }

# define MODP(sz, keylength)  {                                             \
        SN_modp_##sz, NID_modp_##sz,                                        \
        sz,                                                                 \
        keylength,                                                          \
        &ossl_bignum_modp_##sz##_p, &ossl_bignum_modp_##sz##_q,             \
        &ossl_bignum_const_2                                                \
    }

# define RFC5114(name, uid, sz, tag) {                                      \
        name, uid,                                                          \
        sz,                                                                 \
        0,                                                                  \
        &ossl_bignum_dh##tag##_p, &ossl_bignum_dh##tag##_q,                 \
        &ossl_bignum_dh##tag##_g                                            \
    }

#else

# define FFDHE(sz, keylength)           { SN_ffdhe##sz, NID_ffdhe##sz }
# define MODP(sz, keylength)            { SN_modp_##sz, NID_modp_##sz }
# define RFC5114(name, uid, sz, tag)    { name, uid }

#endif

    int uid;
#ifndef OPENSSL_NO_DH
    int32_t nbits;
    int keylength;
    const BIGNUM *p;
    const BIGNUM *q;
    const BIGNUM *g;
#endif
};

/*
 * The private key length values are taken from RFC7919 with the values for
 * MODP primes given the same lengths as the equivalent FFDHE.
 * The MODP 1536 value is approximated.
 */
static const DH_NAMED_GROUP dh_named_groups[] = {
    FFDHE(2048, 225),
    FFDHE(3072, 275),
    FFDHE(4096, 325),
    FFDHE(6144, 375),
    FFDHE(8192, 400),
#ifndef FIPS_MODULE
    MODP(1536, 200),
#endif
    MODP(2048, 225),
    MODP(3072, 275),
    MODP(4096, 325),
    MODP(6144, 375),
    MODP(8192, 400),
    /*
     * Additional dh named groups from RFC 5114 that have a different g.
     * The uid can be any unique identifier.
     */
}

#ifndef OPENSSL_NO_DH
int ossl_ffc_named_group_get_keylength(const DH_NAMED_GROUP *group)
{
    if (group == NULL)
        return 0;
    return group->keylength;
}

const BIGNUM *ossl_ffc_named_group_get_q(const DH_NAMED_GROUP *group)
{
    if (group == NULL)
        return NULL;
    return group->q;
}

int ossl_ffc_named_group_set(FFC_PARAMS *ffc, const DH_NAMED_GROUP *group)
{
    if (ffc == NULL || group == NULL)
        return 0;

    ossl_ffc_params_set0_pqg(ffc, (BIGNUM *)group->p, (BIGNUM *)group->q,
                             (BIGNUM *)group->g);
    ffc->keylength = group->keylength;

    /* flush the cached nid, The DH layer is responsible for caching */
    ffc->nid = NID_undef;
    return 1;