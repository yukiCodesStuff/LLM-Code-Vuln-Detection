#endif
/* }}} */

PHPAPI int (*php_register_internal_extensions_func)(TSRMLS_D) = php_register_internal_extensions;

#ifndef ZTS
php_core_globals core_globals;
			if ((envpath = getenv("PATH")) != NULL) {
				char *search_dir, search_path[MAXPATHLEN];
				char *last = NULL;

				path = estrdup(envpath);
				search_dir = php_strtok_r(path, ":", &last);

				while (search_dir) {
					snprintf(search_path, MAXPATHLEN, "%s/%s", search_dir, sapi_module.executable_location);
					if (VCWD_REALPATH(search_path, binary_location) && !VCWD_ACCESS(binary_location, X_OK)) {
						found = 1;
						break;
					}
					search_dir = php_strtok_r(NULL, ":", &last);