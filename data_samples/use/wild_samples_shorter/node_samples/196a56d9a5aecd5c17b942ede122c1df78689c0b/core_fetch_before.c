    return no_store && !data->force_store;
}

static int ossl_method_construct_precondition(OSSL_PROVIDER *provider,
                                              int operation_id, int no_store,
                                              void *cbdata, int *result)
{
     * It is *expected* that the put function increments the refcnt
     * of the passed method.
     */

    if (!is_temporary_method_store(no_store, data)) {
        /* If we haven't been told not to store, add to the global store */
        data->mcm->put(NULL, method, provider, algo->algorithm_names,
                       algo->property_definition, data->mcm_data);
    } else {
        /*
         * If we have been told not to store the method "permanently", we
         * ask for a temporary store, and store the method there.
         * The owner of |data->mcm| is completely responsible for managing
         * that temporary store.
         */
        if ((data->store = data->mcm->get_tmp_store(data->mcm_data)) == NULL)
            return;

        data->mcm->put(data->store, method, provider, algo->algorithm_names,
                       algo->property_definition, data->mcm_data);
    }

    /* refcnt-- because we're dropping the reference */
    data->mcm->destruct(method, data->mcm_data);
}
    cbdata.mcm_data = mcm_data;
    ossl_algorithm_do_all(libctx, operation_id, provider,
                          ossl_method_construct_precondition,
                          ossl_method_construct_this,
                          ossl_method_construct_postcondition,
                          &cbdata);

    /* If there is a temporary store, try there first */