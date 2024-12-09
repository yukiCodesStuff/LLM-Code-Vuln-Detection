                                 &decoder_store_method);
}

/* Get decoder methods from a store, or put one in */
static void *get_decoder_from_store(void *store, const OSSL_PROVIDER **prov,
                                    void *data)
{
        || !ossl_method_store_cache_get(store, NULL, id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_decoder_store,
            get_decoder_from_store,
            put_decoder_in_store,
            construct_decoder,
            destruct_decoder