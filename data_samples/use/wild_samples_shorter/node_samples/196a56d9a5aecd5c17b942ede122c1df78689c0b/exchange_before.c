
int EVP_KEYEXCH_is_a(const EVP_KEYEXCH *keyexch, const char *name)
{
    return evp_is_a(keyexch->prov, keyexch->name_id, NULL, name);
}

void EVP_KEYEXCH_do_all_provided(OSSL_LIB_CTX *libctx,
                                 void (*fn)(EVP_KEYEXCH *keyexch, void *arg),