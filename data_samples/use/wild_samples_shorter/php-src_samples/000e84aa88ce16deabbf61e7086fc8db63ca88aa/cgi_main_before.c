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
	}

	zend_first_try {
		while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 1, 2)) != -1) {
			switch (c) {
				case 'T':
					benchmark = 1;
					repeats = atoi(php_optarg);