	smart_str headers = {0};
	char* key = NULL;
	time_t t = time(0);
	zend_bool has_proxy_authorization = 0;
	zend_bool has_authorization = 0;

	if (strchr(uri,':') != NULL || IS_ABSOLUTE_PATH(uri, uri_len)) {
		uri_len = strlen(uri);
	} else if (VCWD_REALPATH(uri, fn) == NULL) {
			zval_ptr_dtor(&str_proxy);
		}

		has_proxy_authorization = proxy_authentication(this_ptr, &headers TSRMLS_CC);
	}

	has_authorization = basic_authentication(this_ptr, &headers TSRMLS_CC);

	/* Use HTTP/1.1 with "Connection: close" by default */
	if (php_stream_context_get_option(context, "http", "protocol_version", &tmp) == FAILURE) {
    	zval *http_version;
		ZVAL_DOUBLE(http_version, 1.1);
		php_stream_context_set_option(context, "http", "protocol_version", http_version);
		zval_ptr_dtor(&http_version);
		smart_str_appendl(&headers, "Connection: close\r\n", sizeof("Connection: close\r\n")-1);
	}

	if (headers.len > 0) {
		zval *str_headers;

		if (!context) {
			context = php_stream_context_alloc(TSRMLS_C);
		} else {
			http_context_headers(context, has_authorization, has_proxy_authorization, 0, &headers TSRMLS_CC);
		}

		smart_str_0(&headers);
		MAKE_STD_ZVAL(str_headers);