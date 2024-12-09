		) {
			cgi = 1;
		}
	}

	if(query_string = getenv("QUERY_STRING")) {
		decoded_query_string = strdup(query_string);
		php_url_decode(decoded_query_string, strlen(decoded_query_string));
		if(*decoded_query_string == '-' && strchr(decoded_query_string, '=') == NULL) {
			skip_getopt = 1;
		}
	} else {
		parent = 0;
	}

#endif /* WIN32 */
	}

	zend_first_try {
		while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 1, 2)) != -1) {
			switch (c) {
				case 'T':
					benchmark = 1;
					repeats = atoi(php_optarg);
#ifdef HAVE_GETTIMEOFDAY
					gettimeofday(&start, NULL);
#else
					time(&start);
#endif
					break;
				case 'h':
				case '?':
					if (request) {
						fcgi_destroy_request(request);
					}
					fcgi_shutdown();
					no_headers = 1;
					SG(headers_sent) = 1;
					php_cgi_usage(argv[0]);
					php_output_end_all(TSRMLS_C);
					exit_status = 0;
					goto out;
			}
		}
		php_optind = orig_optind;
		php_optarg = orig_optarg;

		/* start of FAST CGI loop */
		/* Initialise FastCGI request structure */
#ifdef PHP_WIN32
		/* attempt to set security impersonation for fastcgi
		 * will only happen on NT based OS, others will ignore it. */
		if (fastcgi && CGIG(impersonate)) {
			fcgi_impersonate();
		}
#endif
		while (!fastcgi || fcgi_accept_request(request) >= 0) {
			SG(server_context) = fastcgi ? (void *) request : (void *) 1;
			init_request_info(request TSRMLS_CC);
			CG(interactive) = 0;

			if (!cgi && !fastcgi) {
				while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
					switch (c) {

						case 'a':	/* interactive mode */
							printf("Interactive mode enabled\n\n");
							CG(interactive) = 1;
							break;

						case 'C': /* don't chdir to the script directory */
							SG(options) |= SAPI_OPTION_NO_CHDIR;
							break;

						case 'e': /* enable extended info output */
							CG(compiler_options) |= ZEND_COMPILE_EXTENDED_INFO;
							break;

						case 'f': /* parse file */
							if (script_file) {
								efree(script_file);
							}
							script_file = estrdup(php_optarg);
							no_headers = 1;
							break;

						case 'i': /* php info & quit */
							if (script_file) {
								efree(script_file);
							}
							if (php_request_startup(TSRMLS_C) == FAILURE) {
								SG(server_context) = NULL;
								php_module_shutdown(TSRMLS_C);
								return FAILURE;
							}
							if (no_headers) {
								SG(headers_sent) = 1;
								SG(request_info).no_headers = 1;
							}
							php_print_info(0xFFFFFFFF TSRMLS_CC);
							php_request_shutdown((void *) 0);
							fcgi_shutdown();
							exit_status = 0;
							goto out;

						case 'l': /* syntax check mode */
							no_headers = 1;
							behavior = PHP_MODE_LINT;
							break;

						case 'm': /* list compiled in modules */
							if (script_file) {
								efree(script_file);
							}
							SG(headers_sent) = 1;
							php_printf("[PHP Modules]\n");
							print_modules(TSRMLS_C);
							php_printf("\n[Zend Modules]\n");
							print_extensions(TSRMLS_C);
							php_printf("\n");
							php_output_end_all(TSRMLS_C);
							fcgi_shutdown();
							exit_status = 0;
							goto out;

#if 0 /* not yet operational, see also below ... */
						case '': /* generate indented source mode*/
							behavior=PHP_MODE_INDENT;
							break;
#endif

						case 'q': /* do not generate HTTP headers */
							no_headers = 1;
							break;

						case 'v': /* show php version & quit */
							if (script_file) {
								efree(script_file);
							}
							no_headers = 1;
							if (php_request_startup(TSRMLS_C) == FAILURE) {
								SG(server_context) = NULL;
								php_module_shutdown(TSRMLS_C);
								return FAILURE;
							}
							if (no_headers) {
								SG(headers_sent) = 1;
								SG(request_info).no_headers = 1;
							}
#if ZEND_DEBUG
							php_printf("PHP %s (%s) (built: %s %s) (DEBUG)\nCopyright (c) 1997-2012 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#else
							php_printf("PHP %s (%s) (built: %s %s)\nCopyright (c) 1997-2012 The PHP Group\n%s", PHP_VERSION, sapi_module.name, __DATE__, __TIME__, get_zend_version());
#endif
							php_request_shutdown((void *) 0);
							fcgi_shutdown();
							exit_status = 0;
							goto out;

						case 'w':
							behavior = PHP_MODE_STRIP;
							break;

						case 'z': /* load extension file */
							zend_load_extension(php_optarg);
							break;

						default:
							break;
					}
				}

				if (script_file) {
					/* override path_translated if -f on command line */
					STR_FREE(SG(request_info).path_translated);
					SG(request_info).path_translated = script_file;
					/* before registering argv to module exchange the *new* argv[0] */
					/* we can achieve this without allocating more memory */
					SG(request_info).argc = argc - (php_optind - 1);
					SG(request_info).argv = &argv[php_optind - 1];
					SG(request_info).argv[0] = script_file;
				} else if (argc > php_optind) {
					/* file is on command line, but not in -f opt */
					STR_FREE(SG(request_info).path_translated);
					SG(request_info).path_translated = estrdup(argv[php_optind]);
					/* arguments after the file are considered script args */
					SG(request_info).argc = argc - php_optind;
					SG(request_info).argv = &argv[php_optind];
				}

				if (no_headers) {
					SG(headers_sent) = 1;
					SG(request_info).no_headers = 1;
				}

				/* all remaining arguments are part of the query string
				 * this section of code concatenates all remaining arguments
				 * into a single string, seperating args with a &
				 * this allows command lines like:
				 *
				 *  test.php v1=test v2=hello+world!
				 *  test.php "v1=test&v2=hello world!"
				 *  test.php v1=test "v2=hello world!"
				*/
				if (!SG(request_info).query_string && argc > php_optind) {
					int slen = strlen(PG(arg_separator).input);
					len = 0;
					for (i = php_optind; i < argc; i++) {
						if (i < (argc - 1)) {
							len += strlen(argv[i]) + slen;
						} else {
							len += strlen(argv[i]);
						}
					}

					len += 2;
					s = malloc(len);
					*s = '\0';			/* we are pretending it came from the environment  */
					for (i = php_optind; i < argc; i++) {
						strlcat(s, argv[i], len);
						if (i < (argc - 1)) {
							strlcat(s, PG(arg_separator).input, len);
						}
					}
					SG(request_info).query_string = s;
					free_query_string = 1;
				}
			} /* end !cgi && !fastcgi */

			/*
				we never take stdin if we're (f)cgi, always
				rely on the web server giving us the info
				we need in the environment.
			*/
			if (SG(request_info).path_translated || cgi || fastcgi) {
				file_handle.type = ZEND_HANDLE_FILENAME;
				file_handle.filename = SG(request_info).path_translated;
				file_handle.handle.fp = NULL;
			} else {
				file_handle.filename = "-";
				file_handle.type = ZEND_HANDLE_FP;
				file_handle.handle.fp = stdin;
			}

			file_handle.opened_path = NULL;
			file_handle.free_filename = 0;

			/* request startup only after we've done all we can to
			 * get path_translated */
			if (php_request_startup(TSRMLS_C) == FAILURE) {
				if (fastcgi) {
					fcgi_finish_request(request, 1);
				}
				SG(server_context) = NULL;
				php_module_shutdown(TSRMLS_C);
				return FAILURE;
			}
			if (no_headers) {
				SG(headers_sent) = 1;
				SG(request_info).no_headers = 1;
			}

			/*
				at this point path_translated will be set if:
				1. we are running from shell and got filename was there
				2. we are running as cgi or fastcgi
			*/
			if (cgi || fastcgi || SG(request_info).path_translated) {
				if (php_fopen_primary_script(&file_handle TSRMLS_CC) == FAILURE) {
					zend_try {
						if (errno == EACCES) {
							SG(sapi_headers).http_response_code = 403;
							PUTS("Access denied.\n");
						} else {
							SG(sapi_headers).http_response_code = 404;
							PUTS("No input file specified.\n");
						}
					} zend_catch {
					} zend_end_try();
					/* we want to serve more requests if this is fastcgi
					 * so cleanup and continue, request shutdown is
					 * handled later */
					if (fastcgi) {
						goto fastcgi_request_done;
					}

					STR_FREE(SG(request_info).path_translated);

					if (free_query_string && SG(request_info).query_string) {
						free(SG(request_info).query_string);
						SG(request_info).query_string = NULL;
					}

					php_request_shutdown((void *) 0);
					SG(server_context) = NULL;
					php_module_shutdown(TSRMLS_C);
					sapi_shutdown();
#ifdef ZTS
					tsrm_shutdown();
#endif
					return FAILURE;
				}
			}

			if (CGIG(check_shebang_line)) {
				/* #!php support */
				switch (file_handle.type) {
					case ZEND_HANDLE_FD:
						if (file_handle.handle.fd < 0) {
							break;
						}
						file_handle.type = ZEND_HANDLE_FP;
						file_handle.handle.fp = fdopen(file_handle.handle.fd, "rb");
						/* break missing intentionally */
					case ZEND_HANDLE_FP:
						if (!file_handle.handle.fp ||
						    (file_handle.handle.fp == stdin)) {
							break;
						}
						c = fgetc(file_handle.handle.fp);
						if (c == '#') {
							while (c != '\n' && c != '\r' && c != EOF) {
								c = fgetc(file_handle.handle.fp);	/* skip to end of line */
							}
							/* handle situations where line is terminated by \r\n */
							if (c == '\r') {
								if (fgetc(file_handle.handle.fp) != '\n') {
									long pos = ftell(file_handle.handle.fp);
									fseek(file_handle.handle.fp, pos - 1, SEEK_SET);
								}
							}
							CG(start_lineno) = 2;
						} else {
							rewind(file_handle.handle.fp);
						}
						break;
					case ZEND_HANDLE_STREAM:
						c = php_stream_getc((php_stream*)file_handle.handle.stream.handle);
						if (c == '#') {
							while (c != '\n' && c != '\r' && c != EOF) {
								c = php_stream_getc((php_stream*)file_handle.handle.stream.handle);	/* skip to end of line */
							}
							/* handle situations where line is terminated by \r\n */
							if (c == '\r') {
								if (php_stream_getc((php_stream*)file_handle.handle.stream.handle) != '\n') {
									long pos = php_stream_tell((php_stream*)file_handle.handle.stream.handle);
									php_stream_seek((php_stream*)file_handle.handle.stream.handle, pos - 1, SEEK_SET);
								}
							}
							CG(start_lineno) = 2;
						} else {
							php_stream_rewind((php_stream*)file_handle.handle.stream.handle);
						}
						break;
					case ZEND_HANDLE_MAPPED:
						if (file_handle.handle.stream.mmap.buf[0] == '#') {
						    int i = 1;

						    c = file_handle.handle.stream.mmap.buf[i++];
							while (c != '\n' && c != '\r' && c != EOF) {
								c = file_handle.handle.stream.mmap.buf[i++];
							}
							if (c == '\r') {
								if (file_handle.handle.stream.mmap.buf[i] == '\n') {
									i++;
								}
							}
							file_handle.handle.stream.mmap.buf += i;
							file_handle.handle.stream.mmap.len -= i;
						}
						break;
					default:
						break;
				}
			}

			switch (behavior) {
				case PHP_MODE_STANDARD:
					php_execute_script(&file_handle TSRMLS_CC);
					break;
				case PHP_MODE_LINT:
					PG(during_request_startup) = 0;
					exit_status = php_lint_script(&file_handle TSRMLS_CC);
					if (exit_status == SUCCESS) {
						zend_printf("No syntax errors detected in %s\n", file_handle.filename);
					} else {
						zend_printf("Errors parsing %s\n", file_handle.filename);
					}
					break;
				case PHP_MODE_STRIP:
					if (open_file_for_scanning(&file_handle TSRMLS_CC) == SUCCESS) {
						zend_strip(TSRMLS_C);
						zend_file_handle_dtor(&file_handle TSRMLS_CC);
						php_output_teardown();
					}
					return SUCCESS;
					break;
				case PHP_MODE_HIGHLIGHT:
					{
						zend_syntax_highlighter_ini syntax_highlighter_ini;

						if (open_file_for_scanning(&file_handle TSRMLS_CC) == SUCCESS) {
							php_get_highlight_struct(&syntax_highlighter_ini);
							zend_highlight(&syntax_highlighter_ini TSRMLS_CC);
							if (fastcgi) {
								goto fastcgi_request_done;
							}
							zend_file_handle_dtor(&file_handle TSRMLS_CC);
							php_output_teardown();
						}
						return SUCCESS;
					}
					break;
#if 0
				/* Zeev might want to do something with this one day */
				case PHP_MODE_INDENT:
					open_file_for_scanning(&file_handle TSRMLS_CC);
					zend_indent();
					zend_file_handle_dtor(&file_handle TSRMLS_CC);
					php_output_teardown();
					return SUCCESS;
					break;
#endif
			}

fastcgi_request_done:
			{
				STR_FREE(SG(request_info).path_translated);

				php_request_shutdown((void *) 0);

				if (exit_status == 0) {
					exit_status = EG(exit_status);
				}

				if (free_query_string && SG(request_info).query_string) {
					free(SG(request_info).query_string);
					SG(request_info).query_string = NULL;
				}
			}

			if (!fastcgi) {
				if (benchmark) {
					repeats--;
					if (repeats > 0) {
						script_file = NULL;
						php_optind = orig_optind;
						php_optarg = orig_optarg;
						continue;
					}
				}
				break;
			}

			/* only fastcgi will get here */
			requests++;
			if (max_requests && (requests == max_requests)) {
				fcgi_finish_request(request, 1);
				if (bindpath) {
					free(bindpath);
				}
				if (max_requests != 1) {
					/* no need to return exit_status of the last request */
					exit_status = 0;
				}
				break;
			}
			/* end of fastcgi loop */
		}
		if (request) {
			fcgi_destroy_request(request);
		}
		fcgi_shutdown();

		if (cgi_sapi_module.php_ini_path_override) {
			free(cgi_sapi_module.php_ini_path_override);
		}
		if (cgi_sapi_module.ini_entries) {
			free(cgi_sapi_module.ini_entries);
		}
	} zend_catch {
		exit_status = 255;
	} zend_end_try();

out:
	if (benchmark) {
		int sec;
#ifdef HAVE_GETTIMEOFDAY
		int usec;

		gettimeofday(&end, NULL);
		sec = (int)(end.tv_sec - start.tv_sec);
		if (end.tv_usec >= start.tv_usec) {
			usec = (int)(end.tv_usec - start.tv_usec);
		} else {
			sec -= 1;
			usec = (int)(end.tv_usec + 1000000 - start.tv_usec);
		}
		fprintf(stderr, "\nElapsed time: %d.%06d sec\n", sec, usec);
#else
		time(&end);
		sec = (int)(end - start);
		fprintf(stderr, "\nElapsed time: %d sec\n", sec);
#endif
	}

#ifndef PHP_WIN32
parent_out:
#endif

	SG(server_context) = NULL;
	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();

#ifdef ZTS
	tsrm_shutdown();
#endif

#if defined(PHP_WIN32) && ZEND_DEBUG && 0
	_CrtDumpMemoryLeaks();
#endif

	return exit_status;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */