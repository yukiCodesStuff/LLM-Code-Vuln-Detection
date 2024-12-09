/* Implementation store functions */
OSSL_METHOD_STORE *ossl_method_store_new(OSSL_LIB_CTX *ctx);
void ossl_method_store_free(OSSL_METHOD_STORE *store);
int ossl_method_store_add(OSSL_METHOD_STORE *store, const OSSL_PROVIDER *prov,
                          int nid, const char *properties, void *method,
                          int (*method_up_ref)(void *),
                          void (*method_destruct)(void *));