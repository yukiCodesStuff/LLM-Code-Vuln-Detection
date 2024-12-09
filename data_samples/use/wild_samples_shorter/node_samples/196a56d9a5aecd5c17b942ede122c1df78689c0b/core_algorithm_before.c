    int operation_id;            /* May be zero for finding them all */
    int (*pre)(OSSL_PROVIDER *, int operation_id, int no_store, void *data,
               int *result);
    void (*fn)(OSSL_PROVIDER *, const OSSL_ALGORITHM *, int no_store,
               void *data);
    int (*post)(OSSL_PROVIDER *, int operation_id, int no_store, void *data,
                int *result);
    void *data;
};
    struct algorithm_data_st *data = cbdata;
    int ret = 0;

    /* Do we fulfill pre-conditions? */
    if (data->pre == NULL) {
        /* If there is no pre-condition function, assume "yes" */
        ret = 1;
    } else if (!data->pre(provider, cur_operation, no_store, data->data,
                          &ret)) {
        /* Error, bail out! */
        return -1;
    }

    /*
     * If pre-condition not fulfilled don't add this set of implementations,
     * but do continue with the next.  This simply means that another thread
     * got to it first.
     */
    if (ret == 0)
        return 1;

    if (map != NULL) {
        const OSSL_ALGORITHM *thismap;

    } else if (!data->post(provider, cur_operation, no_store, data->data,
                           &ret)) {
        /* Error, bail out! */
        return -1;
    }

    return ret;
}

/*
         cur_operation++) {
        int no_store = 0;        /* Assume caching is ok */
        const OSSL_ALGORITHM *map = NULL;
        int ret;

        map = ossl_provider_query_operation(provider, cur_operation,
                                            &no_store);
        ret = algorithm_do_map(provider, map, cur_operation, no_store, data);
                           OSSL_PROVIDER *provider,
                           int (*pre)(OSSL_PROVIDER *, int operation_id,
                                      int no_store, void *data, int *result),
                           void (*fn)(OSSL_PROVIDER *provider,
                                      const OSSL_ALGORITHM *algo,
                                      int no_store, void *data),
                           int (*post)(OSSL_PROVIDER *, int operation_id,
                                       int no_store, void *data, int *result),
                           void *data)
{
    cbdata.libctx = libctx;
    cbdata.operation_id = operation_id;
    cbdata.pre = pre;
    cbdata.fn = fn;
    cbdata.post = post;
    cbdata.data = data;

    if (provider == NULL) {