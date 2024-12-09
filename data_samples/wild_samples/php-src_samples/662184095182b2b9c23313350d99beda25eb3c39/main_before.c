	if (php_init_stream_wrappers(module_number) == FAILURE)	{
		php_printf("PHP:  Unable to initialize stream url wrappers.\n");
		return FAILURE;
	}

	zuv.html_errors = 1;
	zuv.import_use_extension = ".php";
	php_startup_auto_globals();
	zend_set_utility_values(&zuv);
	php_startup_sapi_content_types();

	/* startup extensions statically compiled in */
	if (php_register_internal_extensions_func() == FAILURE) {
		php_printf("Unable to start builtin modules\n");
		return FAILURE;
	}