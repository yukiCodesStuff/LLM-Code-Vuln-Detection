	/* turn off magic_quotes while importing environment variables */
	int magic_quotes_gpc = PG(magic_quotes_gpc);

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}

	for (env = environ; env != NULL && *env != NULL; env++) {
	if (t != buf && t != NULL) {
		efree(t);
	}

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}

zend_bool php_std_auto_global_callback(char *name, uint name_len TSRMLS_DC)
{
		zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_SERVER]);
	}
	PG(http_globals)[TRACK_VARS_SERVER] = array_ptr;
	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}

	/* Server variables */
		php_register_variable_ex("REQUEST_TIME", &new_entry, array_ptr TSRMLS_CC);
	}

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}
/* }}} */

/* {{{ php_autoglobal_merge