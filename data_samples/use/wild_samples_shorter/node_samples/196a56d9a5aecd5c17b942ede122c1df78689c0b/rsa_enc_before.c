/*
 * Copyright 2019-2021 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
    const OSSL_PARAM *p;
    char mdname[OSSL_MAX_NAME_SIZE];
    char mdprops[OSSL_MAX_PROPQUERY_SIZE] = { '\0' };
    char *str = mdname;

    if (prsactx == NULL)
        return 0;
    if (params == NULL)

    p = OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST);
    if (p != NULL) {
        if (!OSSL_PARAM_get_utf8_string(p, &str, sizeof(mdname)))
            return 0;

        str = mdprops;
        p = OSSL_PARAM_locate_const(params,
                                    OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST_PROPS);
        if (p != NULL) {
            if (!OSSL_PARAM_get_utf8_string(p, &str, sizeof(mdprops)))
                return 0;
        }


    p = OSSL_PARAM_locate_const(params, OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST);
    if (p != NULL) {
        if (!OSSL_PARAM_get_utf8_string(p, &str, sizeof(mdname)))
            return 0;

        str = mdprops;
        p = OSSL_PARAM_locate_const(params,
                                    OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST_PROPS);
        if (p != NULL) {
            if (!OSSL_PARAM_get_utf8_string(p, &str, sizeof(mdprops)))
                return 0;
        } else {
            str = NULL;