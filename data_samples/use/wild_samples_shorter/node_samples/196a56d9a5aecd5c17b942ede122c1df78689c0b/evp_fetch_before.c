                                 &evp_method_store_method);
}

/*
 * To identify the method in the EVP method store, we mix the name identity
 * with the operation identity, under the assumption that we don't have more
 * than 2^23 names or more than 2^8 operation types.
        || !ossl_method_store_cache_get(store, prov, meth_id, propq, &method)) {
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_evp_method_store,
            get_evp_method_from_store,
            put_evp_method_in_store,
            construct_evp_method,
            destruct_evp_method