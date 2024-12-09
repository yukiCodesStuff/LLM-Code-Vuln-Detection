                || (DSO_load(prov->module, merged_path, NULL, 0)) == NULL) {
                DSO_free(prov->module);
                prov->module = NULL;
            }

            OPENSSL_free(merged_path);
            OPENSSL_free(allocated_path);
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
    prov->provctx = tmp_provctx;
    prov->dispatch = provider_dispatch;

    for (; provider_dispatch->function_id != 0; provider_dispatch++) {
        switch (provider_dispatch->function_id) {
        case OSSL_FUNC_PROVIDER_TEARDOWN:
            prov->teardown =
                OSSL_FUNC_provider_teardown(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_GETTABLE_PARAMS:
            prov->gettable_params =
                OSSL_FUNC_provider_gettable_params(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_GET_PARAMS:
            prov->get_params =
                OSSL_FUNC_provider_get_params(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_SELF_TEST:
            prov->self_test =
                OSSL_FUNC_provider_self_test(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_GET_CAPABILITIES:
            prov->get_capabilities =
                OSSL_FUNC_provider_get_capabilities(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_QUERY_OPERATION:
            prov->query_operation =
                OSSL_FUNC_provider_query_operation(provider_dispatch);
            break;
        case OSSL_FUNC_PROVIDER_UNQUERY_OPERATION:
            prov->unquery_operation =
                OSSL_FUNC_provider_unquery_operation(provider_dispatch);
            break;
#ifndef OPENSSL_NO_ERR
# ifndef FIPS_MODULE
        case OSSL_FUNC_PROVIDER_GET_REASON_STRINGS:
            p_get_reason_strings =
                OSSL_FUNC_provider_get_reason_strings(provider_dispatch);
            break;
# endif
#endif
        }
    }

#ifndef OPENSSL_NO_ERR
# ifndef FIPS_MODULE
    if (p_get_reason_strings != NULL) {
        const OSSL_ITEM *reasonstrings = p_get_reason_strings(prov->provctx);
        size_t cnt, cnt2;

        /*
         * ERR_load_strings() handles ERR_STRING_DATA rather than OSSL_ITEM,
         * although they are essentially the same type.
         * Furthermore, ERR_load_strings() patches the array's error number
         * with the error library number, so we need to make a copy of that
         * array either way.
         */
        cnt = 0;
        while (reasonstrings[cnt].id != 0) {
            if (ERR_GET_LIB(reasonstrings[cnt].id) != 0)
                goto end;
            cnt++;
        }
        cnt++;                   /* One for the terminating item */

        /* Allocate one extra item for the "library" name */
        prov->error_strings =
            OPENSSL_zalloc(sizeof(ERR_STRING_DATA) * (cnt + 1));
        if (prov->error_strings == NULL)
            goto end;

        /*
         * Set the "library" name.
         */
        prov->error_strings[0].error = ERR_PACK(prov->error_lib, 0, 0);
        prov->error_strings[0].string = prov->name;
        /*
         * Copy reasonstrings item 0..cnt-1 to prov->error_trings positions
         * 1..cnt.
         */
        for (cnt2 = 1; cnt2 <= cnt; cnt2++) {
            prov->error_strings[cnt2].error = (int)reasonstrings[cnt2-1].id;
            prov->error_strings[cnt2].string = reasonstrings[cnt2-1].ptr;
        }

        ERR_load_strings(prov->error_lib, prov->error_strings);
    }
# endif
#endif

    /* With this flag set, this provider has become fully "loaded". */
    prov->flag_initialized = 1;
    ok = 1;

 end:
    return ok;
}

/*
 * Deactivate a provider. If upcalls is 0 then we suppress any upcalls to a
 * parent provider. If removechildren is 0 then we suppress any calls to remove
 * child providers.
 * Return -1 on failure and the activation count on success
 */
static int provider_deactivate(OSSL_PROVIDER *prov, int upcalls,
                               int removechildren)
{
    int count;
    struct provider_store_st *store;
#ifndef FIPS_MODULE
    int freeparent = 0;
#endif
    int lock = 1;

    if (!ossl_assert(prov != NULL))
        return -1;

    /*
     * No need to lock if we've got no store because we've not been shared with
     * other threads.
     */
    store = get_provider_store(prov->libctx);
    if (store == NULL)
        lock = 0;

    if (lock && !CRYPTO_THREAD_read_lock(store->lock))
        return -1;
    if (lock && !CRYPTO_THREAD_write_lock(prov->flag_lock)) {
        CRYPTO_THREAD_unlock(store->lock);
        return -1;
    }

#ifndef FIPS_MODULE
    if (prov->activatecnt >= 2 && prov->ischild && upcalls) {
        /*
         * We have had a direct activation in this child libctx so we need to
         * now down the ref count in the parent provider. We do the actual down
         * ref outside of the flag_lock, since it could involve getting other
         * locks.
         */
        freeparent = 1;
    }
#endif

    if ((count = --prov->activatecnt) < 1)
        prov->flag_activated = 0;
#ifndef FIPS_MODULE
    else
        removechildren = 0;
#endif

#ifndef FIPS_MODULE
    if (removechildren && store != NULL) {
        int i, max = sk_OSSL_PROVIDER_CHILD_CB_num(store->child_cbs);
        OSSL_PROVIDER_CHILD_CB *child_cb;

        for (i = 0; i < max; i++) {
            child_cb = sk_OSSL_PROVIDER_CHILD_CB_value(store->child_cbs, i);
            child_cb->remove_cb((OSSL_CORE_HANDLE *)prov, child_cb->cbdata);
        }
    }
#endif
    if (lock) {
        CRYPTO_THREAD_unlock(prov->flag_lock);
        CRYPTO_THREAD_unlock(store->lock);
    }
#ifndef FIPS_MODULE
    if (freeparent)
        ossl_provider_free_parent(prov, 1);
#endif

    /* We don't deinit here, that's done in ossl_provider_free() */
    return count;
}

/*
 * Activate a provider.
 * Return -1 on failure and the activation count on success
 */
static int provider_activate(OSSL_PROVIDER *prov, int lock, int upcalls)
{
    int count = -1;
    struct provider_store_st *store;
    int ret = 1;

    store = prov->store;
    /*
    * If the provider hasn't been added to the store, then we don't need
    * any locks because we've not shared it with other threads.
    */
    if (store == NULL) {
        lock = 0;
        if (!provider_init(prov))
            return -1;
    }

#ifndef FIPS_MODULE
    if (prov->ischild && upcalls && !ossl_provider_up_ref_parent(prov, 1))
        return -1;
#endif

    if (lock && !CRYPTO_THREAD_read_lock(store->lock)) {
#ifndef FIPS_MODULE
        if (prov->ischild && upcalls)
            ossl_provider_free_parent(prov, 1);
#endif
        return -1;
    }

    if (lock && !CRYPTO_THREAD_write_lock(prov->flag_lock)) {
        CRYPTO_THREAD_unlock(store->lock);
#ifndef FIPS_MODULE
        if (prov->ischild && upcalls)
            ossl_provider_free_parent(prov, 1);
#endif
        return -1;
    }

    count = ++prov->activatecnt;
    prov->flag_activated = 1;

    if (prov->activatecnt == 1 && store != NULL) {
        ret = create_provider_children(prov);
    }
    if (lock) {
        CRYPTO_THREAD_unlock(prov->flag_lock);
        CRYPTO_THREAD_unlock(store->lock);
    }

    if (!ret)
        return -1;

    return count;
}

static int provider_flush_store_cache(const OSSL_PROVIDER *prov)
{
    struct provider_store_st *store;
    int freeing;

    if ((store = get_provider_store(prov->libctx)) == NULL)
        return 0;

    if (!CRYPTO_THREAD_read_lock(store->lock))
        return 0;
    freeing = store->freeing;
    CRYPTO_THREAD_unlock(store->lock);

    if (!freeing) {
        int acc
            = evp_method_store_cache_flush(prov->libctx)
#ifndef FIPS_MODULE
            + ossl_encoder_store_cache_flush(prov->libctx)
            + ossl_decoder_store_cache_flush(prov->libctx)
            + ossl_store_loader_store_cache_flush(prov->libctx)
#endif
            ;

#ifndef FIPS_MODULE
        return acc == 4;
#else
        return acc == 1;
#endif
    }
    return 1;
}

static int provider_remove_store_methods(OSSL_PROVIDER *prov)
{
    struct provider_store_st *store;
    int freeing;

    if ((store = get_provider_store(prov->libctx)) == NULL)
        return 0;

    if (!CRYPTO_THREAD_read_lock(store->lock))
        return 0;
    freeing = store->freeing;
    CRYPTO_THREAD_unlock(store->lock);

    if (!freeing) {
        int acc;

        if (!CRYPTO_THREAD_read_lock(prov->opbits_lock))
            return 0;
        OPENSSL_free(prov->operation_bits);
        prov->operation_bits = NULL;
        prov->operation_bits_sz = 0;
        CRYPTO_THREAD_unlock(prov->opbits_lock);

        acc = evp_method_store_remove_all_provided(prov)
#ifndef FIPS_MODULE
            + ossl_encoder_store_remove_all_provided(prov)
            + ossl_decoder_store_remove_all_provided(prov)
            + ossl_store_loader_store_remove_all_provided(prov)
#endif
            ;

#ifndef FIPS_MODULE
        return acc == 4;
#else
        return acc == 1;
#endif
    }
    return 1;
}

int ossl_provider_activate(OSSL_PROVIDER *prov, int upcalls, int aschild)
{
    int count;

    if (prov == NULL)
        return 0;
#ifndef FIPS_MODULE
    /*
     * If aschild is true, then we only actually do the activation if the
     * provider is a child. If its not, this is still success.
     */
    if (aschild && !prov->ischild)
        return 1;
#endif
    if ((count = provider_activate(prov, 1, upcalls)) > 0)
        return count == 1 ? provider_flush_store_cache(prov) : 1;

    return 0;
}

int ossl_provider_deactivate(OSSL_PROVIDER *prov, int removechildren)
{
    int count;

    if (prov == NULL
            || (count = provider_deactivate(prov, 1, removechildren)) < 0)
        return 0;
    return count == 0 ? provider_remove_store_methods(prov) : 1;
}

void *ossl_provider_ctx(const OSSL_PROVIDER *prov)
{
    return prov->provctx;
}

/*
 * This function only does something once when store->use_fallbacks == 1,
 * and then sets store->use_fallbacks = 0, so the second call and so on is
 * effectively a no-op.
 */
static int provider_activate_fallbacks(struct provider_store_st *store)
{
    int use_fallbacks;
    int activated_fallback_count = 0;
    int ret = 0;
    const OSSL_PROVIDER_INFO *p;

    if (!CRYPTO_THREAD_read_lock(store->lock))
        return 0;
    use_fallbacks = store->use_fallbacks;
    CRYPTO_THREAD_unlock(store->lock);
    if (!use_fallbacks)
        return 1;

    if (!CRYPTO_THREAD_write_lock(store->lock))
        return 0;
    /* Check again, just in case another thread changed it */
    use_fallbacks = store->use_fallbacks;
    if (!use_fallbacks) {
        CRYPTO_THREAD_unlock(store->lock);
        return 1;
    }

    for (p = ossl_predefined_providers; p->name != NULL; p++) {
        OSSL_PROVIDER *prov = NULL;

        if (!p->is_fallback)
            continue;
        /*
         * We use the internal constructor directly here,
         * otherwise we get a call loop
         */
        prov = provider_new(p->name, p->init, NULL);
        if (prov == NULL)
            goto err;
        prov->libctx = store->libctx;
#ifndef FIPS_MODULE
        prov->error_lib = ERR_get_next_error_library();
#endif

        /*
         * We are calling provider_activate while holding the store lock. This
         * means the init function will be called while holding a lock. Normally
         * we try to avoid calling a user callback while holding a lock.
         * However, fallbacks are never third party providers so we accept this.
         */
        if (provider_activate(prov, 0, 0) < 0) {
            ossl_provider_free(prov);
            goto err;
        }
        prov->store = store;
        if (sk_OSSL_PROVIDER_push(store->providers, prov) == 0) {
            ossl_provider_free(prov);
            goto err;
        }
        activated_fallback_count++;
    }

    if (activated_fallback_count > 0) {
        store->use_fallbacks = 0;
        ret = 1;
    }
 err:
    CRYPTO_THREAD_unlock(store->lock);
    return ret;
}

int ossl_provider_doall_activated(OSSL_LIB_CTX *ctx,
                                  int (*cb)(OSSL_PROVIDER *provider,
                                            void *cbdata),
                                  void *cbdata)
{
    int ret = 0, curr, max, ref = 0;
    struct provider_store_st *store = get_provider_store(ctx);
    STACK_OF(OSSL_PROVIDER) *provs = NULL;

#ifndef FIPS_MODULE
    /*
     * Make sure any providers are loaded from config before we try to use
     * them.
     */
    if (ossl_lib_ctx_is_default(ctx))
        OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CONFIG, NULL);
#endif

    if (store == NULL)
        return 1;
    if (!provider_activate_fallbacks(store))
        return 0;

    /*
     * Under lock, grab a copy of the provider list and up_ref each
     * provider so that they don't disappear underneath us.
     */
    if (!CRYPTO_THREAD_read_lock(store->lock))
        return 0;
    provs = sk_OSSL_PROVIDER_dup(store->providers);
    if (provs == NULL) {
        CRYPTO_THREAD_unlock(store->lock);
        return 0;
    }
    max = sk_OSSL_PROVIDER_num(provs);
    /*
     * We work backwards through the stack so that we can safely delete items
     * as we go.
     */
    for (curr = max - 1; curr >= 0; curr--) {
        OSSL_PROVIDER *prov = sk_OSSL_PROVIDER_value(provs, curr);

        if (!CRYPTO_THREAD_write_lock(prov->flag_lock))
            goto err_unlock;
        if (prov->flag_activated) {
            /*
             * We call CRYPTO_UP_REF directly rather than ossl_provider_up_ref
             * to avoid upping the ref count on the parent provider, which we
             * must not do while holding locks.
             */
            if (CRYPTO_UP_REF(&prov->refcnt, &ref, prov->refcnt_lock) <= 0) {
                CRYPTO_THREAD_unlock(prov->flag_lock);
                goto err_unlock;
            }
    if (!freeing) {
        int acc;

        if (!CRYPTO_THREAD_read_lock(prov->opbits_lock))
            return 0;
        OPENSSL_free(prov->operation_bits);
        prov->operation_bits = NULL;
        prov->operation_bits_sz = 0;
        CRYPTO_THREAD_unlock(prov->opbits_lock);

        acc = evp_method_store_remove_all_provided(prov)
#ifndef FIPS_MODULE
            + ossl_encoder_store_remove_all_provided(prov)
            + ossl_decoder_store_remove_all_provided(prov)
            + ossl_store_loader_store_remove_all_provided(prov)
#endif
            ;

#ifndef FIPS_MODULE
        return acc == 4;
#else
        return acc == 1;
#endif
    }
    return 1;
}

int ossl_provider_activate(OSSL_PROVIDER *prov, int upcalls, int aschild)
{
    int count;

    if (prov == NULL)
        return 0;
#ifndef FIPS_MODULE
    /*
     * If aschild is true, then we only actually do the activation if the
     * provider is a child. If its not, this is still success.
     */
    if (aschild && !prov->ischild)
        return 1;
#endif
    if ((count = provider_activate(prov, 1, upcalls)) > 0)
        return count == 1 ? provider_flush_store_cache(prov) : 1;

    return 0;
}

int ossl_provider_deactivate(OSSL_PROVIDER *prov, int removechildren)
{
    int count;

    if (prov == NULL
            || (count = provider_deactivate(prov, 1, removechildren)) < 0)
        return 0;
    return count == 0 ? provider_remove_store_methods(prov) : 1;
}

void *ossl_provider_ctx(const OSSL_PROVIDER *prov)
{
    return prov->provctx;
}

/*
 * This function only does something once when store->use_fallbacks == 1,
 * and then sets store->use_fallbacks = 0, so the second call and so on is
 * effectively a no-op.
 */
static int provider_activate_fallbacks(struct provider_store_st *store)
{
    int use_fallbacks;
    int activated_fallback_count = 0;
    int ret = 0;
    const OSSL_PROVIDER_INFO *p;

    if (!CRYPTO_THREAD_read_lock(store->lock))
        return 0;
    use_fallbacks = store->use_fallbacks;
    CRYPTO_THREAD_unlock(store->lock);
    if (!use_fallbacks)
        return 1;

    if (!CRYPTO_THREAD_write_lock(store->lock))
        return 0;
    /* Check again, just in case another thread changed it */
    use_fallbacks = store->use_fallbacks;
    if (!use_fallbacks) {
        CRYPTO_THREAD_unlock(store->lock);
        return 1;
    }

    for (p = ossl_predefined_providers; p->name != NULL; p++) {
        OSSL_PROVIDER *prov = NULL;

        if (!p->is_fallback)
            continue;
        /*
         * We use the internal constructor directly here,
         * otherwise we get a call loop
         */
        prov = provider_new(p->name, p->init, NULL);
        if (prov == NULL)
            goto err;
        prov->libctx = store->libctx;
#ifndef FIPS_MODULE
        prov->error_lib = ERR_get_next_error_library();
#endif

        /*
         * We are calling provider_activate while holding the store lock. This
         * means the init function will be called while holding a lock. Normally
         * we try to avoid calling a user callback while holding a lock.
         * However, fallbacks are never third party providers so we accept this.
         */
        if (provider_activate(prov, 0, 0) < 0) {
            ossl_provider_free(prov);
            goto err;
        }
    return count == 0 ? provider_remove_store_methods(prov) : 1;
}

void *ossl_provider_ctx(const OSSL_PROVIDER *prov)
{
    return prov->provctx;
}
static const OSSL_PARAM param_types[] = {
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_CORE_VERSION, OSSL_PARAM_UTF8_PTR, NULL, 0),
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_CORE_PROV_NAME, OSSL_PARAM_UTF8_PTR,
                    NULL, 0),
#ifndef FIPS_MODULE
    OSSL_PARAM_DEFN(OSSL_PROV_PARAM_CORE_MODULE_FILENAME, OSSL_PARAM_UTF8_PTR,
                    NULL, 0),
#endif
    OSSL_PARAM_END
};

/*
 * Forward declare all the functions that are provided aa dispatch.
 * This ensures that the compiler will complain if they aren't defined
 * with the correct signature.
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

static const OSSL_PARAM *core_gettable_params(const OSSL_CORE_HANDLE *handle)
{
    return param_types;
}
{
    return ERR_pop_to_mark();
}

static int core_obj_add_sigid(const OSSL_CORE_HANDLE *prov,
                              const char *sign_name, const char *digest_name,
                              const char *pkey_name)
{
    int sign_nid = OBJ_txt2nid(sign_name);
    int digest_nid = NID_undef;
    int pkey_nid = OBJ_txt2nid(pkey_name);

    if (digest_name != NULL && digest_name[0] != '\0'
        && (digest_nid = OBJ_txt2nid(digest_name)) == NID_undef)
            return 0;

    if (sign_nid == NID_undef)
        return 0;

    /*
     * Check if it already exists. This is a success if so (even if we don't
     * have nids for the digest/pkey)
     */
    if (OBJ_find_sigid_algs(sign_nid, NULL, NULL))
        return 1;

    if (pkey_nid == NID_undef)
        return 0;

    return OBJ_add_sigid(sign_nid, digest_nid, pkey_nid);
}

static int core_obj_create(const OSSL_CORE_HANDLE *prov, const char *oid,
                           const char *sn, const char *ln)
{
    /* Check if it already exists and create it if not */
    return OBJ_txt2nid(oid) != NID_undef
           || OBJ_create(oid, sn, ln) != NID_undef;
}
#endif /* FIPS_MODULE */

/*
 * Functions provided by the core.
 */
static const OSSL_DISPATCH core_dispatch_[] = {
    { OSSL_FUNC_CORE_GETTABLE_PARAMS, (void (*)(void))core_gettable_params },
    { OSSL_FUNC_CORE_GET_PARAMS, (void (*)(void))core_get_params },
    { OSSL_FUNC_CORE_GET_LIBCTX, (void (*)(void))core_get_libctx },
    { OSSL_FUNC_CORE_THREAD_START, (void (*)(void))core_thread_start },
#ifndef FIPS_MODULE
    { OSSL_FUNC_CORE_NEW_ERROR, (void (*)(void))core_new_error },
    { OSSL_FUNC_CORE_SET_ERROR_DEBUG, (void (*)(void))core_set_error_debug },
    { OSSL_FUNC_CORE_VSET_ERROR, (void (*)(void))core_vset_error },
    { OSSL_FUNC_CORE_SET_ERROR_MARK, (void (*)(void))core_set_error_mark },
    { OSSL_FUNC_CORE_CLEAR_LAST_ERROR_MARK,
      (void (*)(void))core_clear_last_error_mark },
    { OSSL_FUNC_CORE_POP_ERROR_TO_MARK, (void (*)(void))core_pop_error_to_mark },
    { OSSL_FUNC_BIO_NEW_FILE, (void (*)(void))ossl_core_bio_new_file },
    { OSSL_FUNC_BIO_NEW_MEMBUF, (void (*)(void))ossl_core_bio_new_mem_buf },
    { OSSL_FUNC_BIO_READ_EX, (void (*)(void))ossl_core_bio_read_ex },
    { OSSL_FUNC_BIO_WRITE_EX, (void (*)(void))ossl_core_bio_write_ex },
    { OSSL_FUNC_BIO_GETS, (void (*)(void))ossl_core_bio_gets },
    { OSSL_FUNC_BIO_PUTS, (void (*)(void))ossl_core_bio_puts },
    { OSSL_FUNC_BIO_CTRL, (void (*)(void))ossl_core_bio_ctrl },
    { OSSL_FUNC_BIO_UP_REF, (void (*)(void))ossl_core_bio_up_ref },
    { OSSL_FUNC_BIO_FREE, (void (*)(void))ossl_core_bio_free },
    { OSSL_FUNC_BIO_VPRINTF, (void (*)(void))ossl_core_bio_vprintf },
    { OSSL_FUNC_BIO_VSNPRINTF, (void (*)(void))BIO_vsnprintf },
    { OSSL_FUNC_SELF_TEST_CB, (void (*)(void))OSSL_SELF_TEST_get_callback },
    { OSSL_FUNC_GET_ENTROPY, (void (*)(void))ossl_rand_get_entropy },
    { OSSL_FUNC_CLEANUP_ENTROPY, (void (*)(void))ossl_rand_cleanup_entropy },
    { OSSL_FUNC_GET_NONCE, (void (*)(void))ossl_rand_get_nonce },
    { OSSL_FUNC_CLEANUP_NONCE, (void (*)(void))ossl_rand_cleanup_nonce },
#endif
    { OSSL_FUNC_CRYPTO_MALLOC, (void (*)(void))CRYPTO_malloc },
    { OSSL_FUNC_CRYPTO_ZALLOC, (void (*)(void))CRYPTO_zalloc },
    { OSSL_FUNC_CRYPTO_FREE, (void (*)(void))CRYPTO_free },
    { OSSL_FUNC_CRYPTO_CLEAR_FREE, (void (*)(void))CRYPTO_clear_free },
    { OSSL_FUNC_CRYPTO_REALLOC, (void (*)(void))CRYPTO_realloc },
    { OSSL_FUNC_CRYPTO_CLEAR_REALLOC, (void (*)(void))CRYPTO_clear_realloc },
    { OSSL_FUNC_CRYPTO_SECURE_MALLOC, (void (*)(void))CRYPTO_secure_malloc },
    { OSSL_FUNC_CRYPTO_SECURE_ZALLOC, (void (*)(void))CRYPTO_secure_zalloc },
    { OSSL_FUNC_CRYPTO_SECURE_FREE, (void (*)(void))CRYPTO_secure_free },
    { OSSL_FUNC_CRYPTO_SECURE_CLEAR_FREE,
        (void (*)(void))CRYPTO_secure_clear_free },
    { OSSL_FUNC_CRYPTO_SECURE_ALLOCATED,
        (void (*)(void))CRYPTO_secure_allocated },
    { OSSL_FUNC_OPENSSL_CLEANSE, (void (*)(void))OPENSSL_cleanse },
#ifndef FIPS_MODULE
    { OSSL_FUNC_PROVIDER_REGISTER_CHILD_CB,
        (void (*)(void))ossl_provider_register_child_cb },
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
};
static const OSSL_DISPATCH *core_dispatch = core_dispatch_;
static const OSSL_DISPATCH core_dispatch_[] = {
    { OSSL_FUNC_CORE_GETTABLE_PARAMS, (void (*)(void))core_gettable_params },
    { OSSL_FUNC_CORE_GET_PARAMS, (void (*)(void))core_get_params },
    { OSSL_FUNC_CORE_GET_LIBCTX, (void (*)(void))core_get_libctx },
    { OSSL_FUNC_CORE_THREAD_START, (void (*)(void))core_thread_start },
#ifndef FIPS_MODULE
    { OSSL_FUNC_CORE_NEW_ERROR, (void (*)(void))core_new_error },
    { OSSL_FUNC_CORE_SET_ERROR_DEBUG, (void (*)(void))core_set_error_debug },
    { OSSL_FUNC_CORE_VSET_ERROR, (void (*)(void))core_vset_error },
    { OSSL_FUNC_CORE_SET_ERROR_MARK, (void (*)(void))core_set_error_mark },
    { OSSL_FUNC_CORE_CLEAR_LAST_ERROR_MARK,
      (void (*)(void))core_clear_last_error_mark },
    { OSSL_FUNC_CORE_POP_ERROR_TO_MARK, (void (*)(void))core_pop_error_to_mark },
    { OSSL_FUNC_BIO_NEW_FILE, (void (*)(void))ossl_core_bio_new_file },
    { OSSL_FUNC_BIO_NEW_MEMBUF, (void (*)(void))ossl_core_bio_new_mem_buf },
    { OSSL_FUNC_BIO_READ_EX, (void (*)(void))ossl_core_bio_read_ex },
    { OSSL_FUNC_BIO_WRITE_EX, (void (*)(void))ossl_core_bio_write_ex },
    { OSSL_FUNC_BIO_GETS, (void (*)(void))ossl_core_bio_gets },
    { OSSL_FUNC_BIO_PUTS, (void (*)(void))ossl_core_bio_puts },
    { OSSL_FUNC_BIO_CTRL, (void (*)(void))ossl_core_bio_ctrl },
    { OSSL_FUNC_BIO_UP_REF, (void (*)(void))ossl_core_bio_up_ref },
    { OSSL_FUNC_BIO_FREE, (void (*)(void))ossl_core_bio_free },
    { OSSL_FUNC_BIO_VPRINTF, (void (*)(void))ossl_core_bio_vprintf },
    { OSSL_FUNC_BIO_VSNPRINTF, (void (*)(void))BIO_vsnprintf },
    { OSSL_FUNC_SELF_TEST_CB, (void (*)(void))OSSL_SELF_TEST_get_callback },
    { OSSL_FUNC_GET_ENTROPY, (void (*)(void))ossl_rand_get_entropy },
    { OSSL_FUNC_CLEANUP_ENTROPY, (void (*)(void))ossl_rand_cleanup_entropy },
    { OSSL_FUNC_GET_NONCE, (void (*)(void))ossl_rand_get_nonce },
    { OSSL_FUNC_CLEANUP_NONCE, (void (*)(void))ossl_rand_cleanup_nonce },
#endif
    { OSSL_FUNC_CRYPTO_MALLOC, (void (*)(void))CRYPTO_malloc },
    { OSSL_FUNC_CRYPTO_ZALLOC, (void (*)(void))CRYPTO_zalloc },
    { OSSL_FUNC_CRYPTO_FREE, (void (*)(void))CRYPTO_free },
    { OSSL_FUNC_CRYPTO_CLEAR_FREE, (void (*)(void))CRYPTO_clear_free },
    { OSSL_FUNC_CRYPTO_REALLOC, (void (*)(void))CRYPTO_realloc },
    { OSSL_FUNC_CRYPTO_CLEAR_REALLOC, (void (*)(void))CRYPTO_clear_realloc },
    { OSSL_FUNC_CRYPTO_SECURE_MALLOC, (void (*)(void))CRYPTO_secure_malloc },
    { OSSL_FUNC_CRYPTO_SECURE_ZALLOC, (void (*)(void))CRYPTO_secure_zalloc },
    { OSSL_FUNC_CRYPTO_SECURE_FREE, (void (*)(void))CRYPTO_secure_free },
    { OSSL_FUNC_CRYPTO_SECURE_CLEAR_FREE,
        (void (*)(void))CRYPTO_secure_clear_free },
    { OSSL_FUNC_CRYPTO_SECURE_ALLOCATED,
        (void (*)(void))CRYPTO_secure_allocated },
    { OSSL_FUNC_OPENSSL_CLEANSE, (void (*)(void))OPENSSL_cleanse },
#ifndef FIPS_MODULE
    { OSSL_FUNC_PROVIDER_REGISTER_CHILD_CB,
        (void (*)(void))ossl_provider_register_child_cb },
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
};
static const OSSL_DISPATCH *core_dispatch = core_dispatch_;
static const OSSL_DISPATCH core_dispatch_[] = {
    { OSSL_FUNC_CORE_GETTABLE_PARAMS, (void (*)(void))core_gettable_params },
    { OSSL_FUNC_CORE_GET_PARAMS, (void (*)(void))core_get_params },
    { OSSL_FUNC_CORE_GET_LIBCTX, (void (*)(void))core_get_libctx },
    { OSSL_FUNC_CORE_THREAD_START, (void (*)(void))core_thread_start },
#ifndef FIPS_MODULE
    { OSSL_FUNC_CORE_NEW_ERROR, (void (*)(void))core_new_error },
    { OSSL_FUNC_CORE_SET_ERROR_DEBUG, (void (*)(void))core_set_error_debug },
    { OSSL_FUNC_CORE_VSET_ERROR, (void (*)(void))core_vset_error },
    { OSSL_FUNC_CORE_SET_ERROR_MARK, (void (*)(void))core_set_error_mark },
    { OSSL_FUNC_CORE_CLEAR_LAST_ERROR_MARK,
      (void (*)(void))core_clear_last_error_mark },
    { OSSL_FUNC_CORE_POP_ERROR_TO_MARK, (void (*)(void))core_pop_error_to_mark },
    { OSSL_FUNC_BIO_NEW_FILE, (void (*)(void))ossl_core_bio_new_file },
    { OSSL_FUNC_BIO_NEW_MEMBUF, (void (*)(void))ossl_core_bio_new_mem_buf },
    { OSSL_FUNC_BIO_READ_EX, (void (*)(void))ossl_core_bio_read_ex },
    { OSSL_FUNC_BIO_WRITE_EX, (void (*)(void))ossl_core_bio_write_ex },
    { OSSL_FUNC_BIO_GETS, (void (*)(void))ossl_core_bio_gets },
    { OSSL_FUNC_BIO_PUTS, (void (*)(void))ossl_core_bio_puts },
    { OSSL_FUNC_BIO_CTRL, (void (*)(void))ossl_core_bio_ctrl },
    { OSSL_FUNC_BIO_UP_REF, (void (*)(void))ossl_core_bio_up_ref },
    { OSSL_FUNC_BIO_FREE, (void (*)(void))ossl_core_bio_free },
    { OSSL_FUNC_BIO_VPRINTF, (void (*)(void))ossl_core_bio_vprintf },
    { OSSL_FUNC_BIO_VSNPRINTF, (void (*)(void))BIO_vsnprintf },
    { OSSL_FUNC_SELF_TEST_CB, (void (*)(void))OSSL_SELF_TEST_get_callback },
    { OSSL_FUNC_GET_ENTROPY, (void (*)(void))ossl_rand_get_entropy },
    { OSSL_FUNC_CLEANUP_ENTROPY, (void (*)(void))ossl_rand_cleanup_entropy },
    { OSSL_FUNC_GET_NONCE, (void (*)(void))ossl_rand_get_nonce },
    { OSSL_FUNC_CLEANUP_NONCE, (void (*)(void))ossl_rand_cleanup_nonce },
#endif
    { OSSL_FUNC_CRYPTO_MALLOC, (void (*)(void))CRYPTO_malloc },
    { OSSL_FUNC_CRYPTO_ZALLOC, (void (*)(void))CRYPTO_zalloc },
    { OSSL_FUNC_CRYPTO_FREE, (void (*)(void))CRYPTO_free },
    { OSSL_FUNC_CRYPTO_CLEAR_FREE, (void (*)(void))CRYPTO_clear_free },
    { OSSL_FUNC_CRYPTO_REALLOC, (void (*)(void))CRYPTO_realloc },
    { OSSL_FUNC_CRYPTO_CLEAR_REALLOC, (void (*)(void))CRYPTO_clear_realloc },
    { OSSL_FUNC_CRYPTO_SECURE_MALLOC, (void (*)(void))CRYPTO_secure_malloc },
    { OSSL_FUNC_CRYPTO_SECURE_ZALLOC, (void (*)(void))CRYPTO_secure_zalloc },
    { OSSL_FUNC_CRYPTO_SECURE_FREE, (void (*)(void))CRYPTO_secure_free },
    { OSSL_FUNC_CRYPTO_SECURE_CLEAR_FREE,
        (void (*)(void))CRYPTO_secure_clear_free },
    { OSSL_FUNC_CRYPTO_SECURE_ALLOCATED,
        (void (*)(void))CRYPTO_secure_allocated },
    { OSSL_FUNC_OPENSSL_CLEANSE, (void (*)(void))OPENSSL_cleanse },
#ifndef FIPS_MODULE
    { OSSL_FUNC_PROVIDER_REGISTER_CHILD_CB,
        (void (*)(void))ossl_provider_register_child_cb },
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
};
static const OSSL_DISPATCH *core_dispatch = core_dispatch_;