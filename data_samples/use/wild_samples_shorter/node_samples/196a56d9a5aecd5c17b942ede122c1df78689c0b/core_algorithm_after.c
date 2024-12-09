    int operation_id;            /* May be zero for finding them all */
    int (*pre)(OSSL_PROVIDER *, int operation_id, int no_store, void *data,
               int *result);
    int (*reserve_store)(int no_store, void *data);
    void (*fn)(OSSL_PROVIDER *, const OSSL_ALGORITHM *, int no_store,
               void *data);
    int (*unreserve_store)(void *data);
    int (*post)(OSSL_PROVIDER *, int operation_id, int no_store, void *data,
                int *result);
    void *data;
};
    struct algorithm_data_st *data = cbdata;
    int ret = 0;

    if (!data->reserve_store(no_store, data->data))
        /* Error, bail out! */
        return -1;

    /* Do we fulfill pre-conditions? */
    if (data->pre == NULL) {
        /* If there is no pre-condition function, assume "yes" */
        ret = 1;
    } else if (!data->pre(provider, cur_operation, no_store, data->data,
                          &ret)) {
        /* Error, bail out! */
        ret = -1;
        goto end;
    }

    /*
     * If pre-condition not fulfilled don't add this set of implementations,
     * but do continue with the next.  This simply means that another thread
     * got to it first.
     */
    if (ret == 0) {
        ret = 1;
        goto end;
    }

    if (map != NULL) {
        const OSSL_ALGORITHM *thismap;

    } else if (!data->post(provider, cur_operation, no_store, data->data,
                           &ret)) {
        /* Error, bail out! */
        ret = -1;
    }

 end:
    data->unreserve_store(data->data);

    return ret;
}

/*
         cur_operation++) {
        int no_store = 0;        /* Assume caching is ok */
        const OSSL_ALGORITHM *map = NULL;
        int ret = 0;

        map = ossl_provider_query_operation(provider, cur_operation,
                                            &no_store);
        ret = algorithm_do_map(provider, map, cur_operation, no_store, data);
                           OSSL_PROVIDER *provider,
                           int (*pre)(OSSL_PROVIDER *, int operation_id,
                                      int no_store, void *data, int *result),
                           int (*reserve_store)(int no_store, void *data),
                           void (*fn)(OSSL_PROVIDER *provider,
                                      const OSSL_ALGORITHM *algo,
                                      int no_store, void *data),
                           int (*unreserve_store)(void *data),
                           int (*post)(OSSL_PROVIDER *, int operation_id,
                                       int no_store, void *data, int *result),
                           void *data)
{
    cbdata.libctx = libctx;
    cbdata.operation_id = operation_id;
    cbdata.pre = pre;
    cbdata.reserve_store = reserve_store;
    cbdata.fn = fn;
    cbdata.unreserve_store = unreserve_store;
    cbdata.post = post;
    cbdata.data = data;

    if (provider == NULL) {