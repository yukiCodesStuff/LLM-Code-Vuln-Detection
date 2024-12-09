{
    struct construct_data_st *data = cbdata;

    return no_store && !data->force_store;
}

static int ossl_method_construct_reserve_store(int no_store, void *cbdata)
{
    struct construct_data_st *data = cbdata;

    if (is_temporary_method_store(no_store, data) && data->store == NULL) {
        /*
         * If we have been told not to store the method "permanently", we
         * ask for a temporary store, and store the method there.
         * The owner of |data->mcm| is completely responsible for managing
         * that temporary store.
         */
        if ((data->store = data->mcm->get_tmp_store(data->mcm_data)) == NULL)
            return 0;
    }
{
    struct construct_data_st *data = cbdata;
    void *method = NULL;

    if ((method = data->mcm->construct(algo, provider, data->mcm_data))
        == NULL)
        return;

    /*
     * Note regarding putting the method in stores:
     *
     * we don't need to care if it actually got in or not here.
     * If it didn't get in, it will simply not be available when
     * ossl_method_construct() tries to get it from the store.
     *
     * It is *expected* that the put function increments the refcnt
     * of the passed method.
     */
    data->mcm->put(data->store, method, provider, algo->algorithm_names,
                   algo->property_definition, data->mcm_data);

    /* refcnt-- because we're dropping the reference */
    data->mcm->destruct(method, data->mcm_data);
}

void *ossl_method_construct(OSSL_LIB_CTX *libctx, int operation_id,
                            OSSL_PROVIDER **provider_rw, int force_store,
                            OSSL_METHOD_CONSTRUCT_METHOD *mcm, void *mcm_data)
{
    void *method = NULL;
    OSSL_PROVIDER *provider = provider_rw != NULL ? *provider_rw : NULL;
    struct construct_data_st cbdata;

    /*
     * We might be tempted to try to look into the method store without
     * constructing to see if we can find our method there already.
     * Unfortunately that does not work well if the query contains
     * optional properties as newly loaded providers can match them better.
     * We trust that ossl_method_construct_precondition() and
     * ossl_method_construct_postcondition() make sure that the
     * ossl_algorithm_do_all() does very little when methods from
     * a provider have already been constructed.
     */

    cbdata.store = NULL;
    cbdata.force_store = force_store;
    cbdata.mcm = mcm;
    cbdata.mcm_data = mcm_data;
    ossl_algorithm_do_all(libctx, operation_id, provider,
                          ossl_method_construct_precondition,
                          ossl_method_construct_reserve_store,
                          ossl_method_construct_this,
                          ossl_method_construct_unreserve_store,
                          ossl_method_construct_postcondition,
                          &cbdata);

    /* If there is a temporary store, try there first */
    if (cbdata.store != NULL)
        method = mcm->get(cbdata.store, (const OSSL_PROVIDER **)provider_rw,
                          mcm_data);

    /* If no method was found yet, try the global store */
    if (method == NULL)
        method = mcm->get(NULL, (const OSSL_PROVIDER **)provider_rw, mcm_data);

    return method;
}
{
    void *method = NULL;
    OSSL_PROVIDER *provider = provider_rw != NULL ? *provider_rw : NULL;
    struct construct_data_st cbdata;

    /*
     * We might be tempted to try to look into the method store without
     * constructing to see if we can find our method there already.
     * Unfortunately that does not work well if the query contains
     * optional properties as newly loaded providers can match them better.
     * We trust that ossl_method_construct_precondition() and
     * ossl_method_construct_postcondition() make sure that the
     * ossl_algorithm_do_all() does very little when methods from
     * a provider have already been constructed.
     */

    cbdata.store = NULL;
    cbdata.force_store = force_store;
    cbdata.mcm = mcm;
    cbdata.mcm_data = mcm_data;
    ossl_algorithm_do_all(libctx, operation_id, provider,
                          ossl_method_construct_precondition,
                          ossl_method_construct_reserve_store,
                          ossl_method_construct_this,
                          ossl_method_construct_unreserve_store,
                          ossl_method_construct_postcondition,
                          &cbdata);

    /* If there is a temporary store, try there first */
    if (cbdata.store != NULL)
        method = mcm->get(cbdata.store, (const OSSL_PROVIDER **)provider_rw,
                          mcm_data);

    /* If no method was found yet, try the global store */
    if (method == NULL)
        method = mcm->get(NULL, (const OSSL_PROVIDER **)provider_rw, mcm_data);

    return method;
}