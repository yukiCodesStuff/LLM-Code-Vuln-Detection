	filter_arg = (array_ptr == PG(http_globals)[TRACK_VARS_ENV])?PARSE_ENV:PARSE_SERVER;

	/* turn off magic_quotes while importing environment variables */
	if (PG(magic_quotes_gpc)) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
	for (zend_hash_internal_pointer_reset_ex(request->env, &pos);
	     zend_hash_get_current_key_ex(request->env, &var, &var_len, &idx, 0, &pos) == HASH_KEY_IS_STRING &&
			php_register_variable_safe(var, *val, new_val_len, array_ptr TSRMLS_CC);
		}
	}
	PG(magic_quotes_gpc) = magic_quotes_gpc;
}

static void sapi_cgi_register_variables(zval *track_vars_array TSRMLS_DC)
{