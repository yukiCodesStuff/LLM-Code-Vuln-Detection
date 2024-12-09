            OPENSSL_free(allocated_load_dir);
        }

        if (prov->module == NULL) {
            /* DSO has already recorded errors, this is just a tracepoint */
            ERR_raise_data(ERR_LIB_CRYPTO, ERR_R_DSO_LIB,
                           "name=%s", prov->name);
            goto end;
        }

        prov->init_function = (OSSL_provider_init_fn *)
            DSO_bind_func(prov->module, "OSSL_provider_init");
#endif
    }

    /* Check for and call the initialise function for the provider. */
    if (prov->init_function == NULL) {
        ERR_raise_data(ERR_LIB_CRYPTO, ERR_R_UNSUPPORTED,
                       "name=%s, provider has no provider init function",
                       prov->name);
        goto end;
    }

    if (!prov->init_function((OSSL_CORE_HANDLE *)prov, core_dispatch,
                             &provider_dispatch, &tmp_provctx)) {
        ERR_raise_data(ERR_LIB_CRYPTO, ERR_R_INIT_FAIL,
                       "name=%s", prov->name);
        goto end;
    }
    if (!freeing) {
        int acc;

        if (!CRYPTO_THREAD_write_lock(prov->opbits_lock))
            return 0;
        OPENSSL_free(prov->operation_bits);
        prov->operation_bits = NULL;
        prov->operation_bits_sz = 0;

void *ossl_provider_ctx(const OSSL_PROVIDER *prov)
{
    return prov != NULL ? prov->provctx : NULL;
}

/*
 * This function only does something once when store->use_fallbacks == 1,
 */
static OSSL_FUNC_core_gettable_params_fn core_gettable_params;
static OSSL_FUNC_core_get_params_fn core_get_params;
static OSSL_FUNC_core_get_libctx_fn core_get_libctx;
static OSSL_FUNC_core_thread_start_fn core_thread_start;
#ifndef FIPS_MODULE
static OSSL_FUNC_core_new_error_fn core_new_error;
static OSSL_FUNC_core_set_error_debug_fn core_set_error_debug;
static OSSL_FUNC_core_vset_error_fn core_vset_error;
static OSSL_FUNC_core_set_error_mark_fn core_set_error_mark;
static OSSL_FUNC_core_clear_last_error_mark_fn core_clear_last_error_mark;
static OSSL_FUNC_core_pop_error_to_mark_fn core_pop_error_to_mark;
OSSL_FUNC_BIO_new_file_fn ossl_core_bio_new_file;
OSSL_FUNC_BIO_new_membuf_fn ossl_core_bio_new_mem_buf;
OSSL_FUNC_BIO_read_ex_fn ossl_core_bio_read_ex;
OSSL_FUNC_BIO_write_ex_fn ossl_core_bio_write_ex;
OSSL_FUNC_BIO_gets_fn ossl_core_bio_gets;
OSSL_FUNC_BIO_puts_fn ossl_core_bio_puts;
OSSL_FUNC_BIO_up_ref_fn ossl_core_bio_up_ref;
OSSL_FUNC_BIO_free_fn ossl_core_bio_free;
OSSL_FUNC_BIO_vprintf_fn ossl_core_bio_vprintf;
OSSL_FUNC_BIO_vsnprintf_fn BIO_vsnprintf;
static OSSL_FUNC_self_test_cb_fn core_self_test_get_callback;
OSSL_FUNC_get_entropy_fn ossl_rand_get_entropy;
OSSL_FUNC_cleanup_entropy_fn ossl_rand_cleanup_entropy;
OSSL_FUNC_get_nonce_fn ossl_rand_get_nonce;
OSSL_FUNC_cleanup_nonce_fn ossl_rand_cleanup_nonce;
#endif
OSSL_FUNC_CRYPTO_malloc_fn CRYPTO_malloc;
OSSL_FUNC_CRYPTO_zalloc_fn CRYPTO_zalloc;
OSSL_FUNC_CRYPTO_free_fn CRYPTO_free;
OSSL_FUNC_CRYPTO_clear_free_fn CRYPTO_clear_free;
OSSL_FUNC_CRYPTO_realloc_fn CRYPTO_realloc;
OSSL_FUNC_CRYPTO_clear_realloc_fn CRYPTO_clear_realloc;
OSSL_FUNC_CRYPTO_secure_malloc_fn CRYPTO_secure_malloc;
OSSL_FUNC_CRYPTO_secure_zalloc_fn CRYPTO_secure_zalloc;
OSSL_FUNC_CRYPTO_secure_free_fn CRYPTO_secure_free;
OSSL_FUNC_CRYPTO_secure_clear_free_fn CRYPTO_secure_clear_free;
OSSL_FUNC_CRYPTO_secure_allocated_fn CRYPTO_secure_allocated;
OSSL_FUNC_OPENSSL_cleanse_fn OPENSSL_cleanse;
#ifndef FIPS_MODULE
OSSL_FUNC_provider_register_child_cb_fn ossl_provider_register_child_cb;
OSSL_FUNC_provider_deregister_child_cb_fn ossl_provider_deregister_child_cb;
static OSSL_FUNC_provider_name_fn core_provider_get0_name;
static OSSL_FUNC_provider_get0_provider_ctx_fn core_provider_get0_provider_ctx;
static OSSL_FUNC_provider_get0_dispatch_fn core_provider_get0_dispatch;
static OSSL_FUNC_provider_up_ref_fn core_provider_up_ref_intern;
static OSSL_FUNC_provider_free_fn core_provider_free_intern;
static OSSL_FUNC_core_obj_add_sigid_fn core_obj_add_sigid;
static OSSL_FUNC_core_obj_create_fn core_obj_create;
#endif

    return ERR_pop_to_mark();
}

static void core_self_test_get_callback(OPENSSL_CORE_CTX *libctx,
                                        OSSL_CALLBACK **cb, void **cbarg)
{
    OSSL_SELF_TEST_get_callback((OSSL_LIB_CTX *)libctx, cb, cbarg);
}

static const char *core_provider_get0_name(const OSSL_CORE_HANDLE *prov)
{
    return OSSL_PROVIDER_get0_name((const OSSL_PROVIDER *)prov);
}

static void *core_provider_get0_provider_ctx(const OSSL_CORE_HANDLE *prov)
{
    return OSSL_PROVIDER_get0_provider_ctx((const OSSL_PROVIDER *)prov);
}

static const OSSL_DISPATCH *
core_provider_get0_dispatch(const OSSL_CORE_HANDLE *prov)
{
    return OSSL_PROVIDER_get0_dispatch((const OSSL_PROVIDER *)prov);
}

static int core_provider_up_ref_intern(const OSSL_CORE_HANDLE *prov,
                                       int activate)
{
    return provider_up_ref_intern((OSSL_PROVIDER *)prov, activate);
}

static int core_provider_free_intern(const OSSL_CORE_HANDLE *prov,
                                     int deactivate)
{
    return provider_free_intern((OSSL_PROVIDER *)prov, deactivate);
}

static int core_obj_add_sigid(const OSSL_CORE_HANDLE *prov,
                              const char *sign_name, const char *digest_name,
                              const char *pkey_name)
{
    { OSSL_FUNC_BIO_FREE, (void (*)(void))ossl_core_bio_free },
    { OSSL_FUNC_BIO_VPRINTF, (void (*)(void))ossl_core_bio_vprintf },
    { OSSL_FUNC_BIO_VSNPRINTF, (void (*)(void))BIO_vsnprintf },
    { OSSL_FUNC_SELF_TEST_CB, (void (*)(void))core_self_test_get_callback },
    { OSSL_FUNC_GET_ENTROPY, (void (*)(void))ossl_rand_get_entropy },
    { OSSL_FUNC_CLEANUP_ENTROPY, (void (*)(void))ossl_rand_cleanup_entropy },
    { OSSL_FUNC_GET_NONCE, (void (*)(void))ossl_rand_get_nonce },
    { OSSL_FUNC_CLEANUP_NONCE, (void (*)(void))ossl_rand_cleanup_nonce },
    { OSSL_FUNC_PROVIDER_DEREGISTER_CHILD_CB,
        (void (*)(void))ossl_provider_deregister_child_cb },
    { OSSL_FUNC_PROVIDER_NAME,
        (void (*)(void))core_provider_get0_name },
    { OSSL_FUNC_PROVIDER_GET0_PROVIDER_CTX,
        (void (*)(void))core_provider_get0_provider_ctx },
    { OSSL_FUNC_PROVIDER_GET0_DISPATCH,
        (void (*)(void))core_provider_get0_dispatch },
    { OSSL_FUNC_PROVIDER_UP_REF,
        (void (*)(void))core_provider_up_ref_intern },
    { OSSL_FUNC_PROVIDER_FREE,
        (void (*)(void))core_provider_free_intern },
    { OSSL_FUNC_CORE_OBJ_ADD_SIGID, (void (*)(void))core_obj_add_sigid },
    { OSSL_FUNC_CORE_OBJ_CREATE, (void (*)(void))core_obj_create },
#endif
    { 0, NULL }