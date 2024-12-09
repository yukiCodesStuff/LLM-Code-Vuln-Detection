		TSRMLS_DC)
{
	SSL_METHOD *method;
	long ssl_ctx_options = SSL_OP_ALL;
	
	if (sslsock->ssl_handle) {
		if (sslsock->s.is_blocked) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "SSL/TLS already set-up for this stream");
		return -1;
	}

#if OPENSSL_VERSION_NUMBER >= 0x0090605fL
	ssl_ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif
	SSL_CTX_set_options(sslsock->ctx, ssl_ctx_options);

#if OPENSSL_VERSION_NUMBER >= 0x0090806fL
	{
		zval **val;