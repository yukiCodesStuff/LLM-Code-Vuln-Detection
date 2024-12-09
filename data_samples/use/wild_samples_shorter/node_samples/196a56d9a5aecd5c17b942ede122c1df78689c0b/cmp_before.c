        if ((info = OPENSSL_zalloc(sizeof(*info))) == NULL)
            goto err;
        (void)OSSL_CMP_CTX_set_http_cb_arg(ctx, info);
        /* info will be freed along with CMP ctx */
        info->server = opt_server;
        info->port = server_port;
        /* workaround for callback design flaw, see #17088: */
        info->use_proxy = proxy_host != NULL;
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