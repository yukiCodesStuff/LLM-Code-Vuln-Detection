                                 &encoder_store_method);
}

static int reserve_encoder_store(void *store, void *data)
{
    struct encoder_data_st *methdata = data;

    if (store == NULL
        && (store = get_encoder_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_lock_store(store);
}

static int unreserve_encoder_store(void *store, void *data)
{
    struct encoder_data_st *methdata = data;

    if (store == NULL
        && (store = get_encoder_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_unlock_store(store);
}

/* Get encoder methods from a store, or put one in */
static void *get_encoder_from_store(void *store, const OSSL_PROVIDER **prov,
                                    void *data)
{
        || !ossl_method_store_cache_get(store, NULL, id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_encoder_store,
            reserve_encoder_store,
            unreserve_encoder_store,
            get_encoder_from_store,
            put_encoder_in_store,
            construct_encoder,
            destruct_encoder