		else {
			*found_browser_entry = *browser;
		}
	}

	return 0;
}
/* }}} */

/* {{{ proto mixed get_browser([string browser_name [, bool return_array]])
   Get information about the capabilities of a browser. If browser_name is omitted or null, HTTP_USER_AGENT is used. Returns an object by default; if return_array is true, returns an array. */
PHP_FUNCTION(get_browser)
{
	char *agent_name = NULL;
	int agent_name_len = 0;
	zend_bool return_array = 0;
	zval **agent, **z_agent_name, **http_user_agent;
	zval *found_browser_entry, *tmp_copy;
	char *lookup_browser_name;
	browser_data *bdata;

	if (BROWSCAP_G(activation_bdata).filename[0] != '\0') {
		bdata = &BROWSCAP_G(activation_bdata);
		if (bdata->htab == NULL) { /* not initialized yet */
			if (browscap_read_file(bdata->filename, bdata, 0 TSRMLS_CC) == FAILURE) {
				RETURN_FALSE;
			}
		}
	} else {
		if (!global_bdata.htab) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "browscap ini directive not set");
			RETURN_FALSE;
		}
		bdata = &global_bdata;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s!b", &agent_name, &agent_name_len, &return_array) == FAILURE) {
		return;
	}

	if (agent_name == NULL) {
		zend_is_auto_global("_SERVER", sizeof("_SERVER") - 1 TSRMLS_CC);
		if (!PG(http_globals)[TRACK_VARS_SERVER] ||
			zend_hash_find(HASH_OF(PG(http_globals)[TRACK_VARS_SERVER]), "HTTP_USER_AGENT", sizeof("HTTP_USER_AGENT"), (void **) &http_user_agent) == FAILURE
		) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "HTTP_USER_AGENT variable is not set, cannot determine user agent name");
			RETURN_FALSE;
		}
		agent_name = Z_STRVAL_PP(http_user_agent);
		agent_name_len = Z_STRLEN_PP(http_user_agent);
	}

	lookup_browser_name = estrndup(agent_name, agent_name_len);
	php_strtolower(lookup_browser_name, agent_name_len);

	if (zend_hash_find(bdata->htab, lookup_browser_name, agent_name_len + 1, (void **) &agent) == FAILURE) {
		found_browser_entry = NULL;
		zend_hash_apply_with_arguments(bdata->htab TSRMLS_CC, (apply_func_args_t) browser_reg_compare, 3, lookup_browser_name, agent_name_len, &found_browser_entry);

		if (found_browser_entry) {
			agent = &found_browser_entry;
		} else if (zend_hash_find(bdata->htab, DEFAULT_SECTION_NAME, sizeof(DEFAULT_SECTION_NAME), (void **) &agent) == FAILURE) {
			efree(lookup_browser_name);
			RETURN_FALSE;
		}
	}

	if (return_array) {
		array_init(return_value);
		zend_hash_copy(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}
	else {
		object_init(return_value);
		zend_hash_copy(Z_OBJPROP_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}

	while (zend_hash_find(Z_ARRVAL_PP(agent), "parent", sizeof("parent"), (void **) &z_agent_name) == SUCCESS) {
		if (zend_hash_find(bdata->htab, Z_STRVAL_PP(z_agent_name), Z_STRLEN_PP(z_agent_name) + 1, (void **)&agent) == FAILURE) {
			break;
		}

		if (return_array) {
			zend_hash_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *), 0);
		}
		else {
			zend_hash_merge(Z_OBJPROP_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *), 0);
		}
	}

	efree(lookup_browser_name);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
	if (return_array) {
		array_init(return_value);
		zend_hash_copy(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *));
	}
		if (zend_hash_find(bdata->htab, Z_STRVAL_PP(z_agent_name), Z_STRLEN_PP(z_agent_name) + 1, (void **)&agent) == FAILURE) {
			break;
		}

		if (return_array) {
			zend_hash_merge(Z_ARRVAL_P(return_value), Z_ARRVAL_PP(agent), (copy_ctor_func_t) zval_add_ref, (void *) &tmp_copy, sizeof(zval *), 0);
		}