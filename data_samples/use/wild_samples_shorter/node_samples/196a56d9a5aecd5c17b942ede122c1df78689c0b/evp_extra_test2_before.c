#include <openssl/pem.h>
#include <openssl/provider.h>
#include <openssl/rsa.h>
#include <openssl/core_names.h>
#include "testutil.h"
#include "internal/nelem.h"

static OSSL_LIB_CTX *mainctx = NULL;
    OSSL_PARAM params[2];
    EVP_PKEY *key = NULL;
    EVP_PKEY_CTX *gctx = NULL;

    params[0] = OSSL_PARAM_construct_utf8_string("group", "ffdhe2048", 0);
    params[1] = OSSL_PARAM_construct_end();
    ret = TEST_ptr(gctx = EVP_PKEY_CTX_new_from_name(mainctx, "DHX", NULL))
          && TEST_true(EVP_PKEY_CTX_set_params(gctx, params))
          && TEST_int_gt(EVP_PKEY_generate(gctx, &key), 0)
          && TEST_true(do_pkey_tofrom_data_select(key, "DHX"));
    EVP_PKEY_free(key);
    EVP_PKEY_CTX_free(gctx);
    return ret;
}
    return ret;
}

int setup_tests(void)
{
    if (!test_get_libctx(&mainctx, &nullprov, NULL, NULL, NULL)) {
        OSSL_LIB_CTX_free(mainctx);
    ADD_TEST(test_rsa_pss_sign);
    ADD_TEST(test_evp_md_ctx_copy);
    ADD_ALL_TESTS(test_provider_unload_effective, 2);
    return 1;
}

void cleanup_tests(void)