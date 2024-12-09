	/* turn off magic_quotes while importing environment variables */
	int magic_quotes_gpc = PG(magic_quotes_gpc);

	if (PG(magic_quotes_gpc)) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}

	for (env = environ; env != NULL && *env != NULL; env++) {
	if (t != buf && t != NULL) {
		efree(t);
	}
	PG(magic_quotes_gpc) = magic_quotes_gpc;
}

zend_bool php_std_auto_global_callback(char *name, uint name_len TSRMLS_DC)
{
		zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_SERVER]);
	}
	PG(http_globals)[TRACK_VARS_SERVER] = array_ptr;
	if (PG(magic_quotes_gpc)) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}

	/* Server variables */
		php_register_variable_ex("REQUEST_TIME", &new_entry, array_ptr TSRMLS_CC);
	}

	PG(magic_quotes_gpc) = magic_quotes_gpc;
}
/* }}} */

/* {{{ php_autoglobal_merge