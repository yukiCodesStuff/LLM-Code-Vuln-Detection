/*
 * Copyright 2006-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    out = bio_open_default(outfile, 'w', FORMAT_PEM);
    if (out == NULL)
        goto end;
    pkey = PEM_read_bio_Parameters_ex(in, NULL, app_get0_libctx(),
                                      app_get0_propq());
    if (pkey == NULL) {
        BIO_printf(bio_err, "Error reading parameters\n");
        ERR_print_errors(bio_err);
        goto end;
    }

    if (check) {
        if (e == NULL)
            ctx = EVP_PKEY_CTX_new_from_pkey(app_get0_libctx(), pkey,
                                             app_get0_propq());
        else
            ctx = EVP_PKEY_CTX_new(pkey, e);
        if (ctx == NULL) {
            ERR_print_errors(bio_err);
            goto end;
        }