                                 &decoder_store_method);
}

static int reserve_decoder_store(void *store, void *data)
{
    struct decoder_data_st *methdata = data;

    if (store == NULL
        && (store = get_decoder_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_lock_store(store);
}

static int unreserve_decoder_store(void *store, void *data)
{
    struct decoder_data_st *methdata = data;

    if (store == NULL
        && (store = get_decoder_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_unlock_store(store);
}

/* Get decoder methods from a store, or put one in */
static void *get_decoder_from_store(void *store, const OSSL_PROVIDER **prov,
                                    void *data)
{
        || !ossl_method_store_cache_get(store, NULL, id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_decoder_store,
            reserve_decoder_store,
            unreserve_decoder_store,
            get_decoder_from_store,
            put_decoder_in_store,
            construct_decoder,
            destruct_decoder