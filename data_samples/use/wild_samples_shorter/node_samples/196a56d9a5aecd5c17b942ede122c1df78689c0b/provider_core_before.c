            OPENSSL_free(allocated_load_dir);
        }

        if (prov->module != NULL)
            prov->init_function = (OSSL_provider_init_fn *)
                DSO_bind_func(prov->module, "OSSL_provider_init");
#endif
    }

    /* Call the initialise function for the provider. */
    if (prov->init_function == NULL
        || !prov->init_function((OSSL_CORE_HANDLE *)prov, core_dispatch,
                                &provider_dispatch, &tmp_provctx)) {
        ERR_raise_data(ERR_LIB_CRYPTO, ERR_R_INIT_FAIL,
                       "name=%s", prov->name);
        goto end;
    }
    if (!freeing) {
        int acc;

        if (!CRYPTO_THREAD_read_lock(prov->opbits_lock))
            return 0;
        OPENSSL_free(prov->operation_bits);
        prov->operation_bits = NULL;
        prov->operation_bits_sz = 0;

void *ossl_provider_ctx(const OSSL_PROVIDER *prov)
{
    return prov->provctx;
}

/*
 * This function only does something once when store->use_fallbacks == 1,
 */
static OSSL_FUNC_core_gettable_params_fn core_gettable_params;
static OSSL_FUNC_core_get_params_fn core_get_params;
static OSSL_FUNC_core_thread_start_fn core_thread_start;
static OSSL_FUNC_core_get_libctx_fn core_get_libctx;
#ifndef FIPS_MODULE
static OSSL_FUNC_core_new_error_fn core_new_error;
static OSSL_FUNC_core_set_error_debug_fn core_set_error_debug;
static OSSL_FUNC_core_vset_error_fn core_vset_error;
static OSSL_FUNC_core_set_error_mark_fn core_set_error_mark;
static OSSL_FUNC_core_clear_last_error_mark_fn core_clear_last_error_mark;
static OSSL_FUNC_core_pop_error_to_mark_fn core_pop_error_to_mark;
static OSSL_FUNC_core_obj_add_sigid_fn core_obj_add_sigid;
static OSSL_FUNC_core_obj_create_fn core_obj_create;
#endif

    return ERR_pop_to_mark();
}

static int core_obj_add_sigid(const OSSL_CORE_HANDLE *prov,
                              const char *sign_name, const char *digest_name,
                              const char *pkey_name)
{
    { OSSL_FUNC_BIO_FREE, (void (*)(void))ossl_core_bio_free },
    { OSSL_FUNC_BIO_VPRINTF, (void (*)(void))ossl_core_bio_vprintf },
    { OSSL_FUNC_BIO_VSNPRINTF, (void (*)(void))BIO_vsnprintf },
    { OSSL_FUNC_SELF_TEST_CB, (void (*)(void))OSSL_SELF_TEST_get_callback },
    { OSSL_FUNC_GET_ENTROPY, (void (*)(void))ossl_rand_get_entropy },
    { OSSL_FUNC_CLEANUP_ENTROPY, (void (*)(void))ossl_rand_cleanup_entropy },
    { OSSL_FUNC_GET_NONCE, (void (*)(void))ossl_rand_get_nonce },
    { OSSL_FUNC_CLEANUP_NONCE, (void (*)(void))ossl_rand_cleanup_nonce },
    { OSSL_FUNC_PROVIDER_DEREGISTER_CHILD_CB,
        (void (*)(void))ossl_provider_deregister_child_cb },
    { OSSL_FUNC_PROVIDER_NAME,
        (void (*)(void))OSSL_PROVIDER_get0_name },
    { OSSL_FUNC_PROVIDER_GET0_PROVIDER_CTX,
        (void (*)(void))OSSL_PROVIDER_get0_provider_ctx },
    { OSSL_FUNC_PROVIDER_GET0_DISPATCH,
        (void (*)(void))OSSL_PROVIDER_get0_dispatch },
    { OSSL_FUNC_PROVIDER_UP_REF,
        (void (*)(void))provider_up_ref_intern },
    { OSSL_FUNC_PROVIDER_FREE,
        (void (*)(void))provider_free_intern },
    { OSSL_FUNC_CORE_OBJ_ADD_SIGID, (void (*)(void))core_obj_add_sigid },
    { OSSL_FUNC_CORE_OBJ_CREATE, (void (*)(void))core_obj_create },
#endif
    { 0, NULL }