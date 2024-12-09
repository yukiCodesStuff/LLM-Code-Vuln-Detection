
static int default_libctx = 1;
static int is_fips = 0;

static OSSL_LIB_CTX *testctx = NULL;
static OSSL_LIB_CTX *keyctx = NULL;
static char *testpropq = NULL;
                             output_type, output_structure, pass, pcipher)))
        goto end;

    if ((flags & FLAG_FAIL_IF_FIPS) != 0 && is_fips) {
        if (TEST_false(decode_cb(file, line, (void **)&pkey2, encoded,
                                  encoded_len, output_type, output_structure,
                                  (flags & FLAG_DECODE_WITH_TYPE ? type : NULL),
                                  selection, pass)))
            return 0;
    }

    /* Separate provider/ctx for generating the test data */
    if (!TEST_ptr(keyctx = OSSL_LIB_CTX_new()))
        return 0;
    if (!TEST_ptr(keyprov = OSSL_PROVIDER_load(keyctx, "default")))