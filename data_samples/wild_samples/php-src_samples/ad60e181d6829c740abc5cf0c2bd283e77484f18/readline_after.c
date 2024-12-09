	if (_prepped_callback) {
		rl_callback_handler_remove();
		zval_ptr_dtor(&_prepped_callback);
		_prepped_callback = 0;
	}
#endif

	return SUCCESS;
}

PHP_MINFO_FUNCTION(readline)
{
	PHP_MINFO(cli_readline)(ZEND_MODULE_INFO_FUNC_ARGS_PASSTHRU);
}