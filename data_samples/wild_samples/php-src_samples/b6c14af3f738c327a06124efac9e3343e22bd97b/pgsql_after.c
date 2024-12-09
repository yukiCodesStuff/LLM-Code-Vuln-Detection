		} else {
			field_escaped[j++] = field[i];
		}
	}
	field_escaped[j++] = '"';
	field_escaped[j] = '\0';
	return field_escaped;
}
/* }}} */
#endif

/* {{{ _php_pgsql_strndup, no strndup should be used */
static char *_php_pgsql_strndup(const char *s, size_t len)
{
	char *new;

	if (NULL == s) {
		return (char *)NULL;
	}
static void php_pgsql_escape_internal(INTERNAL_FUNCTION_PARAMETERS, int escape_literal) {
	char *from = NULL, *to = NULL, *tmp = NULL;
	zval *pgsql_link = NULL;
	PGconn *pgsql;
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
			size_t field_len = strlen(field);

			if (_php_pgsql_detect_identifier_escape(field, field_len) == SUCCESS) {
				escaped = _php_pgsql_strndup(field, field_len);
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
		escaped = _php_pgsql_strndup(token, len);
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
			escaped = _php_pgsql_strndup(tmp, len);
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
			escaped = _php_pgsql_strndup(tmp, len);
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