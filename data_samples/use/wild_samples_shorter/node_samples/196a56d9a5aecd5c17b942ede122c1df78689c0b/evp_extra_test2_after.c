#include <openssl/pem.h>
#include <openssl/provider.h>
#include <openssl/rsa.h>
#include <openssl/dh.h>
#include <openssl/core_names.h>

#include "testutil.h"
#include "internal/nelem.h"

static OSSL_LIB_CTX *mainctx = NULL;
    OSSL_PARAM params[2];
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *gctx = NULL;
# ifndef OPENSSL_NO_DEPRECATED_3_0
    const DH *dhkey;
    const BIGNUM *privkey;
# endif

    params[0] = OSSL_PARAM_construct_utf8_string("group", "ffdhe2048", 0);
    params[1] = OSSL_PARAM_construct_end();
    ret = TEST_ptr(gctx = EVP_PKEY_CTX_new_from_name(mainctx, "DHX", NULL))
          && TEST_true(EVP_PKEY_CTX_set_params(gctx, params))
          && TEST_int_gt(EVP_PKEY_generate(gctx, &key), 0)
          && TEST_true(do_pkey_tofrom_data_select(key, "DHX"));
# ifndef OPENSSL_NO_DEPRECATED_3_0
    ret = ret && TEST_ptr(dhkey = EVP_PKEY_get0_DH(key))
              && TEST_ptr(privkey = DH_get0_priv_key(dhkey))
              && TEST_int_le(BN_num_bits(privkey), 225);
# endif
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(gctx);
    return ret;
}
    return ret;
}

#if !defined OPENSSL_NO_DES && !defined OPENSSL_NO_MD5
static int test_evp_pbe_alg_add(void)
{
    int ret = 0;
    int cipher_nid = 0, md_nid = 0;
    EVP_PBE_KEYGEN_EX *keygen_ex = NULL;
    EVP_PBE_KEYGEN *keygen = NULL;

    if (!TEST_true(EVP_PBE_alg_add(NID_pbeWithMD5AndDES_CBC, EVP_des_cbc(), EVP_md5(),
                                   PKCS5_PBE_keyivgen)))
        goto err;

    if (!TEST_true(EVP_PBE_find_ex(EVP_PBE_TYPE_OUTER, NID_pbeWithMD5AndDES_CBC,
                                   &cipher_nid, &md_nid, &keygen, &keygen_ex)))
        goto err;

    if (!TEST_true(keygen != NULL))
        goto err;
    if (!TEST_true(keygen_ex == NULL))
        goto err;

    ret = 1;

err:
    return ret;
}
#endif

int setup_tests(void)
{
    if (!test_get_libctx(&mainctx, &nullprov, NULL, NULL, NULL)) {
        OSSL_LIB_CTX_free(mainctx);
    ADD_TEST(test_rsa_pss_sign);
    ADD_TEST(test_evp_md_ctx_copy);
    ADD_ALL_TESTS(test_provider_unload_effective, 2);
#if !defined OPENSSL_NO_DES && !defined OPENSSL_NO_MD5
    ADD_TEST(test_evp_pbe_alg_add);
#endif
    return 1;
}

void cleanup_tests(void)