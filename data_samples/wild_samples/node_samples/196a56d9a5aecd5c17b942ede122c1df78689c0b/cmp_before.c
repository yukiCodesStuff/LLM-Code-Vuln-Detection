            } else if (opt_tls_cert == NULL) {
                CMP_err("missing -tls_cert option");
                goto err;
            }
        }

        if ((info = OPENSSL_zalloc(sizeof(*info))) == NULL)
            goto err;
        (void)OSSL_CMP_CTX_set_http_cb_arg(ctx, info);
        /* info will be freed along with CMP ctx */
        info->server = opt_server;
        info->port = server_port;
        /* workaround for callback design flaw, see #17088: */
        info->use_proxy = proxy_host != NULL;
        info->timeout = OSSL_CMP_CTX_get_option(ctx, OSSL_CMP_OPT_MSG_TIMEOUT);
        info->ssl_ctx = setup_ssl_ctx(ctx, host, engine);

        if (info->ssl_ctx == NULL)
            goto err;
        (void)OSSL_CMP_CTX_set_http_cb(ctx, app_http_tls_cb);
    }
#endif

    if (!setup_protection_ctx(ctx, engine))
        goto err;

    if (!setup_request_ctx(ctx, engine))
        goto err;

    if (!set_name(opt_recipient, OSSL_CMP_CTX_set1_recipient, ctx, "recipient")
            || !set_name(opt_expect_sender, OSSL_CMP_CTX_set1_expected_sender,
                         ctx, "expected sender"))
        goto err;

    if (opt_geninfo != NULL && !handle_opt_geninfo(ctx))
        goto err;

    /* not printing earlier, to minimize confusion in case setup fails before */
    if (opt_rspin != NULL)
        CMP_info("will not contact any server since -rspin is given");
    else
        CMP_info2("will contact %s%s", server_buf, proxy_buf);

    ret = 1;

 err:
    OPENSSL_free(host);
    OPENSSL_free(port);
    OPENSSL_free(path);
    return ret;
 oom:
    CMP_err("out of memory");
    goto err;
}

/*
 * write out the given certificate to the output specified by bio.
 * Depending on options use either PEM or DER format.
 * Returns 1 on success, 0 on error
 */
static int write_cert(BIO *bio, X509 *cert)
{
    if ((opt_certform == FORMAT_PEM && PEM_write_bio_X509(bio, cert))
            || (opt_certform == FORMAT_ASN1 && i2d_X509_bio(bio, cert)))
        return 1;
    if (opt_certform != FORMAT_PEM && opt_certform != FORMAT_ASN1)
        BIO_printf(bio_err,
                   "error: unsupported type '%s' for writing certificates\n",
                   opt_certform_s);
    return 0;
}

/*
 * If destFile != NULL writes out a stack of certs to the given file.
 * In any case frees the certs.
 * Depending on options use either PEM or DER format,
 * where DER does not make much sense for writing more than one cert!
 * Returns number of written certificates on success, -1 on error.
 */
static int save_free_certs(OSSL_CMP_CTX *ctx,
                           STACK_OF(X509) *certs, char *destFile, char *desc)
{
    BIO *bio = NULL;
    int i;
    int n = sk_X509_num(certs);

    if (destFile == NULL)
        goto end;
    CMP_info3("received %d %s certificate(s), saving to file '%s'",
              n, desc, destFile);
    if (n > 1 && opt_certform != FORMAT_PEM)
        CMP_warn("saving more than one certificate in non-PEM format");

    if (destFile == NULL || (bio = BIO_new(BIO_s_file())) == NULL
            || !BIO_write_filename(bio, (char *)destFile)) {
        CMP_err1("could not open file '%s' for writing", destFile);
        n = -1;
        goto end;
    }

    for (i = 0; i < n; i++) {
        if (!write_cert(bio, sk_X509_value(certs, i))) {
            CMP_err1("cannot write certificate to file '%s'", destFile);
            n = -1;
            goto end;
        }
            if (!X509_add_cert(certs, newcert, X509_ADD_FLAG_UP_REF)) {
                sk_X509_free(certs);
                goto err;
            }
            if (save_free_certs(cmp_ctx, certs, opt_certout, "enrolled") < 0)
                goto err;
        }
        if (save_free_certs(cmp_ctx, OSSL_CMP_CTX_get1_newChain(cmp_ctx),
                            opt_chainout, "chain") < 0)
            goto err;

        if (!OSSL_CMP_CTX_reinit(cmp_ctx))
            goto err;
    }
    ret = 1;

 err:
    /* in case we ended up here on error without proper cleaning */
    cleanse(opt_keypass);
    cleanse(opt_newkeypass);
    cleanse(opt_otherpass);
#ifndef OPENSSL_NO_SOCK
    cleanse(opt_tls_keypass);
#endif
    cleanse(opt_secret);
    cleanse(opt_srv_keypass);
    cleanse(opt_srv_secret);

    if (ret != 1)
        OSSL_CMP_CTX_print_errors(cmp_ctx);

    ossl_cmp_mock_srv_free(OSSL_CMP_CTX_get_transfer_cb_arg(cmp_ctx));
#ifndef OPENSSL_NO_SOCK
    APP_HTTP_TLS_INFO_free(OSSL_CMP_CTX_get_http_cb_arg(cmp_ctx));
#endif
    X509_STORE_free(OSSL_CMP_CTX_get_certConf_cb_arg(cmp_ctx));
    OSSL_CMP_CTX_free(cmp_ctx);
    X509_VERIFY_PARAM_free(vpm);
    release_engine(engine);

    NCONF_free(conf); /* must not do as long as opt_... variables are used */
    OSSL_CMP_log_close();

    return ret == 0 ? EXIT_FAILURE : EXIT_SUCCESS; /* ret == -1 for -help */
}