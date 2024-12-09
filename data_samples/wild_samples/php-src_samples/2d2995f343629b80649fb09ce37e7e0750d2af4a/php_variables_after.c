{
	char buf[128];
	char **env, *p, *t = buf;
	size_t alloc_size = sizeof(buf);
	unsigned long nlen; /* ptrdiff_t is not portable */

	/* turn off magic_quotes while importing environment variables */
	int magic_quotes_gpc = PG(magic_quotes_gpc);

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
	if (t != buf && t != NULL) {
		efree(t);
	}

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}

zend_bool php_std_auto_global_callback(char *name, uint name_len TSRMLS_DC)
{
	zend_printf("%s\n", name);
	return 0; /* don't rearm */
}

/* {{{ php_build_argv
 */
static void php_build_argv(char *s, zval *track_vars_array TSRMLS_DC)
{
	zval *arr, *argc, *tmp;
	int count = 0;
	char *ss, *space;
	
	if (!(PG(register_globals) || SG(request_info).argc || track_vars_array)) {
		return;
	}
	
	ALLOC_INIT_ZVAL(arr);
	array_init(arr);

	/* Prepare argv */
	if (SG(request_info).argc) { /* are we in cli sapi? */
		int i;
		for (i = 0; i < SG(request_info).argc; i++) {
			ALLOC_ZVAL(tmp);
			Z_TYPE_P(tmp) = IS_STRING;
			Z_STRLEN_P(tmp) = strlen(SG(request_info).argv[i]);
			Z_STRVAL_P(tmp) = estrndup(SG(request_info).argv[i], Z_STRLEN_P(tmp));
			INIT_PZVAL(tmp);
			if (zend_hash_next_index_insert(Z_ARRVAL_P(arr), &tmp, sizeof(zval *), NULL) == FAILURE) {
				if (Z_TYPE_P(tmp) == IS_STRING) {
					efree(Z_STRVAL_P(tmp));
				}
			}
	if (PG(http_globals)[TRACK_VARS_SERVER]) {
		zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_SERVER]);
	}
	PG(http_globals)[TRACK_VARS_SERVER] = array_ptr;
	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "0", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}

	/* Server variables */
	if (sapi_module.register_server_variables) {
		sapi_module.register_server_variables(array_ptr TSRMLS_CC);
	}

	/* PHP Authentication support */
	if (SG(request_info).auth_user) {
		php_register_variable("PHP_AUTH_USER", SG(request_info).auth_user, array_ptr TSRMLS_CC);
	}
	if (SG(request_info).auth_password) {
		php_register_variable("PHP_AUTH_PW", SG(request_info).auth_password, array_ptr TSRMLS_CC);
	}
	if (SG(request_info).auth_digest) {
		php_register_variable("PHP_AUTH_DIGEST", SG(request_info).auth_digest, array_ptr TSRMLS_CC);
	}
	/* store request init time */
	{
		zval new_entry;
		Z_TYPE(new_entry) = IS_LONG;
		Z_LVAL(new_entry) = sapi_get_request_time(TSRMLS_C);
		php_register_variable_ex("REQUEST_TIME", &new_entry, array_ptr TSRMLS_CC);
	}

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}
/* }}} */

/* {{{ php_autoglobal_merge
 */
static void php_autoglobal_merge(HashTable *dest, HashTable *src TSRMLS_DC)
{
	zval **src_entry, **dest_entry;
	char *string_key;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;
	int key_type;
	int globals_check = (PG(register_globals) && (dest == (&EG(symbol_table))));

	zend_hash_internal_pointer_reset_ex(src, &pos);
	while (zend_hash_get_current_data_ex(src, (void **)&src_entry, &pos) == SUCCESS) {
		key_type = zend_hash_get_current_key_ex(src, &string_key, &string_key_len, &num_key, 0, &pos);
		if (Z_TYPE_PP(src_entry) != IS_ARRAY
			|| (key_type == HASH_KEY_IS_STRING && zend_hash_find(dest, string_key, string_key_len, (void **) &dest_entry) != SUCCESS)
			|| (key_type == HASH_KEY_IS_LONG && zend_hash_index_find(dest, num_key, (void **)&dest_entry) != SUCCESS)
			|| Z_TYPE_PP(dest_entry) != IS_ARRAY
        ) {
			Z_ADDREF_PP(src_entry);
			if (key_type == HASH_KEY_IS_STRING) {
				/* if register_globals is on and working with main symbol table, prevent overwriting of GLOBALS */
				if (!globals_check || string_key_len != sizeof("GLOBALS") || memcmp(string_key, "GLOBALS", sizeof("GLOBALS") - 1)) {
					zend_hash_update(dest, string_key, string_key_len, src_entry, sizeof(zval *), NULL);
				} else {
					Z_DELREF_PP(src_entry);
				}
			} else {
				zend_hash_index_update(dest, num_key, src_entry, sizeof(zval *), NULL);
			}
		} else {
			SEPARATE_ZVAL(dest_entry);
			php_autoglobal_merge(Z_ARRVAL_PP(dest_entry), Z_ARRVAL_PP(src_entry) TSRMLS_CC);
		}
		zend_hash_move_forward_ex(src, &pos);
	}
}
/* }}} */

static zend_bool php_auto_globals_create_server(char *name, uint name_len TSRMLS_DC);
static zend_bool php_auto_globals_create_env(char *name, uint name_len TSRMLS_DC);
static zend_bool php_auto_globals_create_request(char *name, uint name_len TSRMLS_DC);

/* {{{ php_hash_environment
 */
int php_hash_environment(TSRMLS_D)
{
	char *p;
	unsigned char _gpc_flags[5] = {0, 0, 0, 0, 0};
	zend_bool jit_initialization = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
	struct auto_global_record {
		char *name;
		uint name_len;
		char *long_name;
		uint long_name_len;
		zend_bool jit_initialization;
	} auto_global_records[] = {
		{ "_POST", sizeof("_POST"), "HTTP_POST_VARS", sizeof("HTTP_POST_VARS"), 0 },
		{ "_GET", sizeof("_GET"), "HTTP_GET_VARS", sizeof("HTTP_GET_VARS"), 0 },
		{ "_COOKIE", sizeof("_COOKIE"), "HTTP_COOKIE_VARS", sizeof("HTTP_COOKIE_VARS"), 0 },
		{ "_SERVER", sizeof("_SERVER"), "HTTP_SERVER_VARS", sizeof("HTTP_SERVER_VARS"), 1 },
		{ "_ENV", sizeof("_ENV"), "HTTP_ENV_VARS", sizeof("HTTP_ENV_VARS"), 1 },
		{ "_FILES", sizeof("_FILES"), "HTTP_POST_FILES", sizeof("HTTP_POST_FILES"), 0 },
	};
	size_t num_track_vars = sizeof(auto_global_records)/sizeof(struct auto_global_record);
	size_t i;

	/* jit_initialization = 0; */
	for (i=0; i<num_track_vars; i++) {
		PG(http_globals)[i] = NULL;
	}

	for (p=PG(variables_order); p && *p; p++) {
		switch(*p) {
			case 'p':
			case 'P':
				if (!_gpc_flags[0] && !SG(headers_sent) && SG(request_info).request_method && !strcasecmp(SG(request_info).request_method, "POST")) {
					sapi_module.treat_data(PARSE_POST, NULL, NULL TSRMLS_CC);	/* POST Data */
					_gpc_flags[0] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]) TSRMLS_CC);
					}
				}
				break;
			case 'c':
			case 'C':
				if (!_gpc_flags[1]) {
					sapi_module.treat_data(PARSE_COOKIE, NULL, NULL TSRMLS_CC);	/* Cookie Data */
					_gpc_flags[1] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_COOKIE]) TSRMLS_CC);
					}
				}
				break;
			case 'g':
			case 'G':
				if (!_gpc_flags[2]) {
					sapi_module.treat_data(PARSE_GET, NULL, NULL TSRMLS_CC);	/* GET Data */
					_gpc_flags[2] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]) TSRMLS_CC);
					}
				}
				break;
			case 'e':
			case 'E':
				if (!jit_initialization && !_gpc_flags[3]) {
					zend_auto_global_disable_jit("_ENV", sizeof("_ENV")-1 TSRMLS_CC);
					php_auto_globals_create_env("_ENV", sizeof("_ENV")-1 TSRMLS_CC);
					_gpc_flags[3] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_ENV]) TSRMLS_CC);
					}
				}
				break;
			case 's':
			case 'S':
				if (!jit_initialization && !_gpc_flags[4]) {
					zend_auto_global_disable_jit("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
					php_register_server_variables(TSRMLS_C);
					_gpc_flags[4] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]) TSRMLS_CC);
					}
				}
				break;
		}
	}

	/* argv/argc support */
	if (PG(register_argc_argv)) {
		php_build_argv(SG(request_info).query_string, PG(http_globals)[TRACK_VARS_SERVER] TSRMLS_CC);
	}

	for (i=0; i<num_track_vars; i++) {
		if (jit_initialization && auto_global_records[i].jit_initialization) {
			continue;
		}
		if (!PG(http_globals)[i]) {
			ALLOC_ZVAL(PG(http_globals)[i]);
			array_init(PG(http_globals)[i]);
			INIT_PZVAL(PG(http_globals)[i]);
		}

		Z_ADDREF_P(PG(http_globals)[i]);
		zend_hash_update(&EG(symbol_table), auto_global_records[i].name, auto_global_records[i].name_len, &PG(http_globals)[i], sizeof(zval *), NULL);
		if (PG(register_long_arrays)) {
			zend_hash_update(&EG(symbol_table), auto_global_records[i].long_name, auto_global_records[i].long_name_len, &PG(http_globals)[i], sizeof(zval *), NULL);
			Z_ADDREF_P(PG(http_globals)[i]);
		}
	}

	/* Create _REQUEST */
	if (!jit_initialization) {
		zend_auto_global_disable_jit("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
		php_auto_globals_create_request("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
	}

	return SUCCESS;
}
/* }}} */

static zend_bool php_auto_globals_create_server(char *name, uint name_len TSRMLS_DC)
{
	if (PG(variables_order) && (strchr(PG(variables_order),'S') || strchr(PG(variables_order),'s'))) {
		php_register_server_variables(TSRMLS_C);

		if (PG(register_argc_argv)) {
			if (SG(request_info).argc) {
				zval **argc, **argv;
	
				if (zend_hash_find(&EG(symbol_table), "argc", sizeof("argc"), (void**)&argc) == SUCCESS &&
				    zend_hash_find(&EG(symbol_table), "argv", sizeof("argv"), (void**)&argv) == SUCCESS) {
					Z_ADDREF_PP(argc);
					Z_ADDREF_PP(argv);
					zend_hash_update(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "argv", sizeof("argv"), argv, sizeof(zval *), NULL);
					zend_hash_update(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "argc", sizeof("argc"), argc, sizeof(zval *), NULL);
				}
			} else {
				php_build_argv(SG(request_info).query_string, PG(http_globals)[TRACK_VARS_SERVER] TSRMLS_CC);
			}
		}
	
	} else {
		zval *server_vars=NULL;
		ALLOC_ZVAL(server_vars);
		array_init(server_vars);
		INIT_PZVAL(server_vars);
		if (PG(http_globals)[TRACK_VARS_SERVER]) {
			zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_SERVER]);
		}
		PG(http_globals)[TRACK_VARS_SERVER] = server_vars;
	}

	zend_hash_update(&EG(symbol_table), name, name_len + 1, &PG(http_globals)[TRACK_VARS_SERVER], sizeof(zval *), NULL);
	Z_ADDREF_P(PG(http_globals)[TRACK_VARS_SERVER]);

	if (PG(register_long_arrays)) {
		zend_hash_update(&EG(symbol_table), "HTTP_SERVER_VARS", sizeof("HTTP_SERVER_VARS"), &PG(http_globals)[TRACK_VARS_SERVER], sizeof(zval *), NULL);
		Z_ADDREF_P(PG(http_globals)[TRACK_VARS_SERVER]);
	}
	
	return 0; /* don't rearm */
}

static zend_bool php_auto_globals_create_env(char *name, uint name_len TSRMLS_DC)
{
	zval *env_vars = NULL;
	ALLOC_ZVAL(env_vars);
	array_init(env_vars);
	INIT_PZVAL(env_vars);
	if (PG(http_globals)[TRACK_VARS_ENV]) {
		zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_ENV]);
	}
	PG(http_globals)[TRACK_VARS_ENV] = env_vars;
	
	if (PG(variables_order) && (strchr(PG(variables_order),'E') || strchr(PG(variables_order),'e'))) {
		php_import_environment_variables(PG(http_globals)[TRACK_VARS_ENV] TSRMLS_CC);
	}

	zend_hash_update(&EG(symbol_table), name, name_len + 1, &PG(http_globals)[TRACK_VARS_ENV], sizeof(zval *), NULL);
	Z_ADDREF_P(PG(http_globals)[TRACK_VARS_ENV]);

	if (PG(register_long_arrays)) {
		zend_hash_update(&EG(symbol_table), "HTTP_ENV_VARS", sizeof("HTTP_ENV_VARS"), &PG(http_globals)[TRACK_VARS_ENV], sizeof(zval *), NULL);
		Z_ADDREF_P(PG(http_globals)[TRACK_VARS_ENV]);
	}

	return 0; /* don't rearm */
}

static zend_bool php_auto_globals_create_request(char *name, uint name_len TSRMLS_DC)
{
	zval *form_variables;
	unsigned char _gpc_flags[3] = {0, 0, 0};
	char *p;

	ALLOC_ZVAL(form_variables);
	array_init(form_variables);
	INIT_PZVAL(form_variables);

	if(PG(request_order) != NULL) {
		p = PG(request_order);
	} else {
		p = PG(variables_order);
	}

	for (; p && *p; p++) {
		switch (*p) {
			case 'g':
			case 'G':
				if (!_gpc_flags[0]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]) TSRMLS_CC);
					_gpc_flags[0] = 1;
				}
				break;
			case 'p':
			case 'P':
				if (!_gpc_flags[1]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]) TSRMLS_CC);
					_gpc_flags[1] = 1;
				}
				break;
			case 'c':
			case 'C':
				if (!_gpc_flags[2]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_COOKIE]) TSRMLS_CC);
					_gpc_flags[2] = 1;
				}
				break;
		}
	}

	zend_hash_update(&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), &form_variables, sizeof(zval *), NULL);
	return 0;
}

void php_startup_auto_globals(TSRMLS_D)
{
	zend_register_auto_global("_GET", sizeof("_GET")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_POST", sizeof("_POST")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_COOKIE", sizeof("_COOKIE")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_SERVER", sizeof("_SERVER")-1, php_auto_globals_create_server TSRMLS_CC);
	zend_register_auto_global("_ENV", sizeof("_ENV")-1, php_auto_globals_create_env TSRMLS_CC);
	zend_register_auto_global("_REQUEST", sizeof("_REQUEST")-1, php_auto_globals_create_request TSRMLS_CC);
	zend_register_auto_global("_FILES", sizeof("_FILES")-1, NULL TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
	{
		zval new_entry;
		Z_TYPE(new_entry) = IS_LONG;
		Z_LVAL(new_entry) = sapi_get_request_time(TSRMLS_C);
		php_register_variable_ex("REQUEST_TIME", &new_entry, array_ptr TSRMLS_CC);
	}

	if (magic_quotes_gpc) {
		zend_alter_ini_entry_ex("magic_quotes_gpc", sizeof("magic_quotes_gpc"), "1", 1, ZEND_INI_SYSTEM, ZEND_INI_STAGE_ACTIVATE, 1 TSRMLS_CC);
	}
}
/* }}} */

/* {{{ php_autoglobal_merge
 */
static void php_autoglobal_merge(HashTable *dest, HashTable *src TSRMLS_DC)
{
	zval **src_entry, **dest_entry;
	char *string_key;
	uint string_key_len;
	ulong num_key;
	HashPosition pos;
	int key_type;
	int globals_check = (PG(register_globals) && (dest == (&EG(symbol_table))));

	zend_hash_internal_pointer_reset_ex(src, &pos);
	while (zend_hash_get_current_data_ex(src, (void **)&src_entry, &pos) == SUCCESS) {
		key_type = zend_hash_get_current_key_ex(src, &string_key, &string_key_len, &num_key, 0, &pos);
		if (Z_TYPE_PP(src_entry) != IS_ARRAY
			|| (key_type == HASH_KEY_IS_STRING && zend_hash_find(dest, string_key, string_key_len, (void **) &dest_entry) != SUCCESS)
			|| (key_type == HASH_KEY_IS_LONG && zend_hash_index_find(dest, num_key, (void **)&dest_entry) != SUCCESS)
			|| Z_TYPE_PP(dest_entry) != IS_ARRAY
        ) {
			Z_ADDREF_PP(src_entry);
			if (key_type == HASH_KEY_IS_STRING) {
				/* if register_globals is on and working with main symbol table, prevent overwriting of GLOBALS */
				if (!globals_check || string_key_len != sizeof("GLOBALS") || memcmp(string_key, "GLOBALS", sizeof("GLOBALS") - 1)) {
					zend_hash_update(dest, string_key, string_key_len, src_entry, sizeof(zval *), NULL);
				} else {
					Z_DELREF_PP(src_entry);
				}
			} else {
				zend_hash_index_update(dest, num_key, src_entry, sizeof(zval *), NULL);
			}
		} else {
			SEPARATE_ZVAL(dest_entry);
			php_autoglobal_merge(Z_ARRVAL_PP(dest_entry), Z_ARRVAL_PP(src_entry) TSRMLS_CC);
		}
		zend_hash_move_forward_ex(src, &pos);
	}
}
/* }}} */

static zend_bool php_auto_globals_create_server(char *name, uint name_len TSRMLS_DC);
static zend_bool php_auto_globals_create_env(char *name, uint name_len TSRMLS_DC);
static zend_bool php_auto_globals_create_request(char *name, uint name_len TSRMLS_DC);

/* {{{ php_hash_environment
 */
int php_hash_environment(TSRMLS_D)
{
	char *p;
	unsigned char _gpc_flags[5] = {0, 0, 0, 0, 0};
	zend_bool jit_initialization = (PG(auto_globals_jit) && !PG(register_globals) && !PG(register_long_arrays));
	struct auto_global_record {
		char *name;
		uint name_len;
		char *long_name;
		uint long_name_len;
		zend_bool jit_initialization;
	} auto_global_records[] = {
		{ "_POST", sizeof("_POST"), "HTTP_POST_VARS", sizeof("HTTP_POST_VARS"), 0 },
		{ "_GET", sizeof("_GET"), "HTTP_GET_VARS", sizeof("HTTP_GET_VARS"), 0 },
		{ "_COOKIE", sizeof("_COOKIE"), "HTTP_COOKIE_VARS", sizeof("HTTP_COOKIE_VARS"), 0 },
		{ "_SERVER", sizeof("_SERVER"), "HTTP_SERVER_VARS", sizeof("HTTP_SERVER_VARS"), 1 },
		{ "_ENV", sizeof("_ENV"), "HTTP_ENV_VARS", sizeof("HTTP_ENV_VARS"), 1 },
		{ "_FILES", sizeof("_FILES"), "HTTP_POST_FILES", sizeof("HTTP_POST_FILES"), 0 },
	};
	size_t num_track_vars = sizeof(auto_global_records)/sizeof(struct auto_global_record);
	size_t i;

	/* jit_initialization = 0; */
	for (i=0; i<num_track_vars; i++) {
		PG(http_globals)[i] = NULL;
	}

	for (p=PG(variables_order); p && *p; p++) {
		switch(*p) {
			case 'p':
			case 'P':
				if (!_gpc_flags[0] && !SG(headers_sent) && SG(request_info).request_method && !strcasecmp(SG(request_info).request_method, "POST")) {
					sapi_module.treat_data(PARSE_POST, NULL, NULL TSRMLS_CC);	/* POST Data */
					_gpc_flags[0] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]) TSRMLS_CC);
					}
				}
				break;
			case 'c':
			case 'C':
				if (!_gpc_flags[1]) {
					sapi_module.treat_data(PARSE_COOKIE, NULL, NULL TSRMLS_CC);	/* Cookie Data */
					_gpc_flags[1] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_COOKIE]) TSRMLS_CC);
					}
				}
				break;
			case 'g':
			case 'G':
				if (!_gpc_flags[2]) {
					sapi_module.treat_data(PARSE_GET, NULL, NULL TSRMLS_CC);	/* GET Data */
					_gpc_flags[2] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]) TSRMLS_CC);
					}
				}
				break;
			case 'e':
			case 'E':
				if (!jit_initialization && !_gpc_flags[3]) {
					zend_auto_global_disable_jit("_ENV", sizeof("_ENV")-1 TSRMLS_CC);
					php_auto_globals_create_env("_ENV", sizeof("_ENV")-1 TSRMLS_CC);
					_gpc_flags[3] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_ENV]) TSRMLS_CC);
					}
				}
				break;
			case 's':
			case 'S':
				if (!jit_initialization && !_gpc_flags[4]) {
					zend_auto_global_disable_jit("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);
					php_register_server_variables(TSRMLS_C);
					_gpc_flags[4] = 1;
					if (PG(register_globals)) {
						php_autoglobal_merge(&EG(symbol_table), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]) TSRMLS_CC);
					}
				}
				break;
		}
	}

	/* argv/argc support */
	if (PG(register_argc_argv)) {
		php_build_argv(SG(request_info).query_string, PG(http_globals)[TRACK_VARS_SERVER] TSRMLS_CC);
	}

	for (i=0; i<num_track_vars; i++) {
		if (jit_initialization && auto_global_records[i].jit_initialization) {
			continue;
		}
		if (!PG(http_globals)[i]) {
			ALLOC_ZVAL(PG(http_globals)[i]);
			array_init(PG(http_globals)[i]);
			INIT_PZVAL(PG(http_globals)[i]);
		}

		Z_ADDREF_P(PG(http_globals)[i]);
		zend_hash_update(&EG(symbol_table), auto_global_records[i].name, auto_global_records[i].name_len, &PG(http_globals)[i], sizeof(zval *), NULL);
		if (PG(register_long_arrays)) {
			zend_hash_update(&EG(symbol_table), auto_global_records[i].long_name, auto_global_records[i].long_name_len, &PG(http_globals)[i], sizeof(zval *), NULL);
			Z_ADDREF_P(PG(http_globals)[i]);
		}
	}

	/* Create _REQUEST */
	if (!jit_initialization) {
		zend_auto_global_disable_jit("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
		php_auto_globals_create_request("_REQUEST", sizeof("_REQUEST")-1 TSRMLS_CC);
	}

	return SUCCESS;
}
/* }}} */

static zend_bool php_auto_globals_create_server(char *name, uint name_len TSRMLS_DC)
{
	if (PG(variables_order) && (strchr(PG(variables_order),'S') || strchr(PG(variables_order),'s'))) {
		php_register_server_variables(TSRMLS_C);

		if (PG(register_argc_argv)) {
			if (SG(request_info).argc) {
				zval **argc, **argv;
	
				if (zend_hash_find(&EG(symbol_table), "argc", sizeof("argc"), (void**)&argc) == SUCCESS &&
				    zend_hash_find(&EG(symbol_table), "argv", sizeof("argv"), (void**)&argv) == SUCCESS) {
					Z_ADDREF_PP(argc);
					Z_ADDREF_PP(argv);
					zend_hash_update(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "argv", sizeof("argv"), argv, sizeof(zval *), NULL);
					zend_hash_update(Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_SERVER]), "argc", sizeof("argc"), argc, sizeof(zval *), NULL);
				}
			} else {
				php_build_argv(SG(request_info).query_string, PG(http_globals)[TRACK_VARS_SERVER] TSRMLS_CC);
			}
		}
	
	} else {
		zval *server_vars=NULL;
		ALLOC_ZVAL(server_vars);
		array_init(server_vars);
		INIT_PZVAL(server_vars);
		if (PG(http_globals)[TRACK_VARS_SERVER]) {
			zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_SERVER]);
		}
		PG(http_globals)[TRACK_VARS_SERVER] = server_vars;
	}

	zend_hash_update(&EG(symbol_table), name, name_len + 1, &PG(http_globals)[TRACK_VARS_SERVER], sizeof(zval *), NULL);
	Z_ADDREF_P(PG(http_globals)[TRACK_VARS_SERVER]);

	if (PG(register_long_arrays)) {
		zend_hash_update(&EG(symbol_table), "HTTP_SERVER_VARS", sizeof("HTTP_SERVER_VARS"), &PG(http_globals)[TRACK_VARS_SERVER], sizeof(zval *), NULL);
		Z_ADDREF_P(PG(http_globals)[TRACK_VARS_SERVER]);
	}
	
	return 0; /* don't rearm */
}

static zend_bool php_auto_globals_create_env(char *name, uint name_len TSRMLS_DC)
{
	zval *env_vars = NULL;
	ALLOC_ZVAL(env_vars);
	array_init(env_vars);
	INIT_PZVAL(env_vars);
	if (PG(http_globals)[TRACK_VARS_ENV]) {
		zval_ptr_dtor(&PG(http_globals)[TRACK_VARS_ENV]);
	}
	PG(http_globals)[TRACK_VARS_ENV] = env_vars;
	
	if (PG(variables_order) && (strchr(PG(variables_order),'E') || strchr(PG(variables_order),'e'))) {
		php_import_environment_variables(PG(http_globals)[TRACK_VARS_ENV] TSRMLS_CC);
	}

	zend_hash_update(&EG(symbol_table), name, name_len + 1, &PG(http_globals)[TRACK_VARS_ENV], sizeof(zval *), NULL);
	Z_ADDREF_P(PG(http_globals)[TRACK_VARS_ENV]);

	if (PG(register_long_arrays)) {
		zend_hash_update(&EG(symbol_table), "HTTP_ENV_VARS", sizeof("HTTP_ENV_VARS"), &PG(http_globals)[TRACK_VARS_ENV], sizeof(zval *), NULL);
		Z_ADDREF_P(PG(http_globals)[TRACK_VARS_ENV]);
	}

	return 0; /* don't rearm */
}

static zend_bool php_auto_globals_create_request(char *name, uint name_len TSRMLS_DC)
{
	zval *form_variables;
	unsigned char _gpc_flags[3] = {0, 0, 0};
	char *p;

	ALLOC_ZVAL(form_variables);
	array_init(form_variables);
	INIT_PZVAL(form_variables);

	if(PG(request_order) != NULL) {
		p = PG(request_order);
	} else {
		p = PG(variables_order);
	}

	for (; p && *p; p++) {
		switch (*p) {
			case 'g':
			case 'G':
				if (!_gpc_flags[0]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_GET]) TSRMLS_CC);
					_gpc_flags[0] = 1;
				}
				break;
			case 'p':
			case 'P':
				if (!_gpc_flags[1]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_POST]) TSRMLS_CC);
					_gpc_flags[1] = 1;
				}
				break;
			case 'c':
			case 'C':
				if (!_gpc_flags[2]) {
					php_autoglobal_merge(Z_ARRVAL_P(form_variables), Z_ARRVAL_P(PG(http_globals)[TRACK_VARS_COOKIE]) TSRMLS_CC);
					_gpc_flags[2] = 1;
				}
				break;
		}
	}

	zend_hash_update(&EG(symbol_table), "_REQUEST", sizeof("_REQUEST"), &form_variables, sizeof(zval *), NULL);
	return 0;
}

void php_startup_auto_globals(TSRMLS_D)
{
	zend_register_auto_global("_GET", sizeof("_GET")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_POST", sizeof("_POST")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_COOKIE", sizeof("_COOKIE")-1, NULL TSRMLS_CC);
	zend_register_auto_global("_SERVER", sizeof("_SERVER")-1, php_auto_globals_create_server TSRMLS_CC);
	zend_register_auto_global("_ENV", sizeof("_ENV")-1, php_auto_globals_create_env TSRMLS_CC);
	zend_register_auto_global("_REQUEST", sizeof("_REQUEST")-1, php_auto_globals_create_request TSRMLS_CC);
	zend_register_auto_global("_FILES", sizeof("_FILES")-1, NULL TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */