
#ifndef OPENSSL_NO_DH

# define FFDHE(sz) {                                                        \
        SN_ffdhe##sz, NID_ffdhe##sz,                                        \
        sz,                                                                 \
        &ossl_bignum_ffdhe##sz##_p, &ossl_bignum_ffdhe##sz##_q,             \
        &ossl_bignum_const_2,                                               \
    }

# define MODP(sz)  {                                                        \
        SN_modp_##sz, NID_modp_##sz,                                        \
        sz,                                                                 \
        &ossl_bignum_modp_##sz##_p, &ossl_bignum_modp_##sz##_q,             \
        &ossl_bignum_const_2                                                \
    }

# define RFC5114(name, uid, sz, tag) {                                      \
        name, uid,                                                          \
        sz,                                                                 \
        &ossl_bignum_dh##tag##_p, &ossl_bignum_dh##tag##_q,                 \
        &ossl_bignum_dh##tag##_g                                            \
    }

#else

# define FFDHE(sz)                      { SN_ffdhe##sz, NID_ffdhe##sz }
# define MODP(sz)                       { SN_modp_##sz, NID_modp_##sz }
# define RFC5114(name, uid, sz, tag)    { name, uid }

#endif

    int uid;
#ifndef OPENSSL_NO_DH
    int32_t nbits;
    const BIGNUM *p;
    const BIGNUM *q;
    const BIGNUM *g;
#endif
};

static const DH_NAMED_GROUP dh_named_groups[] = {
    FFDHE(2048),
    FFDHE(3072),
    FFDHE(4096),
    FFDHE(6144),
    FFDHE(8192),
#ifndef FIPS_MODULE
    MODP(1536),
#endif
    MODP(2048),
    MODP(3072),
    MODP(4096),
    MODP(6144),
    MODP(8192),
    /*
     * Additional dh named groups from RFC 5114 that have a different g.
     * The uid can be any unique identifier.
     */
}

#ifndef OPENSSL_NO_DH
const BIGNUM *ossl_ffc_named_group_get_q(const DH_NAMED_GROUP *group)
{
    if (group == NULL)
        return NULL;
    return group->q;
}

int ossl_ffc_named_group_set_pqg(FFC_PARAMS *ffc, const DH_NAMED_GROUP *group)
{
    if (ffc == NULL || group == NULL)
        return 0;

    ossl_ffc_params_set0_pqg(ffc, (BIGNUM *)group->p, (BIGNUM *)group->q,
                             (BIGNUM *)group->g);

    /* flush the cached nid, The DH layer is responsible for caching */
    ffc->nid = NID_undef;
    return 1;