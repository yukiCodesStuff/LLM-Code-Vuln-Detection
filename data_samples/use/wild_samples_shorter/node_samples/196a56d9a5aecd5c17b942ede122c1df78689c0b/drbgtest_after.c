
/*
 * When building the FIPS module, it isn't possible to disable the continuous
 * RNG tests.  Tests that require this are skipped and this means a detection
 * mechanism for the FIPS provider being in use.
 */
static int using_fips_rng(void)
{
    EVP_RAND_CTX *primary = RAND_get0_primary(NULL);
    const OSSL_PROVIDER *prov;
    const char *name;

    if (!TEST_ptr(primary))
        return 0;

    prov = EVP_RAND_get0_provider(EVP_RAND_CTX_get0_rand(primary));
    if (!TEST_ptr(prov))
        return 0;
    name = OSSL_PROVIDER_get0_name(prov);
    return strcmp(name, "OpenSSL FIPS Provider") == 0;
}

 /*
 * Disable CRNG testing if it is enabled.
}


#if defined(OPENSSL_SYS_UNIX) && !defined(OPENSSL_RAND_SEED_EGD)
/* number of children to fork */
#define DRBG_FORK_COUNT 9
/* two results per child, two for the parent */
#define DRBG_FORK_RESULT_COUNT (2 * (DRBG_FORK_COUNT + 1))

/*
 * Test whether the default rand_method (RAND_OpenSSL()) is
 * setup correctly, in particular whether reseeding works
 * as designed.
 */
static int test_rand_reseed(void)
{
    int rv = 0;
    time_t before_reseed;

    if (using_fips_rng())
        return TEST_skip("CRNGT cannot be disabled");

#ifndef OPENSSL_NO_DEPRECATED_3_0
    /* Check whether RAND_OpenSSL() is the default method */
    EVP_RAND_uninstantiate(private);
    EVP_RAND_uninstantiate(public);

    /*
     * Test initial seeding of shared DRBGs
     */
    if (!TEST_true(test_drbg_reseed(1,
                                    1, 1, 1, 0)))
        goto error;

    /*
     * Test initial state of shared DRBGs
     */
    if (!TEST_true(test_drbg_reseed(1,
    /* fill 'randomness' buffer with some arbitrary data */
    memset(rand_add_buf, 'r', sizeof(rand_add_buf));

    /*
     * Test whether all three DRBGs are reseeded by RAND_add().
     * The before_reseed time has to be measured here and passed into the
     * test_drbg_reseed() test, because the primary DRBG gets already reseeded
                                    1, 1, 1,
                                    before_reseed)))
        goto error;

    rv = 1;

error:
    unsigned char buf1[51], buf2[sizeof(buf1)];
    int ret = 0, xreseed, yreseed, zreseed;

    if (using_fips_rng())
        return TEST_skip("CRNGT cannot be disabled");

    /* Initialise a three long DRBG chain */
    if (!TEST_ptr(x = new_drbg(NULL))
int setup_tests(void)
{
    ADD_TEST(test_rand_reseed);
#if defined(OPENSSL_SYS_UNIX) && !defined(OPENSSL_RAND_SEED_EGD)
    ADD_ALL_TESTS(test_rand_fork_safety, RANDOM_SIZE);
#endif
    ADD_TEST(test_rand_prediction_resistance);
#if defined(OPENSSL_THREADS)