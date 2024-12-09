{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey))
        return 0;

    if (key != NULL)
        return s390x_x25519_mul(key, pubkey, privkey);

    *keylen = X25519_KEYLEN;
    return 1;
}

{
    const unsigned char *privkey, *pubkey;

    if (!validate_ecx_derive(ctx, key, keylen, &privkey, &pubkey))
        return 0;

    if (key != NULL)
        return s390x_x448_mul(key, pubkey, privkey);

    *keylen = X448_KEYLEN;
    return 1;
}
