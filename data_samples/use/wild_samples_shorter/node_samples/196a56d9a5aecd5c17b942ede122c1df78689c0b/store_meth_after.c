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
        || !ossl_method_store_cache_get(store, NULL, id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_loader_store,
            reserve_loader_store,
            unreserve_loader_store,
            get_loader_from_store,
            put_loader_in_store,
            construct_loader,
            destruct_loader