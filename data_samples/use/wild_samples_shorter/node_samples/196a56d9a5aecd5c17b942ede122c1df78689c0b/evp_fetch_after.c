                                 &evp_method_store_method);
}

static int reserve_evp_method_store(void *store, void *data)
{
    struct evp_method_data_st *methdata = data;

    if (store == NULL
        && (store = get_evp_method_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_lock_store(store);
}

static int unreserve_evp_method_store(void *store, void *data)
{
    struct evp_method_data_st *methdata = data;

    if (store == NULL
        && (store = get_evp_method_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_unlock_store(store);
}

/*
 * To identify the method in the EVP method store, we mix the name identity
 * with the operation identity, under the assumption that we don't have more
 * than 2^23 names or more than 2^8 operation types.
        || !ossl_method_store_cache_get(store, prov, meth_id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_evp_method_store,
            reserve_evp_method_store,
            unreserve_evp_method_store,
            get_evp_method_from_store,
            put_evp_method_in_store,
            construct_evp_method,
            destruct_evp_method