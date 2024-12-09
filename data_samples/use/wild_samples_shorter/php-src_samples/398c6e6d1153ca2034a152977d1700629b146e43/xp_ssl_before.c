		TSRMLS_DC)
{
	SSL_METHOD *method;
	
	if (sslsock->ssl_handle) {
		if (sslsock->s.is_blocked) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "SSL/TLS already set-up for this stream");
		return -1;
	}

	SSL_CTX_set_options(sslsock->ctx, SSL_OP_ALL);

#if OPENSSL_VERSION_NUMBER >= 0x0090806fL
	{
		zval **val;