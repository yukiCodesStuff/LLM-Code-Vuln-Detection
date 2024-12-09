static IMPLEMENT_LHASH_COMP_FN(index_name, OPENSSL_CSTRING)
#undef BSIZE
#define BSIZE 256
BIGNUM *load_serial(const char *serialfile, int *exists, int create,
                    ASN1_INTEGER **retai)
{
    BIO *in = NULL;
    BIGNUM *ret = NULL;
    char buf[1024];
        goto err;

    in = BIO_new_file(serialfile, "r");
    if (exists != NULL)
        *exists = in != NULL;
    if (in == NULL) {
        if (!create) {
            perror(serialfile);
            goto err;
        }
        ERR_clear_error();
        ret = BN_new();
        if (ret == NULL) {
            BIO_printf(bio_err, "Out of memory\n");
        } else if (!rand_serial(ret, ai)) {
            BIO_printf(bio_err, "Error creating random number to store in %s\n",
                       serialfile);
            BN_free(ret);
            ret = NULL;
        }
    } else {
        if (!a2i_ASN1_INTEGER(in, ai, buf, 1024)) {
            BIO_printf(bio_err, "Unable to load number from %s\n",
                       serialfile);
        }
    }

    if (ret != NULL && retai != NULL) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (ret == NULL)
        ERR_print_errors(bio_err);
    BIO_free(in);
    ASN1_INTEGER_free(ai);
    return ret;
}
    APP_HTTP_TLS_INFO *info = (APP_HTTP_TLS_INFO *)arg;
    SSL_CTX *ssl_ctx = info->ssl_ctx;

    if (ssl_ctx == NULL) /* not using TLS */
        return bio;
    if (connect) {
        SSL *ssl;
        BIO *sbio = NULL;

        /* adapt after fixing callback design flaw, see #17088 */
                       "missing SSL_CTX");
        goto end;
    }
    if (!use_ssl && ssl_ctx != NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_INVALID_ARGUMENT,
                       "SSL_CTX given but use_ssl == 0");
        goto end;
    }

    info.server = server;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
    void *prefix = NULL;

    if (b == NULL)
        return NULL;

#ifdef OPENSSL_SYS_VMS
    if (FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif
    BIO *b = BIO_new_fp(stderr,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
#ifdef OPENSSL_SYS_VMS
    if (b != NULL && FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif
    return b;
}