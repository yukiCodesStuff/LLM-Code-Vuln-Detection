/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
}
#endif

/*
 * Tests behavior of the EC_KEY_set_private_key
 */
static int set_private_key(void)
{
    EC_KEY *key = NULL, *aux_key = NULL;
    int testresult = 0;

    key = EC_KEY_new_by_curve_name(NID_secp224r1);
    aux_key = EC_KEY_new_by_curve_name(NID_secp224r1);
    if (!TEST_ptr(key)
        || !TEST_ptr(aux_key)
        || !TEST_int_eq(EC_KEY_generate_key(key), 1)
        || !TEST_int_eq(EC_KEY_generate_key(aux_key), 1))
        goto err;

    /* Test setting a valid private key */
    if (!TEST_int_eq(EC_KEY_set_private_key(key, aux_key->priv_key), 1))
        goto err;

    /* Test compliance with legacy behavior for NULL private keys */
    if (!TEST_int_eq(EC_KEY_set_private_key(key, NULL), 0)
        || !TEST_ptr_null(key->priv_key))
        goto err;

    testresult = 1;

 err:
    EC_KEY_free(key);
    EC_KEY_free(aux_key);
    return testresult;
}

/*
 * Tests behavior of the decoded_from_explicit_params flag and API
 */
static int decoded_flag_test(void)
#ifndef OPENSSL_NO_EC_NISTP_64_GCC_128
    ADD_TEST(underflow_test);
#endif
    ADD_TEST(set_private_key);
    ADD_TEST(decoded_flag_test);
    ADD_ALL_TESTS(ecpkparams_i2d2i_test, crv_len);

    return 1;