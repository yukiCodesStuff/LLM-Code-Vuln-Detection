	) {
		zval_dtor(array_ptr);
		*array_ptr = *PG(http_globals)[TRACK_VARS_SERVER];
		INIT_PZVAL(array_ptr);
		zval_copy_ctor(array_ptr);
		return;
	}

	/* call php's original import as a catch-all */
	php_php_import_environment_variables(array_ptr TSRMLS_CC);

	request = (fcgi_request*) SG(server_context);
	magic_quotes_gpc = PG(magic_quotes_gpc);
	filter_arg = (array_ptr == PG(http_globals)[TRACK_VARS_ENV])?PARSE_ENV:PARSE_SERVER;

	/* turn off magic_quotes while importing environment variables */
	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
		if (sapi_module.input_filter(filter_arg, var, val, strlen(*val), &new_val_len TSRMLS_CC)) {
			php_register_variable_safe(var, *val, new_val_len, array_ptr TSRMLS_CC);
		}
	}
	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}

static void sapi_cgi_register_variables(zval *track_vars_array TSRMLS_DC)
{
	unsigned int php_self_len;
	char *php_self;

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	php_import_environment_variables(track_vars_array TSRMLS_CC);

	if (CGIG(fix_pathinfo)) {
		char *script_name = SG(request_info).request_uri;
		unsigned int script_name_len = script_name ? strlen(script_name) : 0;
		char *path_info = sapi_cgibin_getenv("PATH_INFO", sizeof("PATH_INFO") - 1 TSRMLS_CC);
		unsigned int path_info_len = path_info ? strlen(path_info) : 0;

		php_self_len = script_name_len + path_info_len;
		php_self = emalloc(php_self_len + 1);

		/* Concat script_name and path_info into php_self */
		if (script_name) {
			memcpy(php_self, script_name, script_name_len + 1);
		}
		if (path_info) {
			memcpy(php_self + script_name_len, path_info, path_info_len + 1);
		}

		/* Build the special-case PHP_SELF variable for the CGI version */
		if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, php_self_len, &php_self_len TSRMLS_CC)) {
			php_register_variable_safe("PHP_SELF", php_self, php_self_len, track_vars_array TSRMLS_CC);
		}
		efree(php_self);
	} else {
		php_self = SG(request_info).request_uri ? SG(request_info).request_uri : "";
		php_self_len = strlen(php_self);
		if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, php_self_len, &php_self_len TSRMLS_CC)) {
			php_register_variable_safe("PHP_SELF", php_self, php_self_len, track_vars_array TSRMLS_CC);
		}