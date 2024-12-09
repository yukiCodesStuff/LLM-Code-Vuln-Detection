{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey)
        || (key != NULL
            && s390x_x25519_mul(key, privkey, pubkey) == 0))
        return 0;
    *keylen = X25519_KEYLEN;
    return 1;
}

{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey)
        || (key != NULL
            && s390x_x448_mul(key, pubkey, privkey) == 0))
        return 0;
    *keylen = X448_KEYLEN;
    return 1;
}
