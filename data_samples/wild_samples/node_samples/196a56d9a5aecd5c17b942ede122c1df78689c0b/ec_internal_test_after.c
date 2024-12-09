/*
 * Copyright 2019-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Low level APIs are deprecated for public use, but still ok for internal use.
 */
#include "internal/deprecated.h"

#include "internal/nelem.h"
#include "testutil.h"
#include <openssl/ec.h>
#include "ec_local.h"
#include <openssl/objects.h>

static size_t crv_len = 0;
static EC_builtin_curve *curves = NULL;

/* sanity checks field_inv function pointer in EC_METHOD */
static int group_field_tests(const EC_GROUP *group, BN_CTX *ctx)
{
    BIGNUM *a = NULL, *b = NULL, *c = NULL;
    int ret = 0;

    if (group->meth->field_inv == NULL || group->meth->field_mul == NULL)
        return 1;

    BN_CTX_start(ctx);
    a = BN_CTX_get(ctx);
    b = BN_CTX_get(ctx);
    if (!TEST_ptr(c = BN_CTX_get(ctx))
        /* 1/1 = 1 */
        || !TEST_true(group->meth->field_inv(group, b, BN_value_one(), ctx))
        || !TEST_true(BN_is_one(b))
        /* (1/a)*a = 1 */
        || !TEST_true(BN_rand(a, BN_num_bits(group->field) - 1,
                              BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ANY))
        || !TEST_true(group->meth->field_inv(group, b, a, ctx))
        || (group->meth->field_encode &&
            !TEST_true(group->meth->field_encode(group, a, a, ctx)))
        || (group->meth->field_encode &&
            !TEST_true(group->meth->field_encode(group, b, b, ctx)))
        || !TEST_true(group->meth->field_mul(group, c, a, b, ctx))
        || (group->meth->field_decode &&
            !TEST_true(group->meth->field_decode(group, c, c, ctx)))
        || !TEST_true(BN_is_one(c)))
        goto err;

    /* 1/0 = error */
    BN_zero(a);
    if (!TEST_false(group->meth->field_inv(group, b, a, ctx))
        || !TEST_true(ERR_GET_LIB(ERR_peek_last_error()) == ERR_LIB_EC)
        || !TEST_true(ERR_GET_REASON(ERR_peek_last_error()) ==
                      EC_R_CANNOT_INVERT)
        /* 1/p = error */
        || !TEST_false(group->meth->field_inv(group, b, group->field, ctx))
        || !TEST_true(ERR_GET_LIB(ERR_peek_last_error()) == ERR_LIB_EC)
        || !TEST_true(ERR_GET_REASON(ERR_peek_last_error()) ==
                      EC_R_CANNOT_INVERT))
        goto err;

    ERR_clear_error();
    ret = 1;
 err:
    BN_CTX_end(ctx);
    return ret;
}
{
    BN_CTX *ctx = NULL;
    EC_GROUP *grp = NULL;
    EC_POINT *P = NULL, *Q = NULL, *R = NULL;
    BIGNUM *x1 = NULL, *y1 = NULL, *z1 = NULL, *x2 = NULL, *y2 = NULL;
    BIGNUM *k = NULL;
    int testresult = 0;
    const char *x1str =
        "1534f0077fffffe87e9adcfe000000000000000000003e05a21d2400002e031b1f4"
        "b80000c6fafa4f3c1288798d624a247b5e2ffffffffffffffefe099241900004";
    const char *p521m1 =
        "1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
        "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe";

    ctx = BN_CTX_new();
    if (!TEST_ptr(ctx))
        return 0;

    BN_CTX_start(ctx);
    x1 = BN_CTX_get(ctx);
    y1 = BN_CTX_get(ctx);
    z1 = BN_CTX_get(ctx);
    x2 = BN_CTX_get(ctx);
    y2 = BN_CTX_get(ctx);
    k = BN_CTX_get(ctx);
    if (!TEST_ptr(k))
        goto err;

    grp = EC_GROUP_new_by_curve_name(NID_secp521r1);
    P = EC_POINT_new(grp);
    Q = EC_POINT_new(grp);
    R = EC_POINT_new(grp);
    if (!TEST_ptr(grp) || !TEST_ptr(P) || !TEST_ptr(Q) || !TEST_ptr(R))
        goto err;

    if (!TEST_int_gt(BN_hex2bn(&x1, x1str), 0)
            || !TEST_int_gt(BN_hex2bn(&y1, p521m1), 0)
            || !TEST_int_gt(BN_hex2bn(&z1, p521m1), 0)
            || !TEST_int_gt(BN_hex2bn(&k, "02"), 0)
            || !TEST_true(ossl_ec_GFp_simple_set_Jprojective_coordinates_GFp(grp, P, x1,
                                                                             y1, z1, ctx))
            || !TEST_true(EC_POINT_mul(grp, Q, NULL, P, k, ctx))
            || !TEST_true(EC_POINT_get_affine_coordinates(grp, Q, x1, y1, ctx))
            || !TEST_true(EC_POINT_dbl(grp, R, P, ctx))
            || !TEST_true(EC_POINT_get_affine_coordinates(grp, R, x2, y2, ctx)))
        goto err;

    if (!TEST_int_eq(BN_cmp(x1, x2), 0)
            || !TEST_int_eq(BN_cmp(y1, y2), 0))
        goto err;

    testresult = 1;

 err:
    BN_CTX_end(ctx);
    EC_POINT_free(P);
    EC_POINT_free(Q);
    EC_POINT_free(R);
    EC_GROUP_free(grp);
    BN_CTX_free(ctx);

    return testresult;
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
{
    crv_len = EC_get_builtin_curves(NULL, 0);
    if (!TEST_ptr(curves = OPENSSL_malloc(sizeof(*curves) * crv_len))
        || !TEST_true(EC_get_builtin_curves(curves, crv_len)))
        return 0;

    ADD_TEST(field_tests_ecp_simple);
    ADD_TEST(field_tests_ecp_mont);
#ifndef OPENSSL_NO_EC2M
    ADD_TEST(field_tests_ec2_simple);
#endif
    ADD_ALL_TESTS(field_tests_default, crv_len);
#ifndef OPENSSL_NO_EC_NISTP_64_GCC_128
    ADD_TEST(underflow_test);
#endif
    ADD_TEST(set_private_key);
    ADD_TEST(decoded_flag_test);
    ADD_ALL_TESTS(ecpkparams_i2d2i_test, crv_len);

    return 1;
}

void cleanup_tests(void)
{
    OPENSSL_free(curves);
}