{
	char  fn[MAXPATHLEN];
	sdlPtr sdl = NULL;
	char* old_error_code = SOAP_GLOBAL(error_code);
	int uri_len = 0;
	php_stream_context *context=NULL;
	zval **tmp, **proxy_host, **proxy_port, *orig_context = NULL, *new_context = NULL;
	smart_str headers = {0};
	char* key = NULL;
	time_t t = time(0);

	if (strchr(uri,':') != NULL || IS_ABSOLUTE_PATH(uri, uri_len)) {
		uri_len = strlen(uri);
	} else if (VCWD_REALPATH(uri, fn) == NULL) {
		cache_wsdl = WSDL_CACHE_NONE;
	} else {
		uri = fn;
		uri_len = strlen(uri);
	}

	if ((cache_wsdl & WSDL_CACHE_MEMORY) && SOAP_GLOBAL(mem_cache)) {
		sdl_cache_bucket *p;

		if (SUCCESS == zend_hash_find(SOAP_GLOBAL(mem_cache), uri, uri_len+1, (void*)&p)) {
			if (p->time < t - SOAP_GLOBAL(cache_ttl)) {
				/* in-memory cache entry is expired */
				zend_hash_del(&EG(persistent_list), uri, uri_len+1);
			} else {
				return p->sdl;
			}
		}
	}

	if ((cache_wsdl & WSDL_CACHE_DISK) && (uri_len < MAXPATHLEN)) {
		time_t t = time(0);
		char md5str[33];
		PHP_MD5_CTX context;
		unsigned char digest[16];
		int len = strlen(SOAP_GLOBAL(cache_dir));
		time_t cached;
		char *user = php_get_current_user(TSRMLS_C);
		int user_len = user ? strlen(user) + 1 : 0;

		md5str[0] = '\0';
		PHP_MD5Init(&context);
		PHP_MD5Update(&context, (unsigned char*)uri, uri_len);
		PHP_MD5Final(digest, &context);
		make_digest(md5str, digest);
		key = emalloc(len+sizeof("/wsdl-")-1+user_len+sizeof(md5str));
		memcpy(key,SOAP_GLOBAL(cache_dir),len);
		memcpy(key+len,"/wsdl-",sizeof("/wsdl-")-1);
		len += sizeof("/wsdl-")-1;
		if (user_len) {
			memcpy(key+len, user, user_len-1);
			len += user_len-1;
			key[len++] = '-';
		}
		memcpy(key+len,md5str,sizeof(md5str));

		if ((sdl = get_sdl_from_cache(key, uri, t-SOAP_GLOBAL(cache_ttl), &cached TSRMLS_CC)) != NULL) {
			t = cached;
			efree(key);
			goto cache_in_memory;
		}
	}

	if (SUCCESS == zend_hash_find(Z_OBJPROP_P(this_ptr),
			"_stream_context", sizeof("_stream_context"), (void**)&tmp)) {
		context = php_stream_context_from_zval(*tmp, 0);
	} else {
		context = php_stream_context_alloc(TSRMLS_C);
	}

	if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_user_agent", sizeof("_user_agent"), (void **) &tmp) == SUCCESS &&
	    Z_TYPE_PP(tmp) == IS_STRING && Z_STRLEN_PP(tmp) > 0) {	
		smart_str_appends(&headers, "User-Agent: ");
		smart_str_appends(&headers, Z_STRVAL_PP(tmp));
		smart_str_appends(&headers, "\r\n");
	}

	if (zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_host", sizeof("_proxy_host"), (void **) &proxy_host) == SUCCESS &&
	    Z_TYPE_PP(proxy_host) == IS_STRING &&
	    zend_hash_find(Z_OBJPROP_P(this_ptr), "_proxy_port", sizeof("_proxy_port"), (void **) &proxy_port) == SUCCESS &&
	    Z_TYPE_PP(proxy_port) == IS_LONG) {
	    	zval str_port, *str_proxy;
	    	smart_str proxy = {0};
		str_port = **proxy_port;
		zval_copy_ctor(&str_port);
		convert_to_string(&str_port);
		smart_str_appends(&proxy,"tcp://");
		smart_str_appends(&proxy,Z_STRVAL_PP(proxy_host));
		smart_str_appends(&proxy,":");
		smart_str_appends(&proxy,Z_STRVAL(str_port));
		smart_str_0(&proxy);
		zval_dtor(&str_port);
		MAKE_STD_ZVAL(str_proxy);
		ZVAL_STRING(str_proxy, proxy.c, 1);
		smart_str_free(&proxy);
		
		if (!context) {
			context = php_stream_context_alloc(TSRMLS_C);
		}
		php_stream_context_set_option(context, "http", "proxy", str_proxy);
		zval_ptr_dtor(&str_proxy);

		if (uri_len < sizeof("https://")-1 ||
		    strncasecmp(uri, "https://", sizeof("https://")-1) != 0) {
			MAKE_STD_ZVAL(str_proxy);
			ZVAL_BOOL(str_proxy, 1);
			php_stream_context_set_option(context, "http", "request_fulluri", str_proxy);
			zval_ptr_dtor(&str_proxy);
		}

		proxy_authentication(this_ptr, &headers TSRMLS_CC);
	}

	basic_authentication(this_ptr, &headers TSRMLS_CC);

	/* Use HTTP/1.1 with "Connection: close" by default */
	if (php_stream_context_get_option(context, "http", "protocol_version", &tmp) == FAILURE) {
    	zval *http_version;
		MAKE_STD_ZVAL(http_version);
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
		ZVAL_STRING(str_headers, headers.c, 1);
		php_stream_context_set_option(context, "http", "header", str_headers);
		smart_str_free(&headers);
		zval_ptr_dtor(&str_headers);
	}

	if (context) {
		MAKE_STD_ZVAL(new_context);
		php_stream_context_to_zval(context, new_context);
		orig_context = php_libxml_switch_context(new_context TSRMLS_CC);
	}

	SOAP_GLOBAL(error_code) = "WSDL";

	sdl = load_wsdl(this_ptr, uri TSRMLS_CC);
	if (sdl) {
		sdl->is_persistent = 0;
	}

	SOAP_GLOBAL(error_code) = old_error_code;

	if (context) {
		php_libxml_switch_context(orig_context TSRMLS_CC);
		zval_ptr_dtor(&new_context);
	}

	if ((cache_wsdl & WSDL_CACHE_DISK) && key) {
		if (sdl) {
			add_sdl_to_cache(key, uri, t, sdl TSRMLS_CC);
		}
		efree(key);
	}

cache_in_memory:
	if (cache_wsdl & WSDL_CACHE_MEMORY) {
		if (sdl) {
			sdlPtr psdl;
			sdl_cache_bucket p;

			if (SOAP_GLOBAL(mem_cache) == NULL) {
				SOAP_GLOBAL(mem_cache) = malloc(sizeof(HashTable));
				zend_hash_init(SOAP_GLOBAL(mem_cache), 0, NULL, delete_psdl, 1);
			} else if (SOAP_GLOBAL(cache_limit) > 0 &&
			           SOAP_GLOBAL(cache_limit) <= zend_hash_num_elements(SOAP_GLOBAL(mem_cache))) {
				/* in-memory cache overflow */
				sdl_cache_bucket *q;
				HashPosition pos;
				time_t latest = t;
				char *key = NULL;
				uint key_len;
				ulong idx;

				for (zend_hash_internal_pointer_reset_ex(SOAP_GLOBAL(mem_cache), &pos);
					 zend_hash_get_current_data_ex(SOAP_GLOBAL(mem_cache), (void **) &q, &pos) == SUCCESS;
					 zend_hash_move_forward_ex(SOAP_GLOBAL(mem_cache), &pos)) {
					if (q->time < latest) {
						latest = q->time;
						zend_hash_get_current_key_ex(SOAP_GLOBAL(mem_cache), &key, &key_len, &idx, 0, &pos);
					}
				}
				if (key) {
					zend_hash_del(SOAP_GLOBAL(mem_cache), key, key_len);
				} else {
					return sdl;
				}
			}

			psdl = make_persistent_sdl(sdl TSRMLS_CC);
			psdl->is_persistent = 1;
			p.time = t;
			p.sdl = psdl;

			if (SUCCESS == zend_hash_update(SOAP_GLOBAL(mem_cache), uri,
											uri_len+1, (void*)&p, sizeof(sdl_cache_bucket), NULL)) {
				/* remove non-persitent sdl structure */
				delete_sdl_impl(sdl);
				/* and replace it with persistent one */
				sdl = psdl;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to register persistent entry");
				/* clean up persistent sdl */
				delete_psdl(&p);
				/* keep non-persistent sdl and return it */
			}
		}
	}

	return sdl;
}

/* Deletes */
void delete_sdl_impl(void *handle)
{
	sdlPtr tmp = (sdlPtr)handle;

	zend_hash_destroy(&tmp->functions);
	if (tmp->source) {
		efree(tmp->source);
	}
	if (tmp->target_ns) {
		efree(tmp->target_ns);
	}
	if (tmp->elements) {
		zend_hash_destroy(tmp->elements);
		efree(tmp->elements);
	}
	if (tmp->encoders) {
		zend_hash_destroy(tmp->encoders);
		efree(tmp->encoders);
	}
	if (tmp->types) {
		zend_hash_destroy(tmp->types);
		efree(tmp->types);
	}
	if (tmp->groups) {
		zend_hash_destroy(tmp->groups);
		efree(tmp->groups);
	}
	if (tmp->bindings) {
		zend_hash_destroy(tmp->bindings);
		efree(tmp->bindings);
	}
	if (tmp->requests) {
		zend_hash_destroy(tmp->requests);
		efree(tmp->requests);
	}
	efree(tmp);
}

void delete_sdl(void *handle)
{
	sdlPtr tmp = (sdlPtr)handle;

	if (!tmp->is_persistent) {
		delete_sdl_impl(tmp);
	}
}

static void delete_binding(void *data)
{
	sdlBindingPtr binding = *((sdlBindingPtr*)data);

	if (binding->location) {
		efree(binding->location);
	}
	if (binding->name) {
		efree(binding->name);
	}

	if (binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingPtr soapBind = binding->bindingAttributes;
		if (soapBind) {
			efree(soapBind);
		}
	}
	efree(binding);
}

static void delete_binding_persistent(void *data)
{
	sdlBindingPtr binding = *((sdlBindingPtr*)data);

	if (binding->location) {
		free(binding->location);
	}
	if (binding->name) {
		free(binding->name);
	}

	if (binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingPtr soapBind = binding->bindingAttributes;
		if (soapBind) {
			free(soapBind);
		}
	}
	free(binding);
}

static void delete_sdl_soap_binding_function_body(sdlSoapBindingFunctionBody body)
{
	if (body.ns) {
		efree(body.ns);
	}
	if (body.headers) {
		zend_hash_destroy(body.headers);
		efree(body.headers);
	}
}

static void delete_sdl_soap_binding_function_body_persistent(sdlSoapBindingFunctionBody body)
{
	if (body.ns) {
		free(body.ns);
	}
	if (body.headers) {
		zend_hash_destroy(body.headers);
		free(body.headers);
	}
}

static void delete_function(void *data)
{
	sdlFunctionPtr function = *((sdlFunctionPtr*)data);

	if (function->functionName) {
		efree(function->functionName);
	}
	if (function->requestName) {
		efree(function->requestName);
	}
	if (function->responseName) {
		efree(function->responseName);
	}
	if (function->requestParameters) {
		zend_hash_destroy(function->requestParameters);
		efree(function->requestParameters);
	}
	if (function->responseParameters) {
		zend_hash_destroy(function->responseParameters);
		efree(function->responseParameters);
	}
	if (function->faults) {
		zend_hash_destroy(function->faults);
		efree(function->faults);
	}

	if (function->bindingAttributes &&
	    function->binding && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr soapFunction = function->bindingAttributes;
		if (soapFunction->soapAction) {
			efree(soapFunction->soapAction);
		}
		delete_sdl_soap_binding_function_body(soapFunction->input);
		delete_sdl_soap_binding_function_body(soapFunction->output);
		efree(soapFunction);
	}
	efree(function);
}

static void delete_function_persistent(void *data)
{
	sdlFunctionPtr function = *((sdlFunctionPtr*)data);

	if (function->functionName) {
		free(function->functionName);
	}
	if (function->requestName) {
		free(function->requestName);
	}
	if (function->responseName) {
		free(function->responseName);
	}
	if (function->requestParameters) {
		zend_hash_destroy(function->requestParameters);
		free(function->requestParameters);
	}
	if (function->responseParameters) {
		zend_hash_destroy(function->responseParameters);
		free(function->responseParameters);
	}
	if (function->faults) {
		zend_hash_destroy(function->faults);
		free(function->faults);
	}

	if (function->bindingAttributes &&
	    function->binding && function->binding->bindingType == BINDING_SOAP) {
		sdlSoapBindingFunctionPtr soapFunction = function->bindingAttributes;
		if (soapFunction->soapAction) {
			free(soapFunction->soapAction);
		}
		delete_sdl_soap_binding_function_body_persistent(soapFunction->input);
		delete_sdl_soap_binding_function_body_persistent(soapFunction->output);
		free(soapFunction);
	}
	free(function);
}

static void delete_parameter(void *data)
{
	sdlParamPtr param = *((sdlParamPtr*)data);
	if (param->paramName) {
		efree(param->paramName);
	}
	efree(param);
}

static void delete_parameter_persistent(void *data)
{
	sdlParamPtr param = *((sdlParamPtr*)data);
	if (param->paramName) {
		free(param->paramName);
	}
	free(param);
}

static void delete_header(void *data)
{
	sdlSoapBindingFunctionHeaderPtr hdr = *((sdlSoapBindingFunctionHeaderPtr*)data);
	if (hdr->name) {
		efree(hdr->name);
	}
	if (hdr->ns) {
		efree(hdr->ns);
	}
	if (hdr->headerfaults) {
		zend_hash_destroy(hdr->headerfaults);
		efree(hdr->headerfaults);
	}
	efree(hdr);
}

static void delete_header_persistent(void *data)
{
	sdlSoapBindingFunctionHeaderPtr hdr = *((sdlSoapBindingFunctionHeaderPtr*)data);
	if (hdr->name) {
		free(hdr->name);
	}
	if (hdr->ns) {
		free(hdr->ns);
	}
	if (hdr->headerfaults) {
		zend_hash_destroy(hdr->headerfaults);
		free(hdr->headerfaults);
	}
	free(hdr);
}

static void delete_fault(void *data)
{
	sdlFaultPtr fault = *((sdlFaultPtr*)data);
	if (fault->name) {
		efree(fault->name);
	}
	if (fault->details) {
		zend_hash_destroy(fault->details);
		efree(fault->details);
	}
	if (fault->bindingAttributes) {
		sdlSoapBindingFunctionFaultPtr binding = (sdlSoapBindingFunctionFaultPtr)fault->bindingAttributes;

		if (binding->ns) {
			efree(binding->ns);
		}
		efree(fault->bindingAttributes);
	}
	efree(fault);
}

static void delete_fault_persistent(void *data)
{
	sdlFaultPtr fault = *((sdlFaultPtr*)data);
	if (fault->name) {
		free(fault->name);
	}
	if (fault->details) {
		zend_hash_destroy(fault->details);
		free(fault->details);
	}
	if (fault->bindingAttributes) {
		sdlSoapBindingFunctionFaultPtr binding = (sdlSoapBindingFunctionFaultPtr)fault->bindingAttributes;

		if (binding->ns) {
			free(binding->ns);
		}
		free(fault->bindingAttributes);
	}
	free(fault);
}

static void delete_document(void *doc_ptr)
{
	xmlDocPtr doc = *((xmlDocPtr*)doc_ptr);
	xmlFreeDoc(doc);
}

		    strncasecmp(uri, "https://", sizeof("https://")-1) != 0) {
			MAKE_STD_ZVAL(str_proxy);
			ZVAL_BOOL(str_proxy, 1);
			php_stream_context_set_option(context, "http", "request_fulluri", str_proxy);
			zval_ptr_dtor(&str_proxy);
		}

		proxy_authentication(this_ptr, &headers TSRMLS_CC);
	}

	basic_authentication(this_ptr, &headers TSRMLS_CC);

	/* Use HTTP/1.1 with "Connection: close" by default */
	if (php_stream_context_get_option(context, "http", "protocol_version", &tmp) == FAILURE) {
    	zval *http_version;
		MAKE_STD_ZVAL(http_version);
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
		ZVAL_STRING(str_headers, headers.c, 1);
		php_stream_context_set_option(context, "http", "header", str_headers);
		smart_str_free(&headers);
		zval_ptr_dtor(&str_headers);
	}

	if (context) {
		MAKE_STD_ZVAL(new_context);
		php_stream_context_to_zval(context, new_context);
		orig_context = php_libxml_switch_context(new_context TSRMLS_CC);
	}

	SOAP_GLOBAL(error_code) = "WSDL";

	sdl = load_wsdl(this_ptr, uri TSRMLS_CC);
	if (sdl) {
		sdl->is_persistent = 0;
	}

	SOAP_GLOBAL(error_code) = old_error_code;

	if (context) {
		php_libxml_switch_context(orig_context TSRMLS_CC);
		zval_ptr_dtor(&new_context);
	}

	if ((cache_wsdl & WSDL_CACHE_DISK) && key) {
		if (sdl) {
			add_sdl_to_cache(key, uri, t, sdl TSRMLS_CC);
		}
		efree(key);
	}

cache_in_memory:
	if (cache_wsdl & WSDL_CACHE_MEMORY) {
		if (sdl) {
			sdlPtr psdl;
			sdl_cache_bucket p;

			if (SOAP_GLOBAL(mem_cache) == NULL) {
				SOAP_GLOBAL(mem_cache) = malloc(sizeof(HashTable));
				zend_hash_init(SOAP_GLOBAL(mem_cache), 0, NULL, delete_psdl, 1);
			} else if (SOAP_GLOBAL(cache_limit) > 0 &&
			           SOAP_GLOBAL(cache_limit) <= zend_hash_num_elements(SOAP_GLOBAL(mem_cache))) {
				/* in-memory cache overflow */
				sdl_cache_bucket *q;
				HashPosition pos;
				time_t latest = t;
				char *key = NULL;
				uint key_len;
				ulong idx;

				for (zend_hash_internal_pointer_reset_ex(SOAP_GLOBAL(mem_cache), &pos);
					 zend_hash_get_current_data_ex(SOAP_GLOBAL(mem_cache), (void **) &q, &pos) == SUCCESS;
					 zend_hash_move_forward_ex(SOAP_GLOBAL(mem_cache), &pos)) {
					if (q->time < latest) {
						latest = q->time;
						zend_hash_get_current_key_ex(SOAP_GLOBAL(mem_cache), &key, &key_len, &idx, 0, &pos);
					}
				}
				if (key) {
					zend_hash_del(SOAP_GLOBAL(mem_cache), key, key_len);
				} else {
					return sdl;
				}
			}

			psdl = make_persistent_sdl(sdl TSRMLS_CC);
			psdl->is_persistent = 1;
			p.time = t;
			p.sdl = psdl;

			if (SUCCESS == zend_hash_update(SOAP_GLOBAL(mem_cache), uri,
											uri_len+1, (void*)&p, sizeof(sdl_cache_bucket), NULL)) {
				/* remove non-persitent sdl structure */
				delete_sdl_impl(sdl);
				/* and replace it with persistent one */
				sdl = psdl;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to register persistent entry");
				/* clean up persistent sdl */
				delete_psdl(&p);
				/* keep non-persistent sdl and return it */
			}
		}
	}

	return sdl;
}
	if (php_stream_context_get_option(context, "http", "protocol_version", &tmp) == FAILURE) {
    	zval *http_version;
		MAKE_STD_ZVAL(http_version);
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