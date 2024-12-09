/*
 * Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <string.h>
#include <openssl/core_dispatch.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/param_build.h>
#include <openssl/encoder.h>
#include <openssl/decoder.h>

#include "internal/cryptlib.h"   /* ossl_assert */
#include "crypto/pem.h"          /* For PVK and "blob" PEM headers */
#include "crypto/evp.h"          /* For evp_pkey_is_provided() */

#include "helpers/predefined_dhparams.h"
#include "testutil.h"

/* Extended test macros to allow passing file & line number */
#define TEST_FL_ptr(a)               test_ptr(file, line, #a, a)
#define TEST_FL_mem_eq(a, m, b, n)   test_mem_eq(file, line, #a, #b, a, m, b, n)
#define TEST_FL_strn_eq(a, b, n)     test_strn_eq(file, line, #a, #b, a, n, b, n)
#define TEST_FL_strn2_eq(a, m, b, n) test_strn_eq(file, line, #a, #b, a, m, b, n)
#define TEST_FL_int_eq(a, b)         test_int_eq(file, line, #a, #b, a, b)
#define TEST_FL_int_ge(a, b)         test_int_ge(file, line, #a, #b, a, b)
#define TEST_FL_int_gt(a, b)         test_int_gt(file, line, #a, #b, a, b)
#define TEST_FL_long_gt(a, b)        test_long_gt(file, line, #a, #b, a, b)
#define TEST_FL_true(a)              test_true(file, line, #a, (a) != 0)

#if defined(OPENSSL_NO_DH) && defined(OPENSSL_NO_DSA) && defined(OPENSSL_NO_EC)
# define OPENSSL_NO_KEYPARAMS
#endif

static int default_libctx = 1;
static int is_fips = 0;

static OSSL_LIB_CTX *testctx = NULL;
static OSSL_LIB_CTX *keyctx = NULL;
static char *testpropq = NULL;

static OSSL_PROVIDER *nullprov = NULL;
static OSSL_PROVIDER *deflprov = NULL;
static OSSL_PROVIDER *keyprov = NULL;

#ifndef OPENSSL_NO_EC
static BN_CTX *bnctx = NULL;
static OSSL_PARAM_BLD *bld_prime_nc = NULL;
static OSSL_PARAM_BLD *bld_prime = NULL;
static OSSL_PARAM *ec_explicit_prime_params_nc = NULL;
static OSSL_PARAM *ec_explicit_prime_params_explicit = NULL;

# ifndef OPENSSL_NO_EC2M
static OSSL_PARAM_BLD *bld_tri_nc = NULL;
static OSSL_PARAM_BLD *bld_tri = NULL;
static OSSL_PARAM *ec_explicit_tri_params_nc = NULL;
static OSSL_PARAM *ec_explicit_tri_params_explicit = NULL;
# endif
#endif

#ifndef OPENSSL_NO_KEYPARAMS
static EVP_PKEY *make_template(const char *type, OSSL_PARAM *genparams)
{
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *ctx = NULL;

# ifndef OPENSSL_NO_DH
    /*
     * Use 512-bit DH(X) keys with predetermined parameters for efficiency,
     * for testing only. Use a minimum key size of 2048 for security purposes.
     */
    if (strcmp(type, "DH") == 0)
        return get_dh512(keyctx);

    if (strcmp(type, "X9.42 DH") == 0)
        return get_dhx512(keyctx);
# endif

    /*
     * No real need to check the errors other than for the cascade
     * effect.  |pkey| will simply remain NULL if something goes wrong.
     */
    (void)((ctx = EVP_PKEY_CTX_new_from_name(keyctx, type, testpropq)) != NULL
           && EVP_PKEY_paramgen_init(ctx) > 0
           && (genparams == NULL
               || EVP_PKEY_CTX_set_params(ctx, genparams) > 0)
           && EVP_PKEY_generate(ctx, &pkey) > 0);
    EVP_PKEY_CTX_free(ctx);

    return pkey;
}
{
    void *encoded = NULL;
    long encoded_len = 0;
    EVP_PKEY *pkey2 = NULL;
    void *encoded2 = NULL;
    long encoded2_len = 0;
    int ok = 0;

    /*
     * Encode |pkey|, decode the result into |pkey2|, and finish off by
     * encoding |pkey2| as well.  That last encoding is for checking and
     * dumping purposes.
     */
    if (!TEST_true(encode_cb(file, line, &encoded, &encoded_len, pkey, selection,
                             output_type, output_structure, pass, pcipher)))
        goto end;

    if ((flags & FLAG_FAIL_IF_FIPS) != 0 && is_fips) {
        if (TEST_false(decode_cb(file, line, (void **)&pkey2, encoded,
                                  encoded_len, output_type, output_structure,
                                  (flags & FLAG_DECODE_WITH_TYPE ? type : NULL),
                                  selection, pass)))
            ok = 1;
        goto end;
    }
    } else {
        if (!test_get_libctx(&testctx, &nullprov, config_file, &deflprov, prov_name))
            return 0;
    }

    /* Separate provider/ctx for generating the test data */
    if (!TEST_ptr(keyctx = OSSL_LIB_CTX_new()))
        return 0;
    if (!TEST_ptr(keyprov = OSSL_PROVIDER_load(keyctx, "default")))
        return 0;

#ifndef OPENSSL_NO_EC
    if (!TEST_ptr(bnctx = BN_CTX_new_ex(testctx))
        || !TEST_ptr(bld_prime_nc = OSSL_PARAM_BLD_new())
        || !TEST_ptr(bld_prime = OSSL_PARAM_BLD_new())
        || !create_ec_explicit_prime_params_namedcurve(bld_prime_nc)
        || !create_ec_explicit_prime_params(bld_prime)
        || !TEST_ptr(ec_explicit_prime_params_nc = OSSL_PARAM_BLD_to_param(bld_prime_nc))
        || !TEST_ptr(ec_explicit_prime_params_explicit = OSSL_PARAM_BLD_to_param(bld_prime))
# ifndef OPENSSL_NO_EC2M
        || !TEST_ptr(bld_tri_nc = OSSL_PARAM_BLD_new())
        || !TEST_ptr(bld_tri = OSSL_PARAM_BLD_new())
        || !create_ec_explicit_trinomial_params_namedcurve(bld_tri_nc)
        || !create_ec_explicit_trinomial_params(bld_tri)
        || !TEST_ptr(ec_explicit_tri_params_nc = OSSL_PARAM_BLD_to_param(bld_tri_nc))
        || !TEST_ptr(ec_explicit_tri_params_explicit = OSSL_PARAM_BLD_to_param(bld_tri))
# endif
        )
        return 0;
#endif

    TEST_info("Generating keys...");

#ifndef OPENSSL_NO_DH
    TEST_info("Generating DH keys...");
    MAKE_DOMAIN_KEYS(DH, "DH", NULL);
    MAKE_DOMAIN_KEYS(DHX, "X9.42 DH", NULL);
#endif
#ifndef OPENSSL_NO_DSA
    TEST_info("Generating DSA keys...");
    MAKE_DOMAIN_KEYS(DSA, "DSA", DSA_params);
#endif
#ifndef OPENSSL_NO_EC
    TEST_info("Generating EC keys...");
    MAKE_DOMAIN_KEYS(EC, "EC", EC_params);
    MAKE_DOMAIN_KEYS(ECExplicitPrimeNamedCurve, "EC", ec_explicit_prime_params_nc);
    MAKE_DOMAIN_KEYS(ECExplicitPrime2G, "EC", ec_explicit_prime_params_explicit);
# ifndef OPENSSL_NO_EC2M
    MAKE_DOMAIN_KEYS(ECExplicitTriNamedCurve, "EC", ec_explicit_tri_params_nc);
    MAKE_DOMAIN_KEYS(ECExplicitTri2G, "EC", ec_explicit_tri_params_explicit);
# endif
    MAKE_KEYS(ED25519, "ED25519", NULL);
    MAKE_KEYS(ED448, "ED448", NULL);
    MAKE_KEYS(X25519, "X25519", NULL);
    MAKE_KEYS(X448, "X448", NULL);
#endif
    TEST_info("Loading RSA key...");
    ok = ok && TEST_ptr(key_RSA = load_pkey_pem(rsa_file, keyctx));
    TEST_info("Loading RSA_PSS key...");
    ok = ok && TEST_ptr(key_RSA_PSS = load_pkey_pem(rsa_pss_file, keyctx));
    TEST_info("Generating keys done");

    if (ok) {
#ifndef OPENSSL_NO_DH
        ADD_TEST_SUITE(DH);
        ADD_TEST_SUITE_PARAMS(DH);
        ADD_TEST_SUITE(DHX);
        ADD_TEST_SUITE_PARAMS(DHX);
        /*
         * DH has no support for PEM_write_bio_PrivateKey_traditional(),
         * so no legacy tests.
         */
#endif
#ifndef OPENSSL_NO_DSA
        ADD_TEST_SUITE(DSA);
        ADD_TEST_SUITE_PARAMS(DSA);
        ADD_TEST_SUITE_LEGACY(DSA);
        ADD_TEST_SUITE_MSBLOB(DSA);
        ADD_TEST_SUITE_UNPROTECTED_PVK(DSA);
# ifndef OPENSSL_NO_RC4
        ADD_TEST_SUITE_PROTECTED_PVK(DSA);
# endif
#endif
#ifndef OPENSSL_NO_EC
        ADD_TEST_SUITE(EC);
        ADD_TEST_SUITE_PARAMS(EC);
        ADD_TEST_SUITE_LEGACY(EC);
        ADD_TEST_SUITE(ECExplicitPrimeNamedCurve);
        ADD_TEST_SUITE_LEGACY(ECExplicitPrimeNamedCurve);
        ADD_TEST_SUITE(ECExplicitPrime2G);
        ADD_TEST_SUITE_LEGACY(ECExplicitPrime2G);
# ifndef OPENSSL_NO_EC2M
        ADD_TEST_SUITE(ECExplicitTriNamedCurve);
        ADD_TEST_SUITE_LEGACY(ECExplicitTriNamedCurve);
        ADD_TEST_SUITE(ECExplicitTri2G);
        ADD_TEST_SUITE_LEGACY(ECExplicitTri2G);
# endif
        ADD_TEST_SUITE(ED25519);
        ADD_TEST_SUITE(ED448);
        ADD_TEST_SUITE(X25519);
        ADD_TEST_SUITE(X448);
        /*
         * ED25519, ED448, X25519 and X448 have no support for
         * PEM_write_bio_PrivateKey_traditional(), so no legacy tests.
         */
#endif
        ADD_TEST_SUITE(RSA);
        ADD_TEST_SUITE_LEGACY(RSA);
        ADD_TEST_SUITE(RSA_PSS);
        /*
         * RSA-PSS has no support for PEM_write_bio_PrivateKey_traditional(),
         * so no legacy tests.
         */
        ADD_TEST_SUITE_MSBLOB(RSA);
        ADD_TEST_SUITE_UNPROTECTED_PVK(RSA);
# ifndef OPENSSL_NO_RC4
        ADD_TEST_SUITE_PROTECTED_PVK(RSA);
# endif
    }

    return 1;
}

void cleanup_tests(void)
{
#ifndef OPENSSL_NO_EC
    OSSL_PARAM_free(ec_explicit_prime_params_nc);
    OSSL_PARAM_free(ec_explicit_prime_params_explicit);
    OSSL_PARAM_BLD_free(bld_prime_nc);
    OSSL_PARAM_BLD_free(bld_prime);
# ifndef OPENSSL_NO_EC2M
    OSSL_PARAM_free(ec_explicit_tri_params_nc);
    OSSL_PARAM_free(ec_explicit_tri_params_explicit);
    OSSL_PARAM_BLD_free(bld_tri_nc);
    OSSL_PARAM_BLD_free(bld_tri);
# endif
    BN_CTX_free(bnctx);
#endif /* OPENSSL_NO_EC */

#ifndef OPENSSL_NO_DH
    FREE_DOMAIN_KEYS(DH);
    FREE_DOMAIN_KEYS(DHX);
#endif
#ifndef OPENSSL_NO_DSA
    FREE_DOMAIN_KEYS(DSA);
#endif
#ifndef OPENSSL_NO_EC
    FREE_DOMAIN_KEYS(EC);
    FREE_DOMAIN_KEYS(ECExplicitPrimeNamedCurve);
    FREE_DOMAIN_KEYS(ECExplicitPrime2G);
# ifndef OPENSSL_NO_EC2M
    FREE_DOMAIN_KEYS(ECExplicitTriNamedCurve);
    FREE_DOMAIN_KEYS(ECExplicitTri2G);
# endif
    FREE_KEYS(ED25519);
    FREE_KEYS(ED448);
    FREE_KEYS(X25519);
    FREE_KEYS(X448);
#endif
    FREE_KEYS(RSA);
    FREE_KEYS(RSA_PSS);

    OSSL_PROVIDER_unload(nullprov);
    OSSL_PROVIDER_unload(deflprov);
    OSSL_PROVIDER_unload(keyprov);
    OSSL_LIB_CTX_free(testctx);
    OSSL_LIB_CTX_free(keyctx);
}