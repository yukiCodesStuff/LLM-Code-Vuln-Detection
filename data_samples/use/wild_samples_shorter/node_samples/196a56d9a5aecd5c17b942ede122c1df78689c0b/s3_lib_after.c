
            if (prefer_sha256) {
                const SSL_CIPHER *tmp = sk_SSL_CIPHER_value(allow, ii);
                const EVP_MD *md = ssl_md(s->ctx, tmp->algorithm2);

                if (md != NULL
                        && EVP_MD_is_a(md, OSSL_DIGEST_NAME_SHA2_256)) {
                    ret = tmp;
                    break;
                }
                if (ret == NULL)