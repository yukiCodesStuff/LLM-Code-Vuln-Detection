typedef struct ossl_method_construct_method_st {
    /* Get a temporary store */
    void *(*get_tmp_store)(void *data);
    /* Get an already existing method from a store */
    void *(*get)(void *store, const OSSL_PROVIDER **prov, void *data);
    /* Store a method in a store */
    int (*put)(void *store, void *method, const OSSL_PROVIDER *prov,
               const char *name, const char *propdef, void *data);
    /* Construct a new method */
    void *(*construct)(const OSSL_ALGORITHM *algodef, OSSL_PROVIDER *prov,
                       void *data);
    /* Destruct a method */
    void (*destruct)(void *method, void *data);
} OSSL_METHOD_CONSTRUCT_METHOD;

void *ossl_method_construct(OSSL_LIB_CTX *ctx, int operation_id,
                            OSSL_PROVIDER **provider_rw, int force_cache,
                            OSSL_METHOD_CONSTRUCT_METHOD *mcm, void *mcm_data);

void ossl_algorithm_do_all(OSSL_LIB_CTX *libctx, int operation_id,
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

__owur int ossl_lib_ctx_write_lock(OSSL_LIB_CTX *ctx);
__owur int ossl_lib_ctx_read_lock(OSSL_LIB_CTX *ctx);
int ossl_lib_ctx_unlock(OSSL_LIB_CTX *ctx);
int ossl_lib_ctx_is_child(OSSL_LIB_CTX *ctx);
#endif
typedef struct ossl_method_construct_method_st {
    /* Get a temporary store */
    void *(*get_tmp_store)(void *data);
    /* Get an already existing method from a store */
    void *(*get)(void *store, const OSSL_PROVIDER **prov, void *data);
    /* Store a method in a store */
    int (*put)(void *store, void *method, const OSSL_PROVIDER *prov,
               const char *name, const char *propdef, void *data);
    /* Construct a new method */
    void *(*construct)(const OSSL_ALGORITHM *algodef, OSSL_PROVIDER *prov,
                       void *data);
    /* Destruct a method */
    void (*destruct)(void *method, void *data);
} OSSL_METHOD_CONSTRUCT_METHOD;

void *ossl_method_construct(OSSL_LIB_CTX *ctx, int operation_id,
                            OSSL_PROVIDER **provider_rw, int force_cache,
                            OSSL_METHOD_CONSTRUCT_METHOD *mcm, void *mcm_data);

void ossl_algorithm_do_all(OSSL_LIB_CTX *libctx, int operation_id,
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

__owur int ossl_lib_ctx_write_lock(OSSL_LIB_CTX *ctx);
__owur int ossl_lib_ctx_read_lock(OSSL_LIB_CTX *ctx);
int ossl_lib_ctx_unlock(OSSL_LIB_CTX *ctx);
int ossl_lib_ctx_is_child(OSSL_LIB_CTX *ctx);
#endif