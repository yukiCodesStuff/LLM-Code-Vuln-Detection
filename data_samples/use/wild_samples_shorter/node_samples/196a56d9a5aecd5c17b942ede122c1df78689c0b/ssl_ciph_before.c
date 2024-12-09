/*
 * Copyright 1995-2021 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2002, Oracle and/or its affiliates. All rights reserved
 * Copyright 2005 Nokia. All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
        ctmp.id = s->compress_meth;
        if (ssl_comp_methods != NULL) {
            i = sk_SSL_COMP_find(ssl_comp_methods, &ctmp);
            *comp = sk_SSL_COMP_value(ssl_comp_methods, i);
        }
        /* If were only interested in comp then return success */
        if ((enc == NULL) && (md == NULL))
            return 1;
        if (c->algorithm_mac == SSL_AEAD)
            mac_pkey_type = NULL;
    } else {
        if (!ssl_evp_md_up_ref(ctx->ssl_digest_methods[i])) {
            ssl_evp_cipher_free(*enc);
            return 0;
        }
        *md = ctx->ssl_digest_methods[i];
        if (mac_pkey_type != NULL)
            *mac_pkey_type = ctx->ssl_mac_pkey_id[i];
        if (mac_secret_size != NULL)
            *mac_secret_size = ctx->ssl_mac_secret_size[i];
                 * alphanumeric, so we call this an error.
                 */
                ERR_raise(ERR_LIB_SSL, SSL_R_INVALID_COMMAND);
                retval = found = 0;
                l++;
                break;
            }

            if (rule == CIPHER_SPECIAL) {
                found = 0;      /* unused -- avoid compiler warning */