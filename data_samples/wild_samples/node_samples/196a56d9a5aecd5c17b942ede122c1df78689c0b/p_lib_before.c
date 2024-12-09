{
    /*
     * This will just call evp_keymgmt_util_match when legacy support
     * is gone.
     */

    /* Trivial shortcuts */
    if (a == b)
        return 1;
    if (a == NULL || b == NULL)
        return 0;

    if (a->keymgmt != NULL || b->keymgmt != NULL)
        return evp_pkey_cmp_any(a, b, (SELECT_PARAMETERS
                                       | OSSL_KEYMGMT_SELECT_KEYPAIR));

    /* All legacy keys */
    if (a->type != b->type)
        return -1;

    if (a->ameth != NULL) {
        int ret;
        /* Compare parameters if the algorithm has them */
        if (a->ameth->param_cmp != NULL) {
            ret = a->ameth->param_cmp(a, b);
            if (ret <= 0)
                return ret;
        }

        if (a->ameth->pub_cmp != NULL)
            return a->ameth->pub_cmp(a, b);
    }
    for (i = 0; i < OSSL_NELEM(standard_name2type); i++) {
        if (type == (int)standard_name2type[i].id)
            return standard_name2type[i].ptr;
    }

    return OBJ_nid2sn(type);
}

int EVP_PKEY_is_a(const EVP_PKEY *pkey, const char *name)
{
    if (pkey->keymgmt == NULL) {
        int type = evp_pkey_name2type(name);

        return pkey->type == type;
    }
    return EVP_KEYMGMT_is_a(pkey->keymgmt, name);
}
    if (pkey != NULL && evp_pkey_is_provided(pkey)) {
        size_t return_size = OSSL_PARAM_UNMODIFIED;

        /*
         * We know that this is going to fail, but it will give us a size
         * to allocate.
         */
        EVP_PKEY_get_octet_string_param(pkey,
                                        OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                        NULL, 0, &return_size);
        if (return_size == OSSL_PARAM_UNMODIFIED)
            return 0;

        *ppub = OPENSSL_malloc(return_size);
        if (*ppub == NULL)
            return 0;

        if (!EVP_PKEY_get_octet_string_param(pkey,
                                             OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                             *ppub, return_size, NULL))
            return 0;
        return return_size;
    }
    if (pkey != NULL && evp_pkey_is_provided(pkey)) {
        size_t return_size = OSSL_PARAM_UNMODIFIED;

        /*
         * We know that this is going to fail, but it will give us a size
         * to allocate.
         */
        EVP_PKEY_get_octet_string_param(pkey,
                                        OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                        NULL, 0, &return_size);
        if (return_size == OSSL_PARAM_UNMODIFIED)
            return 0;

        *ppub = OPENSSL_malloc(return_size);
        if (*ppub == NULL)
            return 0;

        if (!EVP_PKEY_get_octet_string_param(pkey,
                                             OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                             *ppub, return_size, NULL))
            return 0;
        return return_size;
    }


    rv = evp_pkey_asn1_ctrl(pkey, ASN1_PKEY_CTRL_GET1_TLS_ENCPT, 0, ppub);
    if (rv <= 0)
        return 0;
    return rv;
}

#endif /* FIPS_MODULE */

/*- All methods below can also be used in FIPS_MODULE */

EVP_PKEY *EVP_PKEY_new(void)
{
    EVP_PKEY *ret = OPENSSL_zalloc(sizeof(*ret));

    if (ret == NULL) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        return NULL;
    }

    ret->type = EVP_PKEY_NONE;
    ret->save_type = EVP_PKEY_NONE;
    ret->references = 1;

    ret->lock = CRYPTO_THREAD_lock_new();
    if (ret->lock == NULL) {
        EVPerr(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }

#ifndef FIPS_MODULE
    ret->save_parameters = 1;
    if (!CRYPTO_new_ex_data(CRYPTO_EX_INDEX_EVP_PKEY, ret, &ret->ex_data)) {
        ERR_raise(ERR_LIB_EVP, ERR_R_MALLOC_FAILURE);
        goto err;
    }
#endif
    return ret;

 err:
    CRYPTO_THREAD_lock_free(ret->lock);
    OPENSSL_free(ret);
    return NULL;
}

/*
 * Setup a public key management method.
 *
 * For legacy keys, either |type| or |str| is expected to have the type
 * information.  In this case, the setup consists of finding an ASN1 method
 * and potentially an ENGINE, and setting those fields in |pkey|.
 *
 * For provider side keys, |keymgmt| is expected to be non-NULL.  In this
 * case, the setup consists of setting the |keymgmt| field in |pkey|.
 *
 * If pkey is NULL just return 1 or 0 if the key management method exists.
 */

static int pkey_set_type(EVP_PKEY *pkey, ENGINE *e, int type, const char *str,
                         int len, EVP_KEYMGMT *keymgmt)
{
#ifndef FIPS_MODULE
    const EVP_PKEY_ASN1_METHOD *ameth = NULL;
    ENGINE **eptr = (e == NULL) ? &e :  NULL;
#endif

    /*
     * The setups can't set both legacy and provider side methods.
     * It is forbidden
     */
    if (!ossl_assert(type == EVP_PKEY_NONE || keymgmt == NULL)
        || !ossl_assert(e == NULL || keymgmt == NULL)) {
        ERR_raise(ERR_LIB_EVP, ERR_R_INTERNAL_ERROR);
        return 0;
    }

    if (pkey != NULL) {
        int free_it = 0;

#ifndef FIPS_MODULE
        free_it = free_it || pkey->pkey.ptr != NULL;
#endif
        free_it = free_it || pkey->keydata != NULL;
        if (free_it)
            evp_pkey_free_it(pkey);
#ifndef FIPS_MODULE
        /*
         * If key type matches and a method exists then this lookup has
         * succeeded once so just indicate success.
         */
        if (pkey->type != EVP_PKEY_NONE
            && type == pkey->save_type
            && pkey->ameth != NULL)
            return 1;
# ifndef OPENSSL_NO_ENGINE
        /* If we have ENGINEs release them */
        ENGINE_finish(pkey->engine);
        pkey->engine = NULL;
        ENGINE_finish(pkey->pmeth_engine);
        pkey->pmeth_engine = NULL;
# endif
#endif
    }
#ifndef FIPS_MODULE
    if (str != NULL)
        ameth = EVP_PKEY_asn1_find_str(eptr, str, len);
    else if (type != EVP_PKEY_NONE)
        ameth = EVP_PKEY_asn1_find(eptr, type);
# ifndef OPENSSL_NO_ENGINE
    if (pkey == NULL && eptr != NULL)
        ENGINE_finish(e);
# endif
#endif


    {
        int check = 1;

#ifndef FIPS_MODULE
        check = check && ameth == NULL;
#endif
        check = check && keymgmt == NULL;
        if (check) {
            ERR_raise(ERR_LIB_EVP, EVP_R_UNSUPPORTED_ALGORITHM);
            return 0;
        }