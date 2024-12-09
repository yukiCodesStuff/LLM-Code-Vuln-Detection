        for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            const SSL_CIPHER *c = sk_SSL_CIPHER_value(sk, i);

            if (!ossl_assert(c != NULL))
                continue;

            p = SSL_CIPHER_get_name(c);
            if (p == NULL)
                break;
            if (i != 0)
                BIO_printf(bio_out, ":");
            BIO_printf(bio_out, "%s", p);
        }
        for (i = 0; i < sk_SSL_CIPHER_num(sk); i++) {
            const SSL_CIPHER *c;

            c = sk_SSL_CIPHER_value(sk, i);

            if (!ossl_assert(c != NULL))
                continue;

            if (Verbose) {
                unsigned long id = SSL_CIPHER_get_id(c);
                int id0 = (int)(id >> 24);
                int id1 = (int)((id >> 16) & 0xffL);
                int id2 = (int)((id >> 8) & 0xffL);
                int id3 = (int)(id & 0xffL);

                if ((id & 0xff000000L) == 0x03000000L)
                    BIO_printf(bio_out, "          0x%02X,0x%02X - ", id2, id3); /* SSL3
                                                                                  * cipher */
                else
                    BIO_printf(bio_out, "0x%02X,0x%02X,0x%02X,0x%02X - ", id0, id1, id2, id3); /* whatever */
            }