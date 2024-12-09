/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    {"keyform", OPT_KEYFORM, 'f', "Key file format (ENGINE, other values ignored)"},
    {"hex", OPT_HEX, '-', "Print as hex dump"},
    {"binary", OPT_BINARY, '-', "Print in binary form"},
    {"xoflen", OPT_XOFLEN, 'p', "Output length for XOF algorithms. To obtain the maximum security strength set this to 32 (or greater) for SHAKE128, and 64 (or greater) for SHAKE256"},
    {"d", OPT_DEBUG, '-', "Print debug info"},
    {"debug", OPT_DEBUG, '-', "Print debug info"},

    OPT_SECTION("Signing"),
    }

    if (hmac_key != NULL) {
        if (md == NULL) {
            md = (EVP_MD *)EVP_sha256();
            digestname = SN_sha256;
        }
        sigkey = EVP_PKEY_new_raw_private_key(EVP_PKEY_HMAC, impl,
                                              (unsigned char *)hmac_key,
                                              strlen(hmac_key));
        if (sigkey == NULL)
            goto end;
        }
        if (do_verify)
            if (impl == NULL)
                res = EVP_DigestVerifyInit_ex(mctx, &pctx, digestname,
                                              app_get0_libctx(),
                                              app_get0_propq(), sigkey, NULL);
            else
                res = EVP_DigestVerifyInit(mctx, &pctx, md, impl, sigkey);
        else
            if (impl == NULL)
                res = EVP_DigestSignInit_ex(mctx, &pctx, digestname,
                                            app_get0_libctx(),
                                            app_get0_propq(), sigkey, NULL);
            else
                res = EVP_DigestSignInit(mctx, &pctx, md, impl, sigkey);
        if (res == 0) {
            BIO_printf(bio_err, "Error setting context\n");
            goto end;
        }
            BIO_printf(bio_err, "Length can only be specified for XOF\n");
            goto end;
        }
        /*
         * Signing using XOF is not supported by any algorithms currently since
         * each algorithm only calls EVP_DigestFinal_ex() in their sign_final
         * and verify_final methods.
         */
        if (sigkey != NULL) {
            BIO_printf(bio_err, "Signing key cannot be specified for XOF\n");
            goto end;
        }
        return;

    /* Filter out message digests that we cannot use */
    md = EVP_MD_fetch(app_get0_libctx(), name->name, app_get0_propq());
    if (md == NULL)
        return;

    BIO_printf(dec->bio, "-%-25s", name->name);