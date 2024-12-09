    if (!verbose) {
        for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            const SSL_CIPHER *c = sk_SSL_CIPHER_value(sk, i);

            if (!ossl_assert(c != NULL))
                continue;

            p = SSL_CIPHER_get_name(c);
            if (p == NULL)
                break;
            if (i != 0)

            c = sk_SSL_CIPHER_value(sk, i);

            if (!ossl_assert(c != NULL))
                continue;

            if (Verbose) {
                unsigned long id = SSL_CIPHER_get_id(c);
                int id0 = (int)(id >> 24);
                int id1 = (int)((id >> 16) & 0xffL);