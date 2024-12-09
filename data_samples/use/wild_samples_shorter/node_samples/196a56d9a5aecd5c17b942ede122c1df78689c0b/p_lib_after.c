    if (a == NULL || b == NULL)
        return 0;

    if (a->keymgmt != NULL || b->keymgmt != NULL) {
        int selection = SELECT_PARAMETERS;

        if (evp_keymgmt_util_has((EVP_PKEY *)a, OSSL_KEYMGMT_SELECT_PUBLIC_KEY)
            && evp_keymgmt_util_has((EVP_PKEY *)b, OSSL_KEYMGMT_SELECT_PUBLIC_KEY))
            selection |= OSSL_KEYMGMT_SELECT_PUBLIC_KEY;
        else
            selection |= OSSL_KEYMGMT_SELECT_KEYPAIR;
        return evp_pkey_cmp_any(a, b, selection);
    }

    /* All legacy keys */
    if (a->type != b->type)
        return -1;

int EVP_PKEY_is_a(const EVP_PKEY *pkey, const char *name)
{
    if (pkey == NULL)
        return 0;
    if (pkey->keymgmt == NULL)
        return pkey->type == evp_pkey_name2type(name);
    return EVP_KEYMGMT_is_a(pkey->keymgmt, name);
}

int EVP_PKEY_type_names_do_all(const EVP_PKEY *pkey,

    if (pkey != NULL && evp_pkey_is_provided(pkey)) {
        size_t return_size = OSSL_PARAM_UNMODIFIED;
        unsigned char *buf;

        /*
         * We know that this is going to fail, but it will give us a size
         * to allocate.
        if (return_size == OSSL_PARAM_UNMODIFIED)
            return 0;

        *ppub = NULL;
        buf = OPENSSL_malloc(return_size);
        if (buf == NULL)
            return 0;

        if (!EVP_PKEY_get_octet_string_param(pkey,
                                             OSSL_PKEY_PARAM_ENCODED_PUBLIC_KEY,
                                             buf, return_size, NULL)) {
            OPENSSL_free(buf);
            return 0;
        }
        *ppub = buf;
        return return_size;
    }

