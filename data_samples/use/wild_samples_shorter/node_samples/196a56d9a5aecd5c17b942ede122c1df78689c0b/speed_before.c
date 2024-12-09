#  define HAVE_FORK 0
# else
#  define HAVE_FORK 1
# endif
#endif

#if HAVE_FORK
    loopargs_t *tempargs = *(loopargs_t **) args;
    EVP_PKEY_CTX *ffdh_ctx = tempargs->ffdh_ctx[testnum];
    unsigned char *derived_secret = tempargs->secret_ff_a;
    size_t outlen = MAX_FFDH_SIZE;
    int count;

    for (count = 0; COND(ffdh_c[testnum][0]); count++)
        EVP_PKEY_derive(ffdh_ctx, derived_secret, &outlen);
    return count;
}
#endif /* OPENSSL_NO_DH */

                goto end;

            if (!EVP_MAC_CTX_set_params(loopargs[i].mctx, params))
                goto end;
        }
        for (testnum = 0; testnum < size_num; testnum++) {
            print_message(names[D_HMAC], c[D_HMAC][testnum], lengths[testnum],
                          seconds.sym);
        EVP_MAC_free(mac);
        mac = NULL;
    }

    if (doit[D_CBC_DES]) {
        int st = 1;

        for (i = 0; st && i < loopargs_len; i++) {
    int n;
    int fd[2];
    int *fds;
    static char sep[] = ":";

    fds = app_malloc(sizeof(*fds) * multi, "fd buffer for do_multi");
    for (n = 0; n < multi; ++n) {
        fclose(f);
    }
    OPENSSL_free(fds);
    return 1;
}
#endif
