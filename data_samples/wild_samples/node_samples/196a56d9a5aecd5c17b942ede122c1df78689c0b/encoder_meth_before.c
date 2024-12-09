{
    return ossl_lib_ctx_get_data(libctx, OSSL_LIB_CTX_ENCODER_STORE_INDEX,
                                 &encoder_store_method);
}

/* Get encoder methods from a store, or put one in */
static void *get_encoder_from_store(void *store, const OSSL_PROVIDER **prov,
                                    void *data)
{
    struct encoder_data_st *methdata = data;
    void *method = NULL;
    int id;

    /*
     * get_encoder_from_store() is only called to try and get the method
     * that OSSL_ENCODER_fetch() is asking for, and the name or name id are
     * passed via methdata.
     */
    if ((id = methdata->id) == 0 && methdata->names != NULL) {
        OSSL_NAMEMAP *namemap = ossl_namemap_stored(methdata->libctx);
        const char *names = methdata->names;
        const char *q = strchr(names, NAME_SEPARATOR);
        size_t l = (q == NULL ? strlen(names) : (size_t)(q - names));

        if (namemap == 0)
            return NULL;
        id = ossl_namemap_name2num_n(namemap, methdata->names, l);
    }
        OSSL_METHOD_CONSTRUCT_METHOD mcm = {
            get_tmp_encoder_store,
            get_encoder_from_store,
            put_encoder_in_store,
            construct_encoder,
            destruct_encoder
        };
        OSSL_PROVIDER *prov = NULL;

        methdata->id = id;
        methdata->names = name;
        methdata->propquery = propq;
        methdata->flag_construct_error_occurred = 0;
        if ((method = ossl_method_construct(methdata->libctx, OSSL_OP_ENCODER,
                                            &prov, 0 /* !force_cache */,
                                            &mcm, methdata)) != NULL) {
            /*
             * If construction did create a method for us, we know that
             * there is a correct name_id and meth_id, since those have
             * already been calculated in get_encoder_from_store() and
             * put_encoder_in_store() above.
             */
            if (id == 0)
                id = ossl_namemap_name2num(namemap, name);
            ossl_method_store_cache_set(store, prov, id, propq, method,
                                        up_ref_encoder, free_encoder);
        }

        /*
         * If we never were in the constructor, the algorithm to be fetched
         * is unsupported.
         */
        unsupported = !methdata->flag_construct_error_occurred;
    }