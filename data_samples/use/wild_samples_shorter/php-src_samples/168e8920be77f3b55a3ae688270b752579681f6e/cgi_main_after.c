#include "php_main.h"
#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/url.h"

#ifdef PHP_WIN32
# include <io.h>
# include <fcntl.h>
#ifndef PHP_WIN32
	int status = 0;
#endif
	char *query_string;
	char *decoded_query_string;
	int skip_getopt = 0;

#if 0 && defined(PHP_DEBUG)
	/* IIS is always making things more difficult.  This allows
	 * us to stop PHP and attach a debugger before much gets started */
		}
	}

	if(query_string = getenv("QUERY_STRING")) {
		decoded_query_string = strdup(query_string);
		php_url_decode(decoded_query_string, strlen(decoded_query_string));
		if(*decoded_query_string == '-' && strchr(decoded_query_string, '=') == NULL) {
			skip_getopt = 1;
		}
		free(decoded_query_string);
	}

	while (!skip_getopt && (c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
		switch (c) {
			case 'c':
				if (cgi_sapi_module.php_ini_path_override) {
					free(cgi_sapi_module.php_ini_path_override);