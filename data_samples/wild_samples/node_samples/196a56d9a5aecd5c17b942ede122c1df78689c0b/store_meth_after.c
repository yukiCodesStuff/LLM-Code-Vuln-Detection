{
    return ossl_lib_ctx_get_data(libctx, OSSL_LIB_CTX_STORE_LOADER_STORE_INDEX,
                                &loader_store_method);
}

static int reserve_loader_store(void *store, void *data)
{
    struct loader_data_st *methdata = data;

    if (store == NULL
        && (store = get_loader_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_lock_store(store);
}

static int unreserve_loader_store(void *store, void *data)
{
    struct loader_data_st *methdata = data;

    if (store == NULL
        && (store = get_loader_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_unlock_store(store);
}

/* Get loader methods from a store, or put one in */
static void *get_loader_from_store(void *store, const OSSL_PROVIDER **prov,
                                   void *data)
{
    struct loader_data_st *methdata = data;
    void *method = NULL;
    int id;

    if ((id = methdata->scheme_id) == 0) {
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(methdata->libctx);

        id = ossl_namemap_name2num(namemap, methdata->scheme);
    }
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_loader_store,
            reserve_loader_store,
            unreserve_loader_store,
            get_loader_from_store,
            put_loader_in_store,
            construct_loader,
            destruct_loader
        };
        OSSL_PROVIDER *prov = NULL;

        methdata->scheme_id = id;
        methdata->scheme = scheme;
        methdata->propquery = propq;
        methdata->flag_construct_error_occurred = 0;
        if ((method = ossl_method_construct(methdata->libctx, OSSL_OP_STORE,
                                            &prov, 0 /* !force_cache */,
                                            &mcm, methdata)) != NULL) {
            /*
             * If construction did create a method for us, we know that there
             * is a correct scheme_id, since those have already been calculated
             * in get_loader_from_store() and put_loader_in_store() above.
             */
            if (id == 0)
                id = ossl_namemap_name2num(namemap, scheme);
            ossl_method_store_cache_set(store, prov, id, propq, method,
                                        up_ref_loader, free_loader);
        }

        /*
         * If we never were in the constructor, the algorithm to be fetched
         * is unsupported.
         */
        unsupported = !methdata->flag_construct_error_occurred;
    }