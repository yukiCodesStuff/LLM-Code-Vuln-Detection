{
#if HAVE_OPENSSL_EXT
	SSL_CTX	*ctx = NULL;
#endif
	if (ftp == NULL) {
		return 0;
	}
			return 0;
		}

		SSL_CTX_set_options(ctx, SSL_OP_ALL);

		ftp->ssl_handle = SSL_new(ctx);
		if (ftp->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create the SSL handle");

#if HAVE_OPENSSL_EXT
	SSL_CTX		*ctx;
#endif

	if (data->fd != -1) {
		goto data_accepted;
			return 0;
		}

		SSL_CTX_set_options(ctx, SSL_OP_ALL);

		data->ssl_handle = SSL_new(ctx);
		if (data->ssl_handle == NULL) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "data_accept: failed to create the SSL handle");