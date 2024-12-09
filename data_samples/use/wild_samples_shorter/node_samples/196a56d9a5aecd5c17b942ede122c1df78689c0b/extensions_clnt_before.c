            if (!tls_group_allowed(s, pgroups[i], SSL_SECOP_CURVE_SUPPORTED))
                continue;

            curve_id = pgroups[i];
            break;
        }
    }
                                  X509 *x, size_t chainidx)
{
#ifndef OPENSSL_NO_TLS1_3
    uint32_t now, agesec, agems = 0;
    size_t reshashsize = 0, pskhashsize = 0, binderoffset, msglen;
    unsigned char *resbinder = NULL, *pskbinder = NULL, *msgstart = NULL;
    const EVP_MD *handmd = NULL, *mdres = NULL, *mdpsk = NULL;
    int dores = 0;
         * this in multiple places in the code, so portability shouldn't be an
         * issue.
         */
        now = (uint32_t)time(NULL);
        agesec = now - (uint32_t)s->session->time;
        /*
         * We calculate the age in seconds but the server may work in ms. Due to
         * rounding errors we could overestimate the age by up to 1s. It is
         * better to underestimate it. Otherwise, if the RTT is very short, when
                break;
        }
        if (i >= num_groups
                || !tls_group_allowed(s, group_id, SSL_SECOP_CURVE_SUPPORTED)) {
            SSLfatal(s, SSL_AD_ILLEGAL_PARAMETER, SSL_R_BAD_KEY_SHARE);
            return 0;
        }
