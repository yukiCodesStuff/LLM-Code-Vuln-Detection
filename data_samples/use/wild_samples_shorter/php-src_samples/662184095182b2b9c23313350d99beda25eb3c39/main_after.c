
	zuv.html_errors = 1;
	zuv.import_use_extension = ".php";
	zuv.import_use_extension_length = (uint)strlen(zuv.import_use_extension);
	php_startup_auto_globals();
	zend_set_utility_values(&zuv);
	php_startup_sapi_content_types();
