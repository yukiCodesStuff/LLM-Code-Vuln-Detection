    if (a == NULL || b == NULL)
        return 0;

    if (a->keymgmt != NULL || b->keymgmt != NULL)
        return evp_pkey_cmp_any(a, b, (SELECT_PARAMETERS
                                       | OSSL_KEYMGMT_SELECT_KEYPAIR));

    /* All legacy keys */
    if (a->type != b->type)
        return -1;

int EVP_PKEY_is_a(const EVP_PKEY *pkey, const char *name)
{
    if (pkey->keymgmt == NULL) {
        int type = evp_pkey_name2type(name);

        return pkey->type == type;
    }
    return EVP_KEYMGMT_is_a(pkey->keymgmt, name);
}

int EVP_PKEY_type_names_do_all(const EVP_PKEY *pkey,

    if (pkey != NULL && evp_pkey_is_provided(pkey)) {
        size_t return_size = OSSL_PARAM_UNMODIFIED;

        /*
         * We know that this is going to fail, but it will give us a size
         * to allocate.
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

