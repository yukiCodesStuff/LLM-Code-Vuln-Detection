{
    return ossl_lib_ctx_get_data(libctx, OSSL_LIB_CTX_EVP_METHOD_STORE_INDEX,
                                 &evp_method_store_method);
}

static int reserve_evp_method_store(void *store, void *data)
{
    struct evp_method_data_st *methdata = data;

    if (store == NULL
        && (store = get_evp_method_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_lock_store(store);
}

static int unreserve_evp_method_store(void *store, void *data)
{
    struct evp_method_data_st *methdata = data;

    if (store == NULL
        && (store = get_evp_method_store(methdata->libctx)) == NULL)
        return 0;

    return ossl_method_unlock_store(store);
}

/*
 * To identify the method in the EVP method store, we mix the name identity
 * with the operation identity, under the assumption that we don't have more
 * than 2^23 names or more than 2^8 operation types.
 *
 * The resulting identity is a 31-bit integer, composed like this:
 *
 * +---------23 bits--------+-8 bits-+
 * |      name identity     | op id  |
 * +------------------------+--------+
 *
 * We limit this composite number to 31 bits, thus leaving the top uint32_t
 * bit always zero, to avoid negative sign extension when downshifting after
 * this number happens to be passed to an int (which happens as soon as it's
 * passed to ossl_method_store_cache_set(), and it's in that form that it
 * gets passed along to filter_on_operation_id(), defined further down.
 */
#define METHOD_ID_OPERATION_MASK        0x000000FF
#define METHOD_ID_OPERATION_MAX         ((1 << 8) - 1)
#define METHOD_ID_NAME_MASK             0x7FFFFF00
#define METHOD_ID_NAME_OFFSET           8
#define METHOD_ID_NAME_MAX              ((1 << 23) - 1)
static uint32_t evp_method_id(int name_id, unsigned int operation_id)
{
    if (!ossl_assert(name_id > 0 && name_id <= METHOD_ID_NAME_MAX)
        || !ossl_assert(operation_id > 0
                        && operation_id <= METHOD_ID_OPERATION_MAX))
        return 0;
    return (((name_id << METHOD_ID_NAME_OFFSET) & METHOD_ID_NAME_MASK)
            | (operation_id & METHOD_ID_OPERATION_MASK));
}

static void *get_evp_method_from_store(void *store, const OSSL_PROVIDER **prov,
                                       void *data)
{
    struct evp_method_data_st *methdata = data;
    void *method = NULL;
    int name_id = 0;
    uint32_t meth_id;

    /*
     * get_evp_method_from_store() is only called to try and get the method
     * that evp_generic_fetch() is asking for, and the operation id as well
     * as the name or name id are passed via methdata.
     */
    if ((name_id = methdata->name_id) == 0 && methdata->names != NULL) {
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(methdata->libctx);
        const char *names = methdata->names;
        const char *q = strchr(names, NAME_SEPARATOR);
        size_t l = (q == NULL ? strlen(names) : (size_t)(q - names));

        if (namemap == 0)
            return NULL;
        name_id = ossl_namemap_name2num_n(namemap, names, l);
    }
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_evp_method_store,
            reserve_evp_method_store,
            unreserve_evp_method_store,
            get_evp_method_from_store,
            put_evp_method_in_store,
            construct_evp_method,
            destruct_evp_method
        };

        methdata->operation_id = operation_id;
        methdata->name_id = name_id;
        methdata->names = name;
        methdata->propquery = propq;
        methdata->method_from_algorithm = new_method;
        methdata->refcnt_up_method = up_ref_method;
        methdata->destruct_method = free_method;
        methdata->flag_construct_error_occurred = 0;
        if ((method = ossl_method_construct(methdata->libctx, operation_id,
                                            &prov, 0 /* !force_cache */,
                                            &mcm, methdata)) != NULL) {
            /*
             * If construction did create a method for us, we know that
             * there is a correct name_id and meth_id, since those have
             * already been calculated in get_evp_method_from_store() and
             * put_evp_method_in_store() above.
             */
            if (name_id == 0)
                name_id = ossl_namemap_name2num(namemap, name);
            meth_id = evp_method_id(name_id, operation_id);
            if (name_id != 0)
                ossl_method_store_cache_set(store, prov, meth_id, propq,
                                            method, up_ref_method, free_method);
        }

        /*
         * If we never were in the constructor, the algorithm to be fetched
         * is unsupported.
         */
        unsupported = !methdata->flag_construct_error_occurred;
    }