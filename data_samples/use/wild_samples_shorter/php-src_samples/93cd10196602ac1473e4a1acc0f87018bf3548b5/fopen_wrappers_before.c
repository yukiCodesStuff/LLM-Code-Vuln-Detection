#else
		if (strncmp(resolved_basedir, resolved_name, resolved_basedir_len) == 0) {
#endif
			/* File is in the right directory */
			return 0;
		} else {
			/* /openbasedir/ and /openbasedir are the same directory */
			if (resolved_basedir_len == (resolved_name_len + 1) && resolved_basedir[resolved_basedir_len - 1] == PHP_DIR_SEPARATOR) {
#if defined(PHP_WIN32) || defined(NETWARE)