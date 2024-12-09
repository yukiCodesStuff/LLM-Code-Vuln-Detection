	smart_str headers = {0};
	char* key = NULL;
	time_t t = time(0);

	if (strchr(uri,':') != NULL || IS_ABSOLUTE_PATH(uri, uri_len)) {
		uri_len = strlen(uri);
	} else if (VCWD_REALPATH(uri, fn) == NULL) {
			zval_ptr_dtor(&str_proxy);
		}

		proxy_authentication(this_ptr, &headers TSRMLS_CC);
	}

	basic_authentication(this_ptr, &headers TSRMLS_CC);

	/* Use HTTP/1.1 with "Connection: close" by default */
	if (php_stream_context_get_option(context, "http", "protocol_version", &tmp) == FAILURE) {
    	zval *http_version;
		ZVAL_DOUBLE(http_version, 1.1);
		php_stream_context_set_option(context, "http", "protocol_version", http_version);
		zval_ptr_dtor(&http_version);
		smart_str_appendl(&headers, "Connection: close", sizeof("Connection: close")-1);
	}

	if (headers.len > 0) {
		zval *str_headers;

		if (!context) {
			context = php_stream_context_alloc(TSRMLS_C);
		}

		smart_str_0(&headers);
		MAKE_STD_ZVAL(str_headers);