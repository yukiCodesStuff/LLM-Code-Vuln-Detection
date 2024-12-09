#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"

#ifdef PHP_WIN32
# include <io.h>
# include <fcntl.h>
#ifndef PHP_WIN32
	int status = 0;
#endif

#if 0 && defined(PHP_DEBUG)
	/* IIS is always making things more difficult.  This allows
	 * us to stop PHP and attach a debugger before much gets started */
		}
	}

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
		switch (c) {
			case 'c':
				if (cgi_sapi_module.php_ini_path_override) {
					free(cgi_sapi_module.php_ini_path_override);