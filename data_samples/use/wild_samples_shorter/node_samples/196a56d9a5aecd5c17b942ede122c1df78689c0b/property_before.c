#include "internal/core.h"
#include "internal/property.h"
#include "internal/provider.h"
#include "crypto/ctype.h"
#include <openssl/lhash.h>
#include <openssl/rand.h>
#include "internal/thread_once.h"
struct ossl_method_store_st {
    OSSL_LIB_CTX *ctx;
    SPARSE_ARRAY_OF(ALGORITHM) *algs;
    CRYPTO_RWLOCK *lock;

    /* query cache specific values */

    /* Count of the query cache entries for all algs */
    LHASH_OF(QUERY) *cache;
    size_t nelem;
    uint32_t seed;
} IMPL_CACHE_FLUSH;

DEFINE_SPARSE_ARRAY_OF(ALGORITHM);

    res = OPENSSL_zalloc(sizeof(*res));
    if (res != NULL) {
        res->ctx = ctx;
        if ((res->algs = ossl_sa_ALGORITHM_new()) == NULL) {
            OPENSSL_free(res);
            return NULL;
        }
        if ((res->lock = CRYPTO_THREAD_lock_new()) == NULL) {
            ossl_sa_ALGORITHM_free(res->algs);
            OPENSSL_free(res);
            return NULL;
        }
    }
    return res;
void ossl_method_store_free(OSSL_METHOD_STORE *store)
{
    if (store != NULL) {
        ossl_sa_ALGORITHM_doall_arg(store->algs, &alg_cleanup, store);
        ossl_sa_ALGORITHM_free(store->algs);
        CRYPTO_THREAD_lock_free(store->lock);
        OPENSSL_free(store);
    }
}

static ALGORITHM *ossl_method_store_retrieve(OSSL_METHOD_STORE *store, int nid)
{
    return ossl_sa_ALGORITHM_get(store->algs, nid);
}

static int ossl_method_store_insert(OSSL_METHOD_STORE *store, ALGORITHM *alg)
{
        return ossl_sa_ALGORITHM_set(store->algs, alg->nid, alg);
}

int ossl_method_store_add(OSSL_METHOD_STORE *store, const OSSL_PROVIDER *prov,
                          int nid, const char *properties, void *method,
static void ossl_method_cache_flush_some(OSSL_METHOD_STORE *store)
{
    IMPL_CACHE_FLUSH state;

    state.nelem = 0;
    if ((state.seed = OPENSSL_rdtsc()) == 0)
        state.seed = 1;
    store->cache_need_flush = 0;
    ossl_sa_ALGORITHM_doall_arg(store->algs, &impl_cache_flush_one_alg, &state);
    store->cache_nelem = state.nelem;
}

int ossl_method_store_cache_get(OSSL_METHOD_STORE *store, OSSL_PROVIDER *prov,
                                int nid, const char *prop_query, void **method)