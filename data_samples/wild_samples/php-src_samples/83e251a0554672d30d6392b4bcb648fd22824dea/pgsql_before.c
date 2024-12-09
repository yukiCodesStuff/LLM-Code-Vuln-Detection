		} else {
			field_escaped[j++] = field[i];
		}
	}
	field_escaped[j++] = '"';
	field_escaped[j] = '\0';
	return field_escaped;
}
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
STD_PHP_INI_BOOLEAN( "pgsql.allow_persistent",      "1",  PHP_INI_SYSTEM, OnUpdateBool, allow_persistent,      zend_pgsql_globals, pgsql_globals)
STD_PHP_INI_ENTRY_EX("pgsql.max_persistent",       "-1",  PHP_INI_SYSTEM, OnUpdateLong, max_persistent,        zend_pgsql_globals, pgsql_globals, display_link_numbers)
STD_PHP_INI_ENTRY_EX("pgsql.max_links",            "-1",  PHP_INI_SYSTEM, OnUpdateLong, max_links,             zend_pgsql_globals, pgsql_globals, display_link_numbers)
STD_PHP_INI_BOOLEAN( "pgsql.auto_reset_persistent", "0",  PHP_INI_SYSTEM, OnUpdateBool, auto_reset_persistent, zend_pgsql_globals, pgsql_globals)
STD_PHP_INI_BOOLEAN( "pgsql.ignore_notice",         "0",  PHP_INI_ALL,    OnUpdateBool, ignore_notices,        zend_pgsql_globals, pgsql_globals)
STD_PHP_INI_BOOLEAN( "pgsql.log_notice",            "0",  PHP_INI_ALL,    OnUpdateBool, log_notices,           zend_pgsql_globals, pgsql_globals)
PHP_INI_END()
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(pgsql)
{
	memset(pgsql_globals, 0, sizeof(zend_pgsql_globals));
	/* Initilize notice message hash at MINIT only */
	zend_hash_init_ex(&pgsql_globals->notices, 0, NULL, PHP_PGSQL_NOTICE_PTR_DTOR, 1, 0); 
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(pgsql)
{
	REGISTER_INI_ENTRIES();
	
	le_link = zend_register_list_destructors_ex(_close_pgsql_link, NULL, "pgsql link", module_number);
	le_plink = zend_register_list_destructors_ex(NULL, _close_pgsql_plink, "pgsql link persistent", module_number);
	le_result = zend_register_list_destructors_ex(_free_result, NULL, "pgsql result", module_number);
	le_lofp = zend_register_list_destructors_ex(_free_ptr, NULL, "pgsql large object", module_number);
	le_string = zend_register_list_destructors_ex(_free_ptr, NULL, "pgsql string", module_number);
#if HAVE_PG_CONFIG_H
	/* PG_VERSION - libpq version */
	REGISTER_STRING_CONSTANT("PGSQL_LIBPQ_VERSION", PG_VERSION, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("PGSQL_LIBPQ_VERSION_STR", PG_VERSION_STR, CONST_CS | CONST_PERSISTENT);
#endif
	/* For connection option */
	REGISTER_LONG_CONSTANT("PGSQL_CONNECT_FORCE_NEW", PGSQL_CONNECT_FORCE_NEW, CONST_CS | CONST_PERSISTENT);
	/* For pg_fetch_array() */
	REGISTER_LONG_CONSTANT("PGSQL_ASSOC", PGSQL_ASSOC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_NUM", PGSQL_NUM, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_BOTH", PGSQL_BOTH, CONST_CS | CONST_PERSISTENT);
	/* For pg_connection_status() */
	REGISTER_LONG_CONSTANT("PGSQL_CONNECTION_BAD", CONNECTION_BAD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_CONNECTION_OK", CONNECTION_OK, CONST_CS | CONST_PERSISTENT);
#if HAVE_PGTRANSACTIONSTATUS
	/* For pg_transaction_status() */
	REGISTER_LONG_CONSTANT("PGSQL_TRANSACTION_IDLE", PQTRANS_IDLE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_TRANSACTION_ACTIVE", PQTRANS_ACTIVE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_TRANSACTION_INTRANS", PQTRANS_INTRANS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_TRANSACTION_INERROR", PQTRANS_INERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_TRANSACTION_UNKNOWN", PQTRANS_UNKNOWN, CONST_CS | CONST_PERSISTENT);
#endif
#if HAVE_PQSETERRORVERBOSITY
	/* For pg_set_error_verbosity() */
	REGISTER_LONG_CONSTANT("PGSQL_ERRORS_TERSE", PQERRORS_TERSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_ERRORS_DEFAULT", PQERRORS_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_ERRORS_VERBOSE", PQERRORS_VERBOSE, CONST_CS | CONST_PERSISTENT);
#endif
	/* For lo_seek() */
	REGISTER_LONG_CONSTANT("PGSQL_SEEK_SET", SEEK_SET, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_SEEK_CUR", SEEK_CUR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_SEEK_END", SEEK_END, CONST_CS | CONST_PERSISTENT);
	/* For pg_result_status() return value type */
	REGISTER_LONG_CONSTANT("PGSQL_STATUS_LONG", PGSQL_STATUS_LONG, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_STATUS_STRING", PGSQL_STATUS_STRING, CONST_CS | CONST_PERSISTENT);
	/* For pg_result_status() return value */
	REGISTER_LONG_CONSTANT("PGSQL_EMPTY_QUERY", PGRES_EMPTY_QUERY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_COMMAND_OK", PGRES_COMMAND_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_TUPLES_OK", PGRES_TUPLES_OK, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_COPY_OUT", PGRES_COPY_OUT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_COPY_IN", PGRES_COPY_IN, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_BAD_RESPONSE", PGRES_BAD_RESPONSE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_NONFATAL_ERROR", PGRES_NONFATAL_ERROR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_FATAL_ERROR", PGRES_FATAL_ERROR, CONST_CS | CONST_PERSISTENT);
#if HAVE_PQRESULTERRORFIELD
	/* For pg_result_error_field() field codes */
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_SEVERITY", PG_DIAG_SEVERITY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_SQLSTATE", PG_DIAG_SQLSTATE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_MESSAGE_PRIMARY", PG_DIAG_MESSAGE_PRIMARY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_MESSAGE_DETAIL", PG_DIAG_MESSAGE_DETAIL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_MESSAGE_HINT", PG_DIAG_MESSAGE_HINT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_STATEMENT_POSITION", PG_DIAG_STATEMENT_POSITION, CONST_CS | CONST_PERSISTENT);
#ifdef PG_DIAG_INTERNAL_POSITION
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_INTERNAL_POSITION", PG_DIAG_INTERNAL_POSITION, CONST_CS | CONST_PERSISTENT);
#endif
#ifdef PG_DIAG_INTERNAL_QUERY
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_INTERNAL_QUERY", PG_DIAG_INTERNAL_QUERY, CONST_CS | CONST_PERSISTENT);
#endif
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_CONTEXT", PG_DIAG_CONTEXT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_SOURCE_FILE", PG_DIAG_SOURCE_FILE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_SOURCE_LINE", PG_DIAG_SOURCE_LINE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DIAG_SOURCE_FUNCTION", PG_DIAG_SOURCE_FUNCTION, CONST_CS | CONST_PERSISTENT);
#endif
	/* pg_convert options */
	REGISTER_LONG_CONSTANT("PGSQL_CONV_IGNORE_DEFAULT", PGSQL_CONV_IGNORE_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_CONV_FORCE_NULL", PGSQL_CONV_FORCE_NULL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_CONV_IGNORE_NOT_NULL", PGSQL_CONV_IGNORE_NOT_NULL, CONST_CS | CONST_PERSISTENT);
	/* pg_insert/update/delete/select options */
	REGISTER_LONG_CONSTANT("PGSQL_DML_NO_CONV", PGSQL_DML_NO_CONV, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DML_EXEC", PGSQL_DML_EXEC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DML_ASYNC", PGSQL_DML_ASYNC, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PGSQL_DML_STRING", PGSQL_DML_STRING, CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(pgsql)
{
	UNREGISTER_INI_ENTRIES();
	zend_hash_destroy(&PGG(notices));

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(pgsql)
{
	PGG(default_link)=-1;
	PGG(num_links) = PGG(num_persistent);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(pgsql)
{
	/* clean up notice messages */
	zend_hash_clean(&PGG(notices));
	/* clean up persistent connection */
	zend_hash_apply(&EG(persistent_list), (apply_func_t) _rollback_transactions TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(pgsql)
{
	char buf[256];

	php_info_print_table_start();
	php_info_print_table_header(2, "PostgreSQL Support", "enabled");
#if HAVE_PG_CONFIG_H
	php_info_print_table_row(2, "PostgreSQL(libpq) Version", PG_VERSION);
	php_info_print_table_row(2, "PostgreSQL(libpq) ", PG_VERSION_STR);
#ifdef HAVE_PGSQL_WITH_MULTIBYTE_SUPPORT
	php_info_print_table_row(2, "Multibyte character support", "enabled");
#else
	php_info_print_table_row(2, "Multibyte character support", "disabled");
#endif
#ifdef USE_SSL
	php_info_print_table_row(2, "SSL support", "enabled");
#else
	php_info_print_table_row(2, "SSL support", "disabled");
#endif
#endif /* HAVE_PG_CONFIG_H */	
	snprintf(buf, sizeof(buf), "%ld", PGG(num_persistent));
	php_info_print_table_row(2, "Active Persistent Links", buf);
	snprintf(buf, sizeof(buf), "%ld", PGG(num_links));
	php_info_print_table_row(2, "Active Links", buf);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */


/* {{{ php_pgsql_do_connect
 */
static void php_pgsql_do_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent)
{
	char *host=NULL,*port=NULL,*options=NULL,*tty=NULL,*dbname=NULL,*connstring=NULL;
	PGconn *pgsql;
	smart_str str = {0};
	zval **args[5];
	int i, connect_type = 0;
	PGresult *pg_result;

	if (ZEND_NUM_ARGS() < 1 || ZEND_NUM_ARGS() > 5
			|| zend_get_parameters_array_ex(ZEND_NUM_ARGS(), args) == FAILURE) {
		WRONG_PARAM_COUNT;
	}
static void php_pgsql_escape_internal(INTERNAL_FUNCTION_PARAMETERS, int escape_literal) {
	char *from = NULL, *to = NULL, *tmp = NULL;
	zval *pgsql_link = NULL;
	PGconn *pgsql;
	int to_len;
	int from_len;
	int id = -1;

	switch (ZEND_NUM_ARGS()) {
		case 1:
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &from, &from_len) == FAILURE) {
				return;
			}
			pgsql_link = NULL;
			id = PGG(default_link);
			break;

		default:
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &pgsql_link, &from, &from_len) == FAILURE) {
				return;
			}
			break;
	}
		if (!skip_field) {
			char *escaped;
			size_t new_len, field_len = strlen(field);

			if (_php_pgsql_detect_identifier_escape(field, field_len) == SUCCESS) {
				escaped = strndup(field, field_len);
			} else {
#if HAVE_PQESCAPELITERAL
				escaped = PQescapeIdentifier(pg_link, field, field_len);
#else
				escaped = _php_pgsql_escape_identifier(field, field_len);
#endif
			}
			add_assoc_zval(result, escaped, new_val);
			free(escaped);
		}
	} /* for */
	zval_dtor(meta);
	FREE_ZVAL(meta);

	if (err) {
		/* shouldn't destroy & free zval here */
		return FAILURE;
	}
	return SUCCESS;
}
{
	char *table_copy, *escaped, *token, *tmp;
	size_t len;

	/* schame.table should be "schame"."table" */
	table_copy = estrdup(table);
	token = php_strtok_r(table_copy, ".", &tmp);
	len = strlen(token);
	if (_php_pgsql_detect_identifier_escape(token, len) == SUCCESS) {
		escaped = strndup(token, len);
	} else {
#if HAVE_PQESCAPELITERAL
		escaped = PQescapeIdentifier(pg_link, token, len);
#else
		escaped = _php_pgsql_escape_identifier(token, len);
#endif
	}
	smart_str_appends(querystr, escaped);
	free(escaped);
	if (tmp && *tmp) {
		len = strlen(tmp);
		/* "schema"."table" format */
		if (_php_pgsql_detect_identifier_escape(tmp, len) == SUCCESS) {
			escaped = strndup(tmp, len);
		} else {
#if HAVE_PQESCAPELITERAL
			escaped = PQescapeIdentifier(pg_link, tmp, len);
#else
			escaped = _php_pgsql_escape_identifier(tmp, len);
#endif
		}
		smart_str_appendc(querystr, '.');
		smart_str_appends(querystr, escaped);
		free(escaped);
	}
	efree(table_copy);
}

/* {{{ php_pgsql_insert
 */
PHP_PGSQL_API int php_pgsql_insert(PGconn *pg_link, const char *table, zval *var_array, ulong opt, char **sql TSRMLS_DC)
{
	zval **val, *converted = NULL;
	char buf[256];
	char *fld;
	smart_str querystr = {0};
	int key_type, ret = FAILURE;
	uint fld_len;
	ulong num_idx;
	HashPosition pos;

	assert(pg_link != NULL);
	assert(table != NULL);
	assert(Z_TYPE_P(var_array) == IS_ARRAY);

	if (zend_hash_num_elements(Z_ARRVAL_P(var_array)) == 0) {
		smart_str_appends(&querystr, "INSERT INTO ");
		build_tablename(&querystr, pg_link, table);
		smart_str_appends(&querystr, " DEFAULT VALUES");

		goto no_values;
	}

	/* convert input array if needed */
	if (!(opt & PGSQL_DML_NO_CONV)) {
		MAKE_STD_ZVAL(converted);
		array_init(converted);
		if (php_pgsql_convert(pg_link, table, var_array, converted, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == FAILURE) {
			goto cleanup;
		}
		var_array = converted;
	}

	smart_str_appends(&querystr, "INSERT INTO ");
	build_tablename(&querystr, pg_link, table);
	smart_str_appends(&querystr, " (");

	zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(var_array), &pos);
	while ((key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(var_array), &fld,
					&fld_len, &num_idx, 0, &pos)) != HASH_KEY_NON_EXISTENT) {
		if (key_type == HASH_KEY_IS_LONG) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects associative array for values to be inserted");
			goto cleanup;
		}
		smart_str_appendl(&querystr, fld, fld_len - 1);
		smart_str_appendc(&querystr, ',');
		zend_hash_move_forward_ex(Z_ARRVAL_P(var_array), &pos);
	}
	querystr.len--;
	smart_str_appends(&querystr, ") VALUES (");
	
	/* make values string */
	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(var_array), &pos);
		 zend_hash_get_current_data_ex(Z_ARRVAL_P(var_array), (void **)&val, &pos) == SUCCESS;
		 zend_hash_move_forward_ex(Z_ARRVAL_P(var_array), &pos)) {
		
		/* we can avoid the key_type check here, because we tested it in the other loop */
		switch(Z_TYPE_PP(val)) {
			case IS_STRING:
				smart_str_appendl(&querystr, Z_STRVAL_PP(val), Z_STRLEN_PP(val));
				break;
			case IS_LONG:
				smart_str_append_long(&querystr, Z_LVAL_PP(val));
				break;
			case IS_DOUBLE:
				smart_str_appendl(&querystr, buf, snprintf(buf, sizeof(buf), "%F", Z_DVAL_PP(val)));
				break;
			default:
				/* should not happen */
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Report this error to php-dev@lists.php.net, type = %d", Z_TYPE_PP(val));
				goto cleanup;
				break;
		}
		smart_str_appendc(&querystr, ',');
	}
	/* Remove the trailing "," */
	querystr.len--;
	smart_str_appends(&querystr, ");");

no_values:

	smart_str_0(&querystr);

	if ((opt & (PGSQL_DML_EXEC|PGSQL_DML_ASYNC)) &&
		do_exec(&querystr, PGRES_COMMAND_OK, pg_link, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == 0) {
		ret = SUCCESS;
	}
	else if (opt & PGSQL_DML_STRING) {
		ret = SUCCESS;
	}
	
cleanup:
	if (!(opt & PGSQL_DML_NO_CONV) && converted) {
		zval_dtor(converted);			
		FREE_ZVAL(converted);
	}
	if (ret == SUCCESS && (opt & PGSQL_DML_STRING)) {
		*sql = querystr.c;
	}
	else {
		smart_str_free(&querystr);
	}
	return ret;
}
/* }}} */

/* {{{ proto mixed pg_insert(resource db, string table, array values[, int options])
   Insert values (filed=>value) to table */
PHP_FUNCTION(pg_insert)
{
	zval *pgsql_link, *values;
	char *table, *sql = NULL;
	int table_len;
	ulong option = PGSQL_DML_EXEC;
	PGconn *pg_link;
	int id = -1, argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "rsa|l",
							  &pgsql_link, &table, &table_len, &values, &option) == FAILURE) {
		return;
	}
	if (option & ~(PGSQL_CONV_OPTS|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_ASYNC|PGSQL_DML_STRING)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid option is specified");
		RETURN_FALSE;
	}
	
	ZEND_FETCH_RESOURCE2(pg_link, PGconn *, &pgsql_link, id, "PostgreSQL link", le_link, le_plink);

	if (php_pgsql_flush_query(pg_link TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected unhandled result(s) in connection");
	}
	if (php_pgsql_insert(pg_link, table, values, option, &sql TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}
	if (option & PGSQL_DML_STRING) {
		RETURN_STRING(sql, 0);
	}
	RETURN_TRUE;
}
/* }}} */

static inline int build_assignment_string(smart_str *querystr, HashTable *ht, int where_cond, const char *pad, int pad_len TSRMLS_DC)
{
	HashPosition pos;
	uint fld_len;
	int key_type;
	ulong num_idx;
	char *fld;
	char buf[256];
	zval **val;

	for (zend_hash_internal_pointer_reset_ex(ht, &pos);
		 zend_hash_get_current_data_ex(ht, (void **)&val, &pos) == SUCCESS;
		 zend_hash_move_forward_ex(ht, &pos)) {
		 key_type = zend_hash_get_current_key_ex(ht, &fld, &fld_len, &num_idx, 0, &pos);		
		if (key_type == HASH_KEY_IS_LONG) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects associative array for values to be inserted");
			return -1;
		}
		smart_str_appendl(querystr, fld, fld_len - 1);
		if (where_cond && Z_TYPE_PP(val) == IS_STRING && !strcmp(Z_STRVAL_PP(val), "NULL")) {
			smart_str_appends(querystr, " IS ");
		} else {
			smart_str_appendc(querystr, '=');
		}
		
		switch(Z_TYPE_PP(val)) {
			case IS_STRING:
				smart_str_appendl(querystr, Z_STRVAL_PP(val), Z_STRLEN_PP(val));
				break;
			case IS_LONG:
				smart_str_append_long(querystr, Z_LVAL_PP(val));
				break;
			case IS_DOUBLE:
				smart_str_appendl(querystr, buf, MIN(snprintf(buf, sizeof(buf), "%F", Z_DVAL_PP(val)), sizeof(buf)-1));
				break;
			default:
				/* should not happen */
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects scaler values other than NULL. Need to convert?");
				return -1;
		}
		smart_str_appendl(querystr, pad, pad_len);
	}
	querystr->len -= pad_len;

	return 0;
}

/* {{{ php_pgsql_update
 */
PHP_PGSQL_API int php_pgsql_update(PGconn *pg_link, const char *table, zval *var_array, zval *ids_array, ulong opt, char **sql TSRMLS_DC) 
{
	zval *var_converted = NULL, *ids_converted = NULL;
	smart_str querystr = {0};
	int ret = FAILURE;

	assert(pg_link != NULL);
	assert(table != NULL);
	assert(Z_TYPE_P(var_array) == IS_ARRAY);
	assert(Z_TYPE_P(ids_array) == IS_ARRAY);
	assert(!(opt & ~(PGSQL_CONV_OPTS|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_STRING)));

	if (zend_hash_num_elements(Z_ARRVAL_P(var_array)) == 0
			|| zend_hash_num_elements(Z_ARRVAL_P(ids_array)) == 0) {
		return FAILURE;
	}

	if (!(opt & PGSQL_DML_NO_CONV)) {
		MAKE_STD_ZVAL(var_converted);
		array_init(var_converted);
		if (php_pgsql_convert(pg_link, table, var_array, var_converted, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == FAILURE) {
			goto cleanup;
		}
		var_array = var_converted;
		MAKE_STD_ZVAL(ids_converted);
		array_init(ids_converted);
		if (php_pgsql_convert(pg_link, table, ids_array, ids_converted, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == FAILURE) {
			goto cleanup;
		}
		ids_array = ids_converted;
	}

	smart_str_appends(&querystr, "UPDATE ");
	build_tablename(&querystr, pg_link, table);
	smart_str_appends(&querystr, " SET ");

	if (build_assignment_string(&querystr, Z_ARRVAL_P(var_array), 0, ",", 1 TSRMLS_CC))
		goto cleanup;
	
	smart_str_appends(&querystr, " WHERE ");
	
	if (build_assignment_string(&querystr, Z_ARRVAL_P(ids_array), 1, " AND ", sizeof(" AND ")-1 TSRMLS_CC))
		goto cleanup;

	smart_str_appendc(&querystr, ';');	
	smart_str_0(&querystr);

	if ((opt & PGSQL_DML_EXEC) && do_exec(&querystr, PGRES_COMMAND_OK, pg_link, opt TSRMLS_CC) == 0) {
		ret = SUCCESS;
	} else if (opt & PGSQL_DML_STRING) {
		ret = SUCCESS;
	}

cleanup:
	if (var_converted) {
		zval_dtor(var_converted);
		FREE_ZVAL(var_converted);
	}
	if (ids_converted) {
		zval_dtor(ids_converted);
		FREE_ZVAL(ids_converted);
	}
	if (ret == SUCCESS && (opt & PGSQL_DML_STRING)) {
		*sql = querystr.c;
	}
	else {
		smart_str_free(&querystr);
	}
	return ret;
}
/* }}} */

/* {{{ proto mixed pg_update(resource db, string table, array fields, array ids[, int options])
   Update table using values (field=>value) and ids (id=>value) */
PHP_FUNCTION(pg_update)
{
	zval *pgsql_link, *values, *ids;
	char *table, *sql = NULL;
	int table_len;
	ulong option =  PGSQL_DML_EXEC;
	PGconn *pg_link;
	int id = -1, argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "rsaa|l",
							  &pgsql_link, &table, &table_len, &values, &ids, &option) == FAILURE) {
		return;
	}
	if (option & ~(PGSQL_CONV_OPTS|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_STRING)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid option is specified");
		RETURN_FALSE;
	}
	
	ZEND_FETCH_RESOURCE2(pg_link, PGconn *, &pgsql_link, id, "PostgreSQL link", le_link, le_plink);

	if (php_pgsql_flush_query(pg_link TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected unhandled result(s) in connection");
	}
	if (php_pgsql_update(pg_link, table, values, ids, option, &sql TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}
	if (option & PGSQL_DML_STRING) {
		RETURN_STRING(sql, 0);
	}
	RETURN_TRUE;
} 
/* }}} */

/* {{{ php_pgsql_delete
 */
PHP_PGSQL_API int php_pgsql_delete(PGconn *pg_link, const char *table, zval *ids_array, ulong opt, char **sql TSRMLS_DC) 
{
	zval *ids_converted = NULL;
	smart_str querystr = {0};
	int ret = FAILURE;

	assert(pg_link != NULL);
	assert(table != NULL);
	assert(Z_TYPE_P(ids_array) == IS_ARRAY);
	assert(!(opt & ~(PGSQL_CONV_FORCE_NULL|PGSQL_DML_EXEC|PGSQL_DML_STRING)));
	
	if (zend_hash_num_elements(Z_ARRVAL_P(ids_array)) == 0) {
		return FAILURE;
	}

	if (!(opt & PGSQL_DML_NO_CONV)) {
		MAKE_STD_ZVAL(ids_converted);
		array_init(ids_converted);
		if (php_pgsql_convert(pg_link, table, ids_array, ids_converted, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == FAILURE) {
			goto cleanup;
		}
		ids_array = ids_converted;
	}

	smart_str_appends(&querystr, "DELETE FROM ");
	build_tablename(&querystr, pg_link, table);
	smart_str_appends(&querystr, " WHERE ");

	if (build_assignment_string(&querystr, Z_ARRVAL_P(ids_array), 1, " AND ", sizeof(" AND ")-1 TSRMLS_CC))
		goto cleanup;

	smart_str_appendc(&querystr, ';');
	smart_str_0(&querystr);

	if ((opt & PGSQL_DML_EXEC) && do_exec(&querystr, PGRES_COMMAND_OK, pg_link, opt TSRMLS_CC) == 0) {
		ret = SUCCESS;
	} else if (opt & PGSQL_DML_STRING) {
		ret = SUCCESS;
	}

cleanup:
	if (!(opt & PGSQL_DML_NO_CONV)) {
		zval_dtor(ids_converted);			
		FREE_ZVAL(ids_converted);
	}
	if (ret == SUCCESS && (opt & PGSQL_DML_STRING)) {
		*sql = querystr.c;
	}
	else {
		smart_str_free(&querystr);
	}
	return ret;
}
/* }}} */

/* {{{ proto mixed pg_delete(resource db, string table, array ids[, int options])
   Delete records has ids (id=>value) */
PHP_FUNCTION(pg_delete)
{
	zval *pgsql_link, *ids;
	char *table, *sql = NULL;
	int table_len;
	ulong option = PGSQL_DML_EXEC;
	PGconn *pg_link;
	int id = -1, argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "rsa|l",
							  &pgsql_link, &table, &table_len, &ids, &option) == FAILURE) {
		return;
	}
	if (option & ~(PGSQL_CONV_FORCE_NULL|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_STRING)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid option is specified");
		RETURN_FALSE;
	}
	
	ZEND_FETCH_RESOURCE2(pg_link, PGconn *, &pgsql_link, id, "PostgreSQL link", le_link, le_plink);

	if (php_pgsql_flush_query(pg_link TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected unhandled result(s) in connection");
	}
	if (php_pgsql_delete(pg_link, table, ids, option, &sql TSRMLS_CC) == FAILURE) {
		RETURN_FALSE;
	}
	if (option & PGSQL_DML_STRING) {
		RETURN_STRING(sql, 0);
	}
	RETURN_TRUE;
} 
/* }}} */

/* {{{ php_pgsql_result2array
 */
PHP_PGSQL_API int php_pgsql_result2array(PGresult *pg_result, zval *ret_array TSRMLS_DC) 
{
	zval *row;
	char *field_name;
	size_t num_fields;
	int pg_numrows, pg_row;
	uint i;
	assert(Z_TYPE_P(ret_array) == IS_ARRAY);

	if ((pg_numrows = PQntuples(pg_result)) <= 0) {
		return FAILURE;
	}
	for (pg_row = 0; pg_row < pg_numrows; pg_row++) {
		MAKE_STD_ZVAL(row);
		array_init(row);
		add_index_zval(ret_array, pg_row, row);
		for (i = 0, num_fields = PQnfields(pg_result); i < num_fields; i++) {
			if (PQgetisnull(pg_result, pg_row, i)) {
				field_name = PQfname(pg_result, i);
				add_assoc_null(row, field_name);
			} else {
				char *element = PQgetvalue(pg_result, pg_row, i);
				if (element) {
					char *data;
					size_t data_len;
					const size_t element_len = strlen(element);

					data = safe_estrndup(element, element_len);
					data_len = element_len;
					
					field_name = PQfname(pg_result, i);
					add_assoc_stringl(row, field_name, data, data_len, 0);
				}
			}
		}
	}
	return SUCCESS;
}
/* }}} */

/* {{{ php_pgsql_select
 */
PHP_PGSQL_API int php_pgsql_select(PGconn *pg_link, const char *table, zval *ids_array, zval *ret_array, ulong opt, char **sql TSRMLS_DC) 
{
	zval *ids_converted = NULL;
	smart_str querystr = {0};
	int ret = FAILURE;
	PGresult *pg_result;

	assert(pg_link != NULL);
	assert(table != NULL);
	assert(Z_TYPE_P(ids_array) == IS_ARRAY);
	assert(Z_TYPE_P(ret_array) == IS_ARRAY);
	assert(!(opt & ~(PGSQL_CONV_OPTS|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_ASYNC|PGSQL_DML_STRING)));
	
	if (zend_hash_num_elements(Z_ARRVAL_P(ids_array)) == 0) {
		return FAILURE;
	}

	if (!(opt & PGSQL_DML_NO_CONV)) {
		MAKE_STD_ZVAL(ids_converted);
		array_init(ids_converted);
		if (php_pgsql_convert(pg_link, table, ids_array, ids_converted, (opt & PGSQL_CONV_OPTS) TSRMLS_CC) == FAILURE) {
			goto cleanup;
		}
		ids_array = ids_converted;
	}

	smart_str_appends(&querystr, "SELECT * FROM ");
	build_tablename(&querystr, pg_link, table);
	smart_str_appends(&querystr, " WHERE ");

	if (build_assignment_string(&querystr, Z_ARRVAL_P(ids_array), 1, " AND ", sizeof(" AND ")-1 TSRMLS_CC))
		goto cleanup;

	smart_str_appendc(&querystr, ';');
	smart_str_0(&querystr);

	pg_result = PQexec(pg_link, querystr.c);
	if (PQresultStatus(pg_result) == PGRES_TUPLES_OK) {
		ret = php_pgsql_result2array(pg_result, ret_array TSRMLS_CC);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Failed to execute '%s'", querystr.c);
	}
	PQclear(pg_result);

cleanup:
	if (!(opt & PGSQL_DML_NO_CONV)) {
		zval_dtor(ids_converted);			
		FREE_ZVAL(ids_converted);
	}
	if (ret == SUCCESS && (opt & PGSQL_DML_STRING)) {
		*sql = querystr.c;
	}
	else {
		smart_str_free(&querystr);
	}
	return ret;
}
/* }}} */

/* {{{ proto mixed pg_select(resource db, string table, array ids[, int options])
   Select records that has ids (id=>value) */
PHP_FUNCTION(pg_select)
{
	zval *pgsql_link, *ids;
	char *table, *sql = NULL;
	int table_len;
	ulong option = PGSQL_DML_EXEC;
	PGconn *pg_link;
	int id = -1, argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc TSRMLS_CC, "rsa|l",
							  &pgsql_link, &table, &table_len, &ids, &option) == FAILURE) {
		return;
	}
	if (option & ~(PGSQL_CONV_FORCE_NULL|PGSQL_DML_NO_CONV|PGSQL_DML_EXEC|PGSQL_DML_ASYNC|PGSQL_DML_STRING)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid option is specified");
		RETURN_FALSE;
	}
	
	ZEND_FETCH_RESOURCE2(pg_link, PGconn *, &pgsql_link, id, "PostgreSQL link", le_link, le_plink);

	if (php_pgsql_flush_query(pg_link TSRMLS_CC)) {
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected unhandled result(s) in connection");
	}
	array_init(return_value);
	if (php_pgsql_select(pg_link, table, ids, return_value, option, &sql TSRMLS_CC) == FAILURE) {
		zval_dtor(return_value);
		RETURN_FALSE;
	}
	if (option & PGSQL_DML_STRING) {
		zval_dtor(return_value);
		RETURN_STRING(sql, 0);
	}
	return;
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
	if (tmp && *tmp) {
		len = strlen(tmp);
		/* "schema"."table" format */
		if (_php_pgsql_detect_identifier_escape(tmp, len) == SUCCESS) {
			escaped = strndup(tmp, len);
		} else {
#if HAVE_PQESCAPELITERAL
			escaped = PQescapeIdentifier(pg_link, tmp, len);
#else
			escaped = _php_pgsql_escape_identifier(tmp, len);
#endif
		}
		smart_str_appendc(querystr, '.');
		smart_str_appends(querystr, escaped);
		free(escaped);
	}