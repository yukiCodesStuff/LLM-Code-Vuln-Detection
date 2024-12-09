
PHP_MINFO_FUNCTION(readline)
{
	return PHP_MINFO(cli_readline)(ZEND_MODULE_INFO_FUNC_ARGS_PASSTHRU);
}

/* }}} */
