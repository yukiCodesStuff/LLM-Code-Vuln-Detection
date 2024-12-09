{
    OPENSSL_INIT_STOP *newhand;

#if !defined(OPENSSL_USE_NODELETE)\
    && !defined(OPENSSL_NO_PINSHARED)
    {
# if defined(DSO_WIN32) && !defined(_WIN32_WCE)
        HMODULE handle = NULL;
        BOOL ret;
        union {
            void *sym;
            void (*func)(void);
        } handlersym;

        handlersym.func = handler;

        /*
         * We don't use the DSO route for WIN32 because there is a better
         * way
         */
        ret = GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                                | GET_MODULE_HANDLE_EX_FLAG_PIN,
                                handlersym.sym, &handle);

        if (!ret)
            return 0;
# elif !defined(DSO_NONE)
        /*
         * Deliberately leak a reference to the handler. This will force the
         * library/code containing the handler to remain loaded until we run the
         * atexit handler. If -znodelete has been used then this is
         * unnecessary.
         */
        DSO *dso = NULL;
        union {
            void *sym;
            void (*func)(void);
        } handlersym;

        handlersym.func = handler;

        ERR_set_mark();
        dso = DSO_dsobyaddr(handlersym.sym, DSO_FLAG_NO_UNLOAD_ON_FREE);
        /* See same code above in ossl_init_base() for an explanation. */
        OSSL_TRACE1(INIT,
                   "atexit: obtained DSO reference? %s\n",
                   (dso == NULL ? "No!" : "Yes."));
        DSO_free(dso);
        ERR_pop_to_mark();
# endif
    }
#endif

    if ((newhand = OPENSSL_malloc(sizeof(*newhand))) == NULL) {
        ERR_raise(ERR_LIB_CRYPTO, ERR_R_MALLOC_FAILURE);
        return 0;
    }

    newhand->handler = handler;
    newhand->next = stop_handlers;
    stop_handlers = newhand;

    return 1;
}
