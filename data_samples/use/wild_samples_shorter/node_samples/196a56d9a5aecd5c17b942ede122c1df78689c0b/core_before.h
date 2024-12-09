typedef struct ossl_method_construct_method_st {
    /* Get a temporary store */
    void *(*get_tmp_store)(void *data);
    /* Get an already existing method from a store */
    void *(*get)(void *store, const OSSL_PROVIDER **prov, void *data);
    /* Store a method in a store */
    int (*put)(void *store, void *method, const OSSL_PROVIDER *prov,
                           OSSL_PROVIDER *provider,
                           int (*pre)(OSSL_PROVIDER *, int operation_id,
                                      int no_store, void *data, int *result),
                           void (*fn)(OSSL_PROVIDER *provider,
                                      const OSSL_ALGORITHM *algo,
                                      int no_store, void *data),
                           int (*post)(OSSL_PROVIDER *, int operation_id,
                                       int no_store, void *data, int *result),
                           void *data);
char *ossl_algorithm_get1_first_name(const OSSL_ALGORITHM *algo);