                                 &encoder_store_method);
}

/* Get encoder methods from a store, or put one in */
static void *get_encoder_from_store(void *store, const OSSL_PROVIDER **prov,
                                    void *data)
{
        || !ossl_method_store_cache_get(store, NULL, id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_encoder_store,
            get_encoder_from_store,
            put_encoder_in_store,
            construct_encoder,
            destruct_encoder