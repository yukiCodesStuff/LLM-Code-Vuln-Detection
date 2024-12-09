/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2019-2020, Oracle and/or its affiliates.  All rights reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
#include "testutil.h"

#include "internal/ffc.h"
#include "crypto/security_bits.h"

#ifndef OPENSSL_NO_DSA
static const unsigned char dsa_2048_224_sha224_p[] = {
    0x93, 0x57, 0x93, 0x62, 0x1b, 0x9a, 0x10, 0x9b, 0xc1, 0x56, 0x0f, 0x24,
    /* fail since N > len(q) */
    if (!TEST_false(ossl_ffc_generate_private_key(ctx, params, N + 1, 112, priv)))
        goto err;
    /* s must be always set */
    if (!TEST_false(ossl_ffc_generate_private_key(ctx, params, N, 0, priv)))
        goto err;
    /* pass since 2s <= N <= len(q) */
    if (!TEST_true(ossl_ffc_generate_private_key(ctx, params, N, 112, priv)))
        goto err;
    /* pass since N = len(q) */
        goto err;
    if (!TEST_true(ossl_ffc_validate_private_key(params->q, priv, &res)))
        goto err;
    /* N is ignored in this case */
    if (!TEST_true(ossl_ffc_generate_private_key(ctx, params, 0,
                                                 ossl_ifc_ffc_compute_security_bits(BN_num_bits(params->p)),
                                                 priv)))
        goto err;
    if (!TEST_int_le(BN_num_bits(priv), 225))
        goto err;
    if (!TEST_true(ossl_ffc_validate_private_key(params->q, priv, &res)))
        goto err;

    BN_CTX_free(ctx);
    return ret;
}

static int ffc_params_copy_test(void)
{
    int ret = 0;
    DH *dh = NULL;
    FFC_PARAMS *params, copy;

    ossl_ffc_params_init(&copy);

    if (!TEST_ptr(dh = DH_new_by_nid(NID_ffdhe3072)))
        goto err;
    params = ossl_dh_get0_params(dh);

    if (!TEST_int_eq(params->keylength, 275))
        goto err;

    if (!TEST_true(ossl_ffc_params_copy(&copy, params)))
        goto err;

    if (!TEST_int_eq(copy.keylength, 275))
        goto err;

    if (!TEST_true(ossl_ffc_params_cmp(&copy, params, 0)))
        goto err;

    ret = 1;
err:
    ossl_ffc_params_cleanup(&copy);
    DH_free(dh);
    return ret;
}
#endif /* OPENSSL_NO_DH */

int setup_tests(void)
{
    ADD_TEST(ffc_public_validate_test);
    ADD_TEST(ffc_private_validate_test);
    ADD_ALL_TESTS(ffc_private_gen_test, 10);
    ADD_TEST(ffc_params_copy_test);
#endif /* OPENSSL_NO_DH */
    return 1;
}