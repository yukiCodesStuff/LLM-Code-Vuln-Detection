#  define HAVE_FORK 0
# else
#  define HAVE_FORK 1
#  include <sys/wait.h>
# endif
#endif

#if HAVE_FORK
    loopargs_t *tempargs = *(loopargs_t **) args;
    EVP_PKEY_CTX *ffdh_ctx = tempargs->ffdh_ctx[testnum];
    unsigned char *derived_secret = tempargs->secret_ff_a;
    int count;

    for (count = 0; COND(ffdh_c[testnum][0]); count++) {
        /* outlen can be overwritten with a too small value (no padding used) */
        size_t outlen = MAX_FFDH_SIZE;

        EVP_PKEY_derive(ffdh_ctx, derived_secret, &outlen);
    }
    return count;
}
#endif /* OPENSSL_NO_DH */

                goto end;

            if (!EVP_MAC_CTX_set_params(loopargs[i].mctx, params))
                goto skip_hmac; /* Digest not found */
        }
        for (testnum = 0; testnum < size_num; testnum++) {
            print_message(names[D_HMAC], c[D_HMAC][testnum], lengths[testnum],
                          seconds.sym);
        EVP_MAC_free(mac);
        mac = NULL;
    }
skip_hmac:
    if (doit[D_CBC_DES]) {
        int st = 1;

        for (i = 0; st && i < loopargs_len; i++) {
    int n;
    int fd[2];
    int *fds;
    int status;
    static char sep[] = ":";

    fds = app_malloc(sizeof(*fds) * multi, "fd buffer for do_multi");
    for (n = 0; n < multi; ++n) {
        fclose(f);
    }
    OPENSSL_free(fds);
    for (n = 0; n < multi; ++n) {
        while (wait(&status) == -1)
            if (errno != EINTR) {
                BIO_printf(bio_err, "Waitng for child failed with 0x%x\n",
                           errno);
                return 1;
            }
        if (WIFEXITED(status) && WEXITSTATUS(status)) {
            BIO_printf(bio_err, "Child exited with %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            BIO_printf(bio_err, "Child terminated by signal %d\n",
                       WTERMSIG(status));
        }
    }
    return 1;
}
#endif
