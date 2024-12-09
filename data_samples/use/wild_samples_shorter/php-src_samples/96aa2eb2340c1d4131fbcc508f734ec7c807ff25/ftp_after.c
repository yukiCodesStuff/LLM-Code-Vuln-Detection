{
#if HAVE_OPENSSL_EXT
	SSL_CTX	*ctx = NULL;
	long ssl_ctx_options = SSL_OP_ALL;
#endif
	if (ftp == NULL) {
		return 0;
	}
			return 0;
		}

#if OPENSSL_VERSION_NUMBER >= 0x0090605fL
		ssl_ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif
		SSL_CTX_set_options(ctx, ssl_ctx_options);

		ftp->ssl_handle = SSL_new(ctx);
		if (ftp->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create the SSL handle");

#if HAVE_OPENSSL_EXT
	SSL_CTX		*ctx;
	long ssl_ctx_options = SSL_OP_ALL;
#endif

	if (data->fd != -1) {
		goto data_accepted;
			return 0;
		}

#if OPENSSL_VERSION_NUMBER >= 0x0090605fL
		ssl_ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif
		SSL_CTX_set_options(ctx, ssl_ctx_options);

		data->ssl_handle = SSL_new(ctx);
		if (data->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_accept: failed to create the SSL handle");