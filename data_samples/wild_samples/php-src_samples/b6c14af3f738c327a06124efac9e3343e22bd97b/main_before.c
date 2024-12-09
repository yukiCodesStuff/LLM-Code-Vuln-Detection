{
	int retval = SUCCESS;

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_STARTUP(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */

#ifdef PHP_WIN32
	PG(com_initialized) = 0;
#endif

#if PHP_SIGCHILD
	signal(SIGCHLD, sigchld_handler);
#endif

	zend_try {
		PG(in_error_log) = 0;
		PG(during_request_startup) = 1;

		php_output_activate(TSRMLS_C);

		/* initialize global variables */
		PG(modules_activated) = 0;
		PG(header_is_being_sent) = 0;
		PG(connection_status) = PHP_CONNECTION_NORMAL;
		PG(in_user_include) = 0;

		zend_activate(TSRMLS_C);
		sapi_activate(TSRMLS_C);

#ifdef ZEND_SIGNALS
		zend_signal_activate(TSRMLS_C);
#endif

		if (PG(max_input_time) == -1) {
			zend_set_timeout(EG(timeout_seconds), 1);
		} else {
			zend_set_timeout(PG(max_input_time), 1);
		}

		/* Disable realpath cache if an open_basedir is set */
		if (PG(open_basedir) && *PG(open_basedir)) {
			CWDG(realpath_cache_size_limit) = 0;
		}

		if (PG(expose_php)) {
			sapi_add_header(SAPI_PHP_VERSION_HEADER, sizeof(SAPI_PHP_VERSION_HEADER)-1, 1);
		}

		if (PG(output_handler) && PG(output_handler)[0]) {
			zval *oh;

			MAKE_STD_ZVAL(oh);
			ZVAL_STRING(oh, PG(output_handler), 1);
			php_output_start_user(oh, 0, PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC);
			zval_ptr_dtor(&oh);
		} else if (PG(output_buffering)) {
			php_output_start_user(NULL, PG(output_buffering) > 1 ? PG(output_buffering) : 0, PHP_OUTPUT_HANDLER_STDFLAGS TSRMLS_CC);
		} else if (PG(implicit_flush)) {
			php_output_set_implicit_flush(1 TSRMLS_CC);
		}

		/* We turn this off in php_execute_script() */
		/* PG(during_request_startup) = 0; */

		php_hash_environment(TSRMLS_C);
		zend_activate_modules(TSRMLS_C);
		PG(modules_activated)=1;
	} zend_catch {
		retval = FAILURE;
	} zend_end_try();

	SG(sapi_started) = 1;

	return retval;
}
# else
int php_request_startup(TSRMLS_D)
{
	int retval = SUCCESS;

#if PHP_SIGCHILD
	signal(SIGCHLD, sigchld_handler);
#endif

	if (php_start_sapi() == FAILURE) {
		return FAILURE;
	}

	php_output_activate(TSRMLS_C);
	sapi_activate(TSRMLS_C);
	php_hash_environment(TSRMLS_C);

	zend_try {
		PG(during_request_startup) = 1;
		if (PG(expose_php)) {
			sapi_add_header(SAPI_PHP_VERSION_HEADER, sizeof(SAPI_PHP_VERSION_HEADER)-1, 1);
		}
	} zend_catch {
		retval = FAILURE;
	} zend_end_try();

	return retval;
}
# endif
/* }}} */

/* {{{ php_request_startup_for_hook
 */
int php_request_startup_for_hook(TSRMLS_D)
{
	int retval = SUCCESS;

#if PHP_SIGCHLD
	signal(SIGCHLD, sigchld_handler);
#endif

	if (php_start_sapi(TSRMLS_C) == FAILURE) {
		return FAILURE;
	}

	php_output_activate(TSRMLS_C);
	sapi_activate_headers_only(TSRMLS_C);
	php_hash_environment(TSRMLS_C);

	return retval;
}
/* }}} */

/* {{{ php_request_shutdown_for_exec
 */
void php_request_shutdown_for_exec(void *dummy)
{
	TSRMLS_FETCH();

	/* used to close fd's in the 3..255 range here, but it's problematic
	 */
	shutdown_memory_manager(1, 1 TSRMLS_CC);
	zend_interned_strings_restore(TSRMLS_C);
}
/* }}} */

/* {{{ php_request_shutdown_for_hook
 */
void php_request_shutdown_for_hook(void *dummy)
{
	TSRMLS_FETCH();

	if (PG(modules_activated)) zend_try {
		php_call_shutdown_functions(TSRMLS_C);
	} zend_end_try();

	if (PG(modules_activated)) {
		zend_deactivate_modules(TSRMLS_C);
		php_free_shutdown_functions(TSRMLS_C);
	}

	zend_try {
		zend_unset_timeout(TSRMLS_C);
	} zend_end_try();

	zend_try {
		int i;

		for (i = 0; i < NUM_TRACK_VARS; i++) {
			if (PG(http_globals)[i]) {
				zval_ptr_dtor(&PG(http_globals)[i]);
			}
		}
	} zend_end_try();

	zend_deactivate(TSRMLS_C);

	zend_try {
		sapi_deactivate(TSRMLS_C);
	} zend_end_try();

	zend_try {
		php_shutdown_stream_hashes(TSRMLS_C);
	} zend_end_try();

	zend_try {
		shutdown_memory_manager(CG(unclean_shutdown), 0 TSRMLS_CC);
	} zend_end_try();

	zend_interned_strings_restore(TSRMLS_C);

#ifdef ZEND_SIGNALS
	zend_try {
		zend_signal_deactivate(TSRMLS_C);
	} zend_end_try();
#endif
}

/* }}} */

/* {{{ php_request_shutdown
 */
void php_request_shutdown(void *dummy)
{
	zend_bool report_memleaks;
	TSRMLS_FETCH();

	report_memleaks = PG(report_memleaks);

	/* EG(opline_ptr) points into nirvana and therefore cannot be safely accessed
	 * inside zend_executor callback functions.
	 */
	EG(opline_ptr) = NULL;
	EG(active_op_array) = NULL;

	php_deactivate_ticks(TSRMLS_C);

	/* 1. Call all possible shutdown functions registered with register_shutdown_function() */
	if (PG(modules_activated)) zend_try {
		php_call_shutdown_functions(TSRMLS_C);
	} zend_end_try();

	/* 2. Call all possible __destruct() functions */
	zend_try {
		zend_call_destructors(TSRMLS_C);
	} zend_end_try();

	/* 3. Flush all output buffers */
	zend_try {
		zend_bool send_buffer = SG(request_info).headers_only ? 0 : 1;

		if (CG(unclean_shutdown) && PG(last_error_type) == E_ERROR &&
			(size_t)PG(memory_limit) < zend_memory_usage(1 TSRMLS_CC)
		) {
			send_buffer = 0;
		}

		if (!send_buffer) {
			php_output_discard_all(TSRMLS_C);
		} else {
			php_output_end_all(TSRMLS_C);
		}
	} zend_end_try();

	/* 4. Reset max_execution_time (no longer executing php code after response sent) */
	zend_try {
		zend_unset_timeout(TSRMLS_C);
	} zend_end_try();

	/* 5. Call all extensions RSHUTDOWN functions */
	if (PG(modules_activated)) {
		zend_deactivate_modules(TSRMLS_C);
		php_free_shutdown_functions(TSRMLS_C);
	}

	/* 6. Shutdown output layer (send the set HTTP headers, cleanup output handlers, etc.) */
	zend_try {
		php_output_deactivate(TSRMLS_C);
	} zend_end_try();

	/* 7. Destroy super-globals */
	zend_try {
		int i;

		for (i=0; i<NUM_TRACK_VARS; i++) {
			if (PG(http_globals)[i]) {
				zval_ptr_dtor(&PG(http_globals)[i]);
			}
		}
	} zend_end_try();

	/* 7.5 free last error information */
	if (PG(last_error_message)) {
		free(PG(last_error_message));
		PG(last_error_message) = NULL;
	}
	if (PG(last_error_file)) {
		free(PG(last_error_file));
		PG(last_error_file) = NULL;
	}

	/* 7. Shutdown scanner/executor/compiler and restore ini entries */
	zend_deactivate(TSRMLS_C);

	/* 8. Call all extensions post-RSHUTDOWN functions */
	zend_try {
		zend_post_deactivate_modules(TSRMLS_C);
	} zend_end_try();

	/* 9. SAPI related shutdown (free stuff) */
	zend_try {
		sapi_deactivate(TSRMLS_C);
	} zend_end_try();

	/* 10. Destroy stream hashes */
	zend_try {
		php_shutdown_stream_hashes(TSRMLS_C);
	} zend_end_try();

	/* 11. Free Willy (here be crashes) */
	zend_try {
		shutdown_memory_manager(CG(unclean_shutdown) || !report_memleaks, 0 TSRMLS_CC);
	} zend_end_try();
	zend_interned_strings_restore(TSRMLS_C);

	/* 12. Reset max_execution_time */
	zend_try {
		zend_unset_timeout(TSRMLS_C);
	} zend_end_try();

#ifdef PHP_WIN32
	if (PG(com_initialized)) {
		CoUninitialize();
		PG(com_initialized) = 0;
	}
#endif

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_SHUTDOWN(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */
}
/* }}} */

/* {{{ php_com_initialize
 */
PHPAPI void php_com_initialize(TSRMLS_D)
{
#ifdef PHP_WIN32
	if (!PG(com_initialized)) {
		if (CoInitialize(NULL) == S_OK) {
			PG(com_initialized) = 1;
		}
	}
#endif
}
/* }}} */

/* {{{ php_output_wrapper
 */
static int php_output_wrapper(const char *str, uint str_length)
{
	TSRMLS_FETCH();
	return php_output_write(str, str_length TSRMLS_CC);
}
/* }}} */

#ifdef ZTS
/* {{{ core_globals_ctor
 */
static void core_globals_ctor(php_core_globals *core_globals TSRMLS_DC)
{
	memset(core_globals, 0, sizeof(*core_globals));

	php_startup_ticks(TSRMLS_C);
}
/* }}} */
#endif

/* {{{ core_globals_dtor
 */
static void core_globals_dtor(php_core_globals *core_globals TSRMLS_DC)
{
	if (core_globals->last_error_message) {
		free(core_globals->last_error_message);
	}
	if (core_globals->last_error_file) {
		free(core_globals->last_error_file);
	}
	if (core_globals->disable_functions) {
		free(core_globals->disable_functions);
	}
	if (core_globals->disable_classes) {
		free(core_globals->disable_classes);
	}
	if (core_globals->php_binary) {
		free(core_globals->php_binary);
	}

	php_shutdown_ticks(TSRMLS_C);
}
/* }}} */

PHP_MINFO_FUNCTION(php_core) { /* {{{ */
	php_info_print_table_start();
	php_info_print_table_row(2, "PHP Version", PHP_VERSION);
	php_info_print_table_end();
	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ php_register_extensions
 */
int php_register_extensions(zend_module_entry **ptr, int count TSRMLS_DC)
{
	zend_module_entry **end = ptr + count;

	while (ptr < end) {
		if (*ptr) {
			if (zend_register_internal_module(*ptr TSRMLS_CC)==NULL) {
				return FAILURE;
			}
		}
		ptr++;
	}
	return SUCCESS;
}
/* }}} */

#if defined(PHP_WIN32) && _MSC_VER >= 1400
static _invalid_parameter_handler old_invalid_parameter_handler;

void dummy_invalid_parameter_handler(
		const wchar_t *expression,
		const wchar_t *function,
		const wchar_t *file,
		unsigned int   line,
		uintptr_t      pEwserved)
{
	static int called = 0;
	char buf[1024];
	int len;

	if (!called) {
		TSRMLS_FETCH();
		if(PG(windows_show_crt_warning)) {
			called = 1;
			if (function) {
				if (file) {
					len = _snprintf(buf, sizeof(buf)-1, "Invalid parameter detected in CRT function '%ws' (%ws:%d)", function, file, line);
				} else {
					len = _snprintf(buf, sizeof(buf)-1, "Invalid parameter detected in CRT function '%ws'", function);
				}
			} else {
				len = _snprintf(buf, sizeof(buf)-1, "Invalid CRT parameter detected (function not known)");
			}
			zend_error(E_WARNING, "%s", buf);
			called = 0;
		}
	}
}
#endif

/* {{{ php_module_startup
 */
int php_module_startup(sapi_module_struct *sf, zend_module_entry *additional_modules, uint num_additional_modules)
{
	zend_utility_functions zuf;
	zend_utility_values zuv;
	int retval = SUCCESS, module_number=0;	/* for REGISTER_INI_ENTRIES() */
	char *php_os;
	zend_module_entry *module;
#ifdef ZTS
	zend_executor_globals *executor_globals;
	void ***tsrm_ls;
	php_core_globals *core_globals;
#endif

#if defined(PHP_WIN32) || (defined(NETWARE) && defined(USE_WINSOCK))
	WORD wVersionRequested = MAKEWORD(2, 0);
	WSADATA wsaData;
#endif
#ifdef PHP_WIN32
	php_os = "WINNT";
#if _MSC_VER >= 1400
	old_invalid_parameter_handler =
		_set_invalid_parameter_handler(dummy_invalid_parameter_handler);
	if (old_invalid_parameter_handler != NULL) {
		_set_invalid_parameter_handler(old_invalid_parameter_handler);
	}

	/* Disable the message box for assertions.*/
	_CrtSetReportMode(_CRT_ASSERT, 0);
#endif
#else
	php_os=PHP_OS;
#endif

#ifdef ZTS
	tsrm_ls = ts_resource(0);
#endif

#ifdef PHP_WIN32
	php_win32_init_rng_lock();
#endif

	module_shutdown = 0;
	module_startup = 1;
	sapi_initialize_empty_request(TSRMLS_C);
	sapi_activate(TSRMLS_C);

	if (module_initialized) {
		return SUCCESS;
	}

	sapi_module = *sf;

	php_output_startup();

	zuf.error_function = php_error_cb;
	zuf.printf_function = php_printf;
	zuf.write_function = php_output_wrapper;
	zuf.fopen_function = php_fopen_wrapper_for_zend;
	zuf.message_handler = php_message_handler_for_zend;
	zuf.block_interruptions = sapi_module.block_interruptions;
	zuf.unblock_interruptions = sapi_module.unblock_interruptions;
	zuf.get_configuration_directive = php_get_configuration_directive_for_zend;
	zuf.ticks_function = php_run_ticks;
	zuf.on_timeout = php_on_timeout;
	zuf.stream_open_function = php_stream_open_for_zend;
	zuf.vspprintf_function = vspprintf;
	zuf.getenv_function = sapi_getenv;
	zuf.resolve_path_function = php_resolve_path_for_zend;
	zend_startup(&zuf, NULL TSRMLS_CC);

#ifdef ZTS
	executor_globals = ts_resource(executor_globals_id);
	ts_allocate_id(&core_globals_id, sizeof(php_core_globals), (ts_allocate_ctor) core_globals_ctor, (ts_allocate_dtor) core_globals_dtor);
	core_globals = ts_resource(core_globals_id);
#ifdef PHP_WIN32
	ts_allocate_id(&php_win32_core_globals_id, sizeof(php_win32_core_globals), (ts_allocate_ctor) php_win32_core_globals_ctor, (ts_allocate_dtor) php_win32_core_globals_dtor);
#endif
#else
	php_startup_ticks(TSRMLS_C);
#endif
	gc_globals_ctor(TSRMLS_C);

#ifdef PHP_WIN32
	{
		OSVERSIONINFOEX *osvi = &EG(windows_version_info);

		ZeroMemory(osvi, sizeof(OSVERSIONINFOEX));
		osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		if( !GetVersionEx((OSVERSIONINFO *) osvi)) {
			php_printf("\nGetVersionEx unusable. %d\n", GetLastError());
			return FAILURE;
		}
	}
#endif
	EG(bailout) = NULL;
	EG(error_reporting) = E_ALL & ~E_NOTICE;
	EG(active_symbol_table) = NULL;
	PG(header_is_being_sent) = 0;
	SG(request_info).headers_only = 0;
	SG(request_info).argv0 = NULL;
	SG(request_info).argc=0;
	SG(request_info).argv=(char **)NULL;
	PG(connection_status) = PHP_CONNECTION_NORMAL;
	PG(during_request_startup) = 0;
	PG(last_error_message) = NULL;
	PG(last_error_file) = NULL;
	PG(last_error_lineno) = 0;
	EG(error_handling)  = EH_NORMAL;
	EG(exception_class) = NULL;
	PG(disable_functions) = NULL;
	PG(disable_classes) = NULL;
	EG(exception) = NULL;
	EG(objects_store).object_buckets = NULL;

#if HAVE_SETLOCALE
	setlocale(LC_CTYPE, "");
	zend_update_current_locale();
#endif

#if HAVE_TZSET
	tzset();
#endif

#if defined(PHP_WIN32) || (defined(NETWARE) && defined(USE_WINSOCK))
	/* start up winsock services */
	if (WSAStartup(wVersionRequested, &wsaData) != 0) {
		php_printf("\nwinsock.dll unusable. %d\n", WSAGetLastError());
		return FAILURE;
	}
#endif

	le_index_ptr = zend_register_list_destructors_ex(NULL, NULL, "index pointer", 0);

	/* Register constants */
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_VERSION", PHP_VERSION, sizeof(PHP_VERSION)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_MAJOR_VERSION", PHP_MAJOR_VERSION, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_MINOR_VERSION", PHP_MINOR_VERSION, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_RELEASE_VERSION", PHP_RELEASE_VERSION, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_EXTRA_VERSION", PHP_EXTRA_VERSION, sizeof(PHP_EXTRA_VERSION) - 1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_VERSION_ID", PHP_VERSION_ID, CONST_PERSISTENT | CONST_CS);
#ifdef ZTS
	REGISTER_MAIN_LONG_CONSTANT("PHP_ZTS", 1, CONST_PERSISTENT | CONST_CS);
#else
	REGISTER_MAIN_LONG_CONSTANT("PHP_ZTS", 0, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_MAIN_LONG_CONSTANT("PHP_DEBUG", PHP_DEBUG, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_OS", php_os, strlen(php_os), CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_SAPI", sapi_module.name, strlen(sapi_module.name), CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("DEFAULT_INCLUDE_PATH", PHP_INCLUDE_PATH, sizeof(PHP_INCLUDE_PATH)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PEAR_INSTALL_DIR", PEAR_INSTALLDIR, sizeof(PEAR_INSTALLDIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PEAR_EXTENSION_DIR", PHP_EXTENSION_DIR, sizeof(PHP_EXTENSION_DIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_EXTENSION_DIR", PHP_EXTENSION_DIR, sizeof(PHP_EXTENSION_DIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_PREFIX", PHP_PREFIX, sizeof(PHP_PREFIX)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_BINDIR", PHP_BINDIR, sizeof(PHP_BINDIR)-1, CONST_PERSISTENT | CONST_CS);
#ifndef PHP_WIN32
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_MANDIR", PHP_MANDIR, sizeof(PHP_MANDIR)-1, CONST_PERSISTENT | CONST_CS);
#endif
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_LIBDIR", PHP_LIBDIR, sizeof(PHP_LIBDIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_DATADIR", PHP_DATADIR, sizeof(PHP_DATADIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_SYSCONFDIR", PHP_SYSCONFDIR, sizeof(PHP_SYSCONFDIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_LOCALSTATEDIR", PHP_LOCALSTATEDIR, sizeof(PHP_LOCALSTATEDIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_CONFIG_FILE_PATH", PHP_CONFIG_FILE_PATH, strlen(PHP_CONFIG_FILE_PATH), CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_CONFIG_FILE_SCAN_DIR", PHP_CONFIG_FILE_SCAN_DIR, sizeof(PHP_CONFIG_FILE_SCAN_DIR)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_SHLIB_SUFFIX", PHP_SHLIB_SUFFIX, sizeof(PHP_SHLIB_SUFFIX)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_STRINGL_CONSTANT("PHP_EOL", PHP_EOL, sizeof(PHP_EOL)-1, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_MAXPATHLEN", MAXPATHLEN, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_INT_MAX", LONG_MAX, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_INT_SIZE", sizeof(long), CONST_PERSISTENT | CONST_CS);

#ifdef PHP_WIN32
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_MAJOR",      EG(windows_version_info).dwMajorVersion, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_MINOR",      EG(windows_version_info).dwMinorVersion, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_BUILD",      EG(windows_version_info).dwBuildNumber, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_PLATFORM",   EG(windows_version_info).dwPlatformId, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_SP_MAJOR",   EG(windows_version_info).wServicePackMajor, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_SP_MINOR",   EG(windows_version_info).wServicePackMinor, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_SUITEMASK",  EG(windows_version_info).wSuiteMask, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_VERSION_PRODUCTTYPE", EG(windows_version_info).wProductType, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_NT_DOMAIN_CONTROLLER", VER_NT_DOMAIN_CONTROLLER, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_NT_SERVER", VER_NT_SERVER, CONST_PERSISTENT | CONST_CS);
	REGISTER_MAIN_LONG_CONSTANT("PHP_WINDOWS_NT_WORKSTATION", VER_NT_WORKSTATION, CONST_PERSISTENT | CONST_CS);
#endif

	php_binary_init(TSRMLS_C);
	if (PG(php_binary)) {
		REGISTER_MAIN_STRINGL_CONSTANT("PHP_BINARY", PG(php_binary), strlen(PG(php_binary)), CONST_PERSISTENT | CONST_CS);
	} else {
		REGISTER_MAIN_STRINGL_CONSTANT("PHP_BINARY", "", 0, CONST_PERSISTENT | CONST_CS);
	}

	php_output_register_constants(TSRMLS_C);
	php_rfc1867_register_constants(TSRMLS_C);

	/* this will read in php.ini, set up the configuration parameters,
	   load zend extensions and register php function extensions
	   to be loaded later */
	if (php_init_config(TSRMLS_C) == FAILURE) {
		return FAILURE;
	}

	/* Register PHP core ini entries */
	REGISTER_INI_ENTRIES();

	/* Register Zend ini entries */
	zend_register_standard_ini_entries(TSRMLS_C);

	/* Disable realpath cache if an open_basedir is set */
	if (PG(open_basedir) && *PG(open_basedir)) {
		CWDG(realpath_cache_size_limit) = 0;
	}

	/* initialize stream wrappers registry
	 * (this uses configuration parameters from php.ini)
	 */
	if (php_init_stream_wrappers(module_number TSRMLS_CC) == FAILURE)	{
		php_printf("PHP:  Unable to initialize stream url wrappers.\n");
		return FAILURE;
	}

	zuv.html_errors = 1;
	zuv.import_use_extension = ".php";
	php_startup_auto_globals(TSRMLS_C);
	zend_set_utility_values(&zuv);
	php_startup_sapi_content_types(TSRMLS_C);

	/* startup extensions statically compiled in */
	if (php_register_internal_extensions_func(TSRMLS_C) == FAILURE) {
		php_printf("Unable to start builtin modules\n");
		return FAILURE;
	}

	/* start additional PHP extensions */
	php_register_extensions(&additional_modules, num_additional_modules TSRMLS_CC);

	/* load and startup extensions compiled as shared objects (aka DLLs)
	   as requested by php.ini entries
	   theese are loaded after initialization of internal extensions
	   as extensions *might* rely on things from ext/standard
	   which is always an internal extension and to be initialized
	   ahead of all other internals
	 */
	php_ini_register_extensions(TSRMLS_C);
	zend_startup_modules(TSRMLS_C);

	/* start Zend extensions */
	zend_startup_extensions();

	zend_collect_module_handlers(TSRMLS_C);

	/* register additional functions */
	if (sapi_module.additional_functions) {
		if (zend_hash_find(&module_registry, "standard", sizeof("standard"), (void**)&module)==SUCCESS) {
			EG(current_module) = module;
			zend_register_functions(NULL, sapi_module.additional_functions, NULL, MODULE_PERSISTENT TSRMLS_CC);
			EG(current_module) = NULL;
		}
	}

	/* disable certain classes and functions as requested by php.ini */
	php_disable_functions(TSRMLS_C);
	php_disable_classes(TSRMLS_C);

	/* make core report what it should */
	if (zend_hash_find(&module_registry, "core", sizeof("core"), (void**)&module)==SUCCESS) {
		module->version = PHP_VERSION;
		module->info_func = PHP_MINFO(php_core);
	}


#ifdef PHP_WIN32
	/* Disable incompatible functions for the running platform */
	if (php_win32_disable_functions(TSRMLS_C) == FAILURE) {
		php_printf("Unable to disable unsupported functions\n");
		return FAILURE;
	}
#endif

#ifdef ZTS
	zend_post_startup(TSRMLS_C);
#endif

	module_initialized = 1;

	/* Check for deprecated directives */
	/* NOTE: If you add anything here, remember to add it to Makefile.global! */
	{
		struct {
			const long error_level;
			const char *phrase;
			const char *directives[16]; /* Remember to change this if the number of directives change */
		} directives[2] = {
			{
				E_DEPRECATED,
				"Directive '%s' is deprecated in PHP 5.3 and greater",
				{
					NULL
				}
			},
			{
				E_CORE_ERROR,
				"Directive '%s' is no longer available in PHP",
				{
					"allow_call_time_pass_reference",
					"define_syslog_variables",
					"highlight.bg",
					"magic_quotes_gpc",
					"magic_quotes_runtime",
					"magic_quotes_sybase",
					"register_globals",
					"register_long_arrays",
					"safe_mode",
					"safe_mode_gid",
					"safe_mode_include_dir",
					"safe_mode_exec_dir",
					"safe_mode_allowed_env_vars",
					"safe_mode_protected_env_vars",
					"zend.ze1_compatibility_mode",
					NULL
				}
			}
		};

		unsigned int i;

		zend_try {
			/* 2 = Count of deprecation structs */
			for (i = 0; i < 2; i++) {
				const char **p = directives[i].directives;

				while(*p) {
					long value;

					if (cfg_get_long((char*)*p, &value) == SUCCESS && value) {
						zend_error(directives[i].error_level, directives[i].phrase, *p);
					}

					++p;
				}
			}
		} zend_catch {
			retval = FAILURE;
		} zend_end_try();
	}

	sapi_deactivate(TSRMLS_C);
	module_startup = 0;

	shutdown_memory_manager(1, 0 TSRMLS_CC);
	zend_interned_strings_snapshot(TSRMLS_C);

	/* we're done */
	return retval;
}
/* }}} */

void php_module_shutdown_for_exec(void)
{
	/* used to close fd's in the range 3.255 here, but it's problematic */
}

/* {{{ php_module_shutdown_wrapper
 */
int php_module_shutdown_wrapper(sapi_module_struct *sapi_globals)
{
	TSRMLS_FETCH();
	php_module_shutdown(TSRMLS_C);
	return SUCCESS;
}
/* }}} */

/* {{{ php_module_shutdown
 */
void php_module_shutdown(TSRMLS_D)
{
	int module_number=0;	/* for UNREGISTER_INI_ENTRIES() */

	module_shutdown = 1;

	if (!module_initialized) {
		return;
	}

#ifdef ZTS
	ts_free_worker_threads();
#endif

#if defined(PHP_WIN32) || (defined(NETWARE) && defined(USE_WINSOCK))
	/*close winsock */
	WSACleanup();
#endif

#ifdef PHP_WIN32
	php_win32_free_rng_lock();
#endif

	sapi_flush(TSRMLS_C);

	zend_shutdown(TSRMLS_C);

	/* Destroys filter & transport registries too */
	php_shutdown_stream_wrappers(module_number TSRMLS_CC);

	UNREGISTER_INI_ENTRIES();

	/* close down the ini config */
	php_shutdown_config();

#ifndef ZTS
	zend_ini_shutdown(TSRMLS_C);
	shutdown_memory_manager(CG(unclean_shutdown), 1 TSRMLS_CC);
#else
	zend_ini_global_shutdown(TSRMLS_C);
#endif

	php_output_shutdown();
	php_shutdown_temporary_directory();

	module_initialized = 0;

#ifndef ZTS
	core_globals_dtor(&core_globals TSRMLS_CC);
	gc_globals_dtor(TSRMLS_C);
#else
	ts_free_id(core_globals_id);
#endif

#if defined(PHP_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1400)
	if (old_invalid_parameter_handler == NULL) {
		_set_invalid_parameter_handler(old_invalid_parameter_handler);
	}
#endif
}
/* }}} */

/* {{{ php_execute_script
 */
PHPAPI int php_execute_script(zend_file_handle *primary_file TSRMLS_DC)
{
	zend_file_handle *prepend_file_p, *append_file_p;
	zend_file_handle prepend_file = {0}, append_file = {0};
#if HAVE_BROKEN_GETCWD 
	volatile int old_cwd_fd = -1;
#else
	char *old_cwd;
	ALLOCA_FLAG(use_heap)
#endif
	int retval = 0;

	EG(exit_status) = 0;
#ifndef HAVE_BROKEN_GETCWD
# define OLD_CWD_SIZE 4096
	old_cwd = do_alloca(OLD_CWD_SIZE, use_heap);
	old_cwd[0] = '\0';
#endif

	zend_try {
		char realfile[MAXPATHLEN];

#ifdef PHP_WIN32
		if(primary_file->filename) {
			UpdateIniFromRegistry(primary_file->filename TSRMLS_CC);
		}
#endif

		PG(during_request_startup) = 0;

		if (primary_file->filename && !(SG(options) & SAPI_OPTION_NO_CHDIR)) {
#if HAVE_BROKEN_GETCWD
			/* this looks nasty to me */
			old_cwd_fd = open(".", 0);
#else
			php_ignore_value(VCWD_GETCWD(old_cwd, OLD_CWD_SIZE-1));
#endif
			VCWD_CHDIR_FILE(primary_file->filename);
		}

 		/* Only lookup the real file path and add it to the included_files list if already opened
		 *   otherwise it will get opened and added to the included_files list in zend_execute_scripts
		 */
 		if (primary_file->filename &&
 		    (primary_file->filename[0] != '-' || primary_file->filename[1] != 0) &&
 			primary_file->opened_path == NULL &&
 			primary_file->type != ZEND_HANDLE_FILENAME
		) {
			int realfile_len;
			int dummy = 1;

			if (expand_filepath(primary_file->filename, realfile TSRMLS_CC)) {
				realfile_len =  strlen(realfile);
				zend_hash_add(&EG(included_files), realfile, realfile_len+1, (void *)&dummy, sizeof(int), NULL);
				primary_file->opened_path = estrndup(realfile, realfile_len);
			}
		}

		if (PG(auto_prepend_file) && PG(auto_prepend_file)[0]) {
			prepend_file.filename = PG(auto_prepend_file);
			prepend_file.opened_path = NULL;
			prepend_file.free_filename = 0;
			prepend_file.type = ZEND_HANDLE_FILENAME;
			prepend_file_p = &prepend_file;
		} else {
			prepend_file_p = NULL;
		}

		if (PG(auto_append_file) && PG(auto_append_file)[0]) {
			append_file.filename = PG(auto_append_file);
			append_file.opened_path = NULL;
			append_file.free_filename = 0;
			append_file.type = ZEND_HANDLE_FILENAME;
			append_file_p = &append_file;
		} else {
			append_file_p = NULL;
		}
		if (PG(max_input_time) != -1) {
#ifdef PHP_WIN32
			zend_unset_timeout(TSRMLS_C);
#endif
			zend_set_timeout(INI_INT("max_execution_time"), 0);
		}
		retval = (zend_execute_scripts(ZEND_REQUIRE TSRMLS_CC, NULL, 3, prepend_file_p, primary_file, append_file_p) == SUCCESS);

	} zend_end_try();

#if HAVE_BROKEN_GETCWD
	if (old_cwd_fd != -1) {
		fchdir(old_cwd_fd);
		close(old_cwd_fd);
	}
#else
	if (old_cwd[0] != '\0') {
		php_ignore_value(VCWD_CHDIR(old_cwd));
	}
	free_alloca(old_cwd, use_heap);
#endif
	return retval;
}
/* }}} */

/* {{{ php_execute_simple_script
 */
PHPAPI int php_execute_simple_script(zend_file_handle *primary_file, zval **ret TSRMLS_DC)
{
	char *old_cwd;
	ALLOCA_FLAG(use_heap)

	EG(exit_status) = 0;
#define OLD_CWD_SIZE 4096
	old_cwd = do_alloca(OLD_CWD_SIZE, use_heap);
	old_cwd[0] = '\0';

	zend_try {
#ifdef PHP_WIN32
		if(primary_file->filename) {
			UpdateIniFromRegistry(primary_file->filename TSRMLS_CC);
		}
#endif

		PG(during_request_startup) = 0;

		if (primary_file->filename && !(SG(options) & SAPI_OPTION_NO_CHDIR)) {
			php_ignore_value(VCWD_GETCWD(old_cwd, OLD_CWD_SIZE-1));
			VCWD_CHDIR_FILE(primary_file->filename);
		}
		zend_execute_scripts(ZEND_REQUIRE TSRMLS_CC, ret, 1, primary_file);
	} zend_end_try();

	if (old_cwd[0] != '\0') {
		php_ignore_value(VCWD_CHDIR(old_cwd));
	}

	free_alloca(old_cwd, use_heap);
	return EG(exit_status);
}
/* }}} */

/* {{{ php_handle_aborted_connection
 */
PHPAPI void php_handle_aborted_connection(void)
{
	TSRMLS_FETCH();

	PG(connection_status) = PHP_CONNECTION_ABORTED;
	php_output_set_status(PHP_OUTPUT_DISABLED TSRMLS_CC);

	if (!PG(ignore_user_abort)) {
		zend_bailout();
	}
}
/* }}} */

/* {{{ php_handle_auth_data
 */
PHPAPI int php_handle_auth_data(const char *auth TSRMLS_DC)
{
	int ret = -1;

	if (auth && auth[0] != '\0' && strncmp(auth, "Basic ", 6) == 0) {
		char *pass;
		char *user;

		user = php_base64_decode(auth + 6, strlen(auth) - 6, NULL);
		if (user) {
			pass = strchr(user, ':');
			if (pass) {
				*pass++ = '\0';
				SG(request_info).auth_user = user;
				SG(request_info).auth_password = estrdup(pass);
				ret = 0;
			} else {
				efree(user);
			}
		}
	}

	if (ret == -1) {
		SG(request_info).auth_user = SG(request_info).auth_password = NULL;
	} else {
		SG(request_info).auth_digest = NULL;
	}

	if (ret == -1 && auth && auth[0] != '\0' && strncmp(auth, "Digest ", 7) == 0) {
		SG(request_info).auth_digest = estrdup(auth + 7);
		ret = 0;
	}

	if (ret == -1) {
		SG(request_info).auth_digest = NULL;
	}

	return ret;
}
/* }}} */

/* {{{ php_lint_script
 */
PHPAPI int php_lint_script(zend_file_handle *file TSRMLS_DC)
{
	zend_op_array *op_array;
	int retval = FAILURE;

	zend_try {
		op_array = zend_compile_file(file, ZEND_INCLUDE TSRMLS_CC);
		zend_destroy_file_handle(file TSRMLS_CC);

		if (op_array) {
			destroy_op_array(op_array TSRMLS_CC);
			efree(op_array);
			retval = SUCCESS;
		}
	} zend_end_try();

	return retval;
}
/* }}} */

#ifdef PHP_WIN32
/* {{{ dummy_indent
   just so that this symbol gets exported... */
PHPAPI void dummy_indent(void)
{
	zend_indent();
}
/* }}} */
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
	if (PG(com_initialized)) {
		CoUninitialize();
		PG(com_initialized) = 0;
	}
#endif

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_SHUTDOWN(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */
}
/* }}} */

/* {{{ php_com_initialize
 */
PHPAPI void php_com_initialize(TSRMLS_D)
{
#ifdef PHP_WIN32
	if (!PG(com_initialized)) {
		if (CoInitialize(NULL) == S_OK) {
			PG(com_initialized) = 1;
		}
	}