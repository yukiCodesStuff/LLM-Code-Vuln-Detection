{
    return keyexch->description;
}

int EVP_KEYEXCH_is_a(const EVP_KEYEXCH *keyexch, const char *name)
{
    return keyexch != NULL
           && evp_is_a(keyexch->prov, keyexch->name_id, NULL, name);
}