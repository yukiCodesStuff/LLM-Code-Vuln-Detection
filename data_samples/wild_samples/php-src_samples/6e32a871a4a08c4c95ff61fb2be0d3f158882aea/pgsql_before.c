	if (!tmp_name2 || !*tmp_name2) {
		/* Default schema */
		tmp_name2 = tmp_name;
		tmp_name = "public";
	}

	smart_str_appends(&querystr, 
			"SELECT a.attname, a.attnum, t.typname, a.attlen, a.attnotNULL, a.atthasdef, a.attndims "
			"FROM pg_class as c, pg_attribute a, pg_type t, pg_namespace n "
			"WHERE a.attnum > 0 AND a.attrelid = c.oid AND c.relname = '");
	tmp_name2 = php_addslashes(tmp_name2, strlen(tmp_name2), &new_len, 0 TSRMLS_CC);
	smart_str_appendl(&querystr, tmp_name2, new_len);
	
	smart_str_appends(&querystr, "' AND c.relnamespace = n.oid AND n.nspname = '");
	tmp_name = php_addslashes(tmp_name, strlen(tmp_name), &new_len, 0 TSRMLS_CC);
	smart_str_appendl(&querystr, tmp_name, new_len);

	smart_str_appends(&querystr, "' AND a.atttypid = t.oid ORDER BY a.attnum;");
	smart_str_0(&querystr);
	
	efree(tmp_name2);
	efree(tmp_name);
	efree(src);	
	
	pg_result = PQexec(pg_link, querystr.c);
	if (PQresultStatus(pg_result) != PGRES_TUPLES_OK || (num_rows = PQntuples(pg_result)) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Table '%s' doesn't exists", table_name);
		smart_str_free(&querystr);
		PQclear(pg_result);
		return FAILURE;
	}
		else {
			add_assoc_bool(elem, "has default", 0);
		}
		add_assoc_long(elem, "array dims", atoi(PQgetvalue(pg_result,i,6)));
		name = PQgetvalue(pg_result,i,0);
		add_assoc_zval(meta, name, elem);
	}
	PQclear(pg_result);
	
	return SUCCESS;
}

/* }}} */


/* {{{ proto array pg_meta_data(resource db, string table)
   Get meta_data */
PHP_FUNCTION(pg_meta_data)
{
	zval *pgsql_link;
	char *table_name;
	uint table_name_len;
	PGconn *pgsql;
	int id = -1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
							  &pgsql_link, &table_name, &table_name_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE2(pgsql, PGconn *, &pgsql_link, id, "PostgreSQL link", le_link, le_plink);
	
	array_init(return_value);
	if (php_pgsql_meta_data(pgsql, table_name, return_value TSRMLS_CC) == FAILURE) {
		zval_dtor(return_value); /* destroy array */
		RETURN_FALSE;
	}
} 
/* }}} */

/* {{{ php_pgsql_get_data_type
 */
static php_pgsql_data_type php_pgsql_get_data_type(const char *type_name, size_t len)
{
    /* This is stupid way to do. I'll fix it when I decied how to support
	   user defined types. (Yasuo) */
	
	/* boolean */
	if (!strcmp(type_name, "bool")|| !strcmp(type_name, "boolean"))
		return PG_BOOL;
	/* object id */
	if (!strcmp(type_name, "oid"))
		return PG_OID;
	/* integer */
	if (!strcmp(type_name, "int2") || !strcmp(type_name, "smallint"))
		return PG_INT2;
	if (!strcmp(type_name, "int4") || !strcmp(type_name, "integer"))
		return PG_INT4;
	if (!strcmp(type_name, "int8") || !strcmp(type_name, "bigint"))
		return PG_INT8;
	/* real and other */
	if (!strcmp(type_name, "float4") || !strcmp(type_name, "real"))
		return PG_FLOAT4;
	if (!strcmp(type_name, "float8") || !strcmp(type_name, "double precision"))
		return PG_FLOAT8;
	if (!strcmp(type_name, "numeric"))
		return PG_NUMERIC;
	if (!strcmp(type_name, "money"))
		return PG_MONEY;
	/* character */
	if (!strcmp(type_name, "text"))
		return PG_TEXT;
	if (!strcmp(type_name, "bpchar") || !strcmp(type_name, "character"))
		return PG_CHAR;
	if (!strcmp(type_name, "varchar") || !strcmp(type_name, "character varying"))
		return PG_VARCHAR;
	/* time and interval */
	if (!strcmp(type_name, "abstime"))
		return PG_UNIX_TIME;
	if (!strcmp(type_name, "reltime"))
		return PG_UNIX_TIME_INTERVAL;
	if (!strcmp(type_name, "tinterval"))
		return PG_UNIX_TIME_INTERVAL;
	if (!strcmp(type_name, "date"))
		return PG_DATE;
	if (!strcmp(type_name, "time"))
		return PG_TIME;
	if (!strcmp(type_name, "time with time zone") || !strcmp(type_name, "timetz"))
		return PG_TIME_WITH_TIMEZONE;
	if (!strcmp(type_name, "timestamp without time zone") || !strcmp(type_name, "timestamp"))
		return PG_TIMESTAMP;
	if (!strcmp(type_name, "timestamp with time zone") || !strcmp(type_name, "timestamptz"))
		return PG_TIMESTAMP_WITH_TIMEZONE;
	if (!strcmp(type_name, "interval"))
		return PG_INTERVAL;
	/* binary */
	if (!strcmp(type_name, "bytea"))
		return PG_BYTEA;
	/* network */
	if (!strcmp(type_name, "cidr"))
		return PG_CIDR;
	if (!strcmp(type_name, "inet"))
		return PG_INET;
	if (!strcmp(type_name, "macaddr"))
		return PG_MACADDR;
	/* bit */
	if (!strcmp(type_name, "bit"))
		return PG_BIT;
	if (!strcmp(type_name, "bit varying"))
		return PG_VARBIT;
	/* geometric */
	if (!strcmp(type_name, "line"))
		return PG_LINE;
	if (!strcmp(type_name, "lseg"))
		return PG_LSEG;
	if (!strcmp(type_name, "box"))
		return PG_BOX;
	if (!strcmp(type_name, "path"))
		return PG_PATH;
	if (!strcmp(type_name, "point"))
		return PG_POINT;
	if (!strcmp(type_name, "polygon"))
		return PG_POLYGON;
	if (!strcmp(type_name, "circle"))
		return PG_CIRCLE;
		
	return PG_UNKNOWN;
}
/* }}} */

/* {{{ php_pgsql_convert_match
 * test field value with regular expression specified.  
 */
static int php_pgsql_convert_match(const char *str, const char *regex , int icase TSRMLS_DC)
{
	regex_t re;	
	regmatch_t *subs;
	int regopt = REG_EXTENDED;
	int regerr, ret = SUCCESS;

	if (icase) {
		regopt |= REG_ICASE;
	}
	
	regerr = regcomp(&re, regex, regopt);
	if (regerr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot compile regex");
		regfree(&re);
		return FAILURE;
	}
	subs = (regmatch_t *)ecalloc(sizeof(regmatch_t), re.re_nsub+1);

	regerr = regexec(&re, str, re.re_nsub+1, subs, 0);
	if (regerr == REG_NOMATCH) {
#ifdef PHP_DEBUG		
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "'%s' does not match with '%s'", str, regex);
#endif
		ret = FAILURE;
	}
	else if (regerr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot exec regex");
		ret = FAILURE;
	}
	regfree(&re);
	efree(subs);
	return ret;
}

/* }}} */

/* {{{ php_pgsql_add_quote
 * add quotes around string.
 */
static int php_pgsql_add_quotes(zval *src, zend_bool should_free TSRMLS_DC) 
{
	smart_str str = {0};
	
	assert(Z_TYPE_P(src) == IS_STRING);
	assert(should_free == 1 || should_free == 0);

	smart_str_appendc(&str, '\'');
	smart_str_appendl(&str, Z_STRVAL_P(src), Z_STRLEN_P(src));
	smart_str_appendc(&str, '\'');
	smart_str_0(&str);
	
	if (should_free) {
		efree(Z_STRVAL_P(src));
	}
	Z_STRVAL_P(src) = str.c;
	Z_STRLEN_P(src) = str.len;

	return SUCCESS;
}
/* }}} */

#define PGSQL_CONV_CHECK_IGNORE() \
				if (!err && Z_TYPE_P(new_val) == IS_STRING && !strcmp(Z_STRVAL_P(new_val), "NULL")) { \
					/* if new_value is string "NULL" and field has default value, remove element to use default value */ \
					if (!(opt & PGSQL_CONV_IGNORE_DEFAULT) && Z_BVAL_PP(has_default)) { \
						zval_dtor(new_val); \
						FREE_ZVAL(new_val); \
						skip_field = 1; \
					} \
					/* raise error if it's not null and cannot be ignored */ \
					else if (!(opt & PGSQL_CONV_IGNORE_NOT_NULL) && Z_BVAL_PP(not_null)) { \
						php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected NULL for 'NOT NULL' field '%s'", field ); \
						err = 1; \
					} \
				} 		

/* {{{ php_pgsql_convert
 * check and convert array values (fieldname=>vlaue pair) for sql
 */
PHP_PGSQL_API int php_pgsql_convert(PGconn *pg_link, const char *table_name, const zval *values, zval *result, ulong opt TSRMLS_DC) 
{
	HashPosition pos;
	char *field = NULL;
	uint field_len = -1;
	ulong num_idx = -1;
	zval *meta, **def, **type, **not_null, **has_default, **val, *new_val;
	int new_len, key_type, err = 0, skip_field;
	
	assert(pg_link != NULL);
	assert(Z_TYPE_P(values) == IS_ARRAY);
	assert(Z_TYPE_P(result) == IS_ARRAY);
	assert(!(opt & ~PGSQL_CONV_OPTS));

	if (!table_name) {
		return FAILURE;
	}
	MAKE_STD_ZVAL(meta);
	array_init(meta);
	if (php_pgsql_meta_data(pg_link, table_name, meta TSRMLS_CC) == FAILURE) {
		zval_dtor(meta);
		FREE_ZVAL(meta);
		return FAILURE;
	}
	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos);
		 zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&val, &pos) == SUCCESS;
		 zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos)) {
		skip_field = 0;
		new_val = NULL;
		
		if ((key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(values), &field, &field_len, &num_idx, 0, &pos)) == HASH_KEY_NON_EXISTANT) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to get array key type");
			err = 1;
		}
		if (!err && key_type == HASH_KEY_IS_LONG) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Accepts only string key for values");
			err = 1;
		}
		if (!err && key_type == HASH_KEY_NON_EXISTANT) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Accepts only string key for values");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_P(meta), field, field_len, (void **)&def) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Invalid field name (%s) in values", field);
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "type", sizeof("type"), (void **)&type) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'type'");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "not null", sizeof("not null"), (void **)&not_null) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'not null'");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "has default", sizeof("has default"), (void **)&has_default) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'has default'");
			err = 1;
		}
		if (!err && (Z_TYPE_PP(val) == IS_ARRAY ||
			 Z_TYPE_PP(val) == IS_OBJECT ||
			 Z_TYPE_PP(val) == IS_CONSTANT_ARRAY)) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects scaler values as field values");
			err = 1;
		}
		if (err) {
			break; /* break out for() */
		}
		ALLOC_INIT_ZVAL(new_val);
		switch(php_pgsql_get_data_type(Z_STRVAL_PP(type), Z_STRLEN_PP(type)))
		{
			case PG_BOOL:
				switch (Z_TYPE_PP(val)) {
					case IS_STRING:
						if (Z_STRLEN_PP(val) == 0) {
							ZVAL_STRING(new_val, "NULL", 1);
						}
	if (php_pgsql_meta_data(pgsql, table_name, return_value TSRMLS_CC) == FAILURE) {
		zval_dtor(return_value); /* destroy array */
		RETURN_FALSE;
	}
} 
/* }}} */

/* {{{ php_pgsql_get_data_type
 */
static php_pgsql_data_type php_pgsql_get_data_type(const char *type_name, size_t len)
{
    /* This is stupid way to do. I'll fix it when I decied how to support
	   user defined types. (Yasuo) */
	
	/* boolean */
	if (!strcmp(type_name, "bool")|| !strcmp(type_name, "boolean"))
		return PG_BOOL;
	/* object id */
	if (!strcmp(type_name, "oid"))
		return PG_OID;
	/* integer */
	if (!strcmp(type_name, "int2") || !strcmp(type_name, "smallint"))
		return PG_INT2;
	if (!strcmp(type_name, "int4") || !strcmp(type_name, "integer"))
		return PG_INT4;
	if (!strcmp(type_name, "int8") || !strcmp(type_name, "bigint"))
		return PG_INT8;
	/* real and other */
	if (!strcmp(type_name, "float4") || !strcmp(type_name, "real"))
		return PG_FLOAT4;
	if (!strcmp(type_name, "float8") || !strcmp(type_name, "double precision"))
		return PG_FLOAT8;
	if (!strcmp(type_name, "numeric"))
		return PG_NUMERIC;
	if (!strcmp(type_name, "money"))
		return PG_MONEY;
	/* character */
	if (!strcmp(type_name, "text"))
		return PG_TEXT;
	if (!strcmp(type_name, "bpchar") || !strcmp(type_name, "character"))
		return PG_CHAR;
	if (!strcmp(type_name, "varchar") || !strcmp(type_name, "character varying"))
		return PG_VARCHAR;
	/* time and interval */
	if (!strcmp(type_name, "abstime"))
		return PG_UNIX_TIME;
	if (!strcmp(type_name, "reltime"))
		return PG_UNIX_TIME_INTERVAL;
	if (!strcmp(type_name, "tinterval"))
		return PG_UNIX_TIME_INTERVAL;
	if (!strcmp(type_name, "date"))
		return PG_DATE;
	if (!strcmp(type_name, "time"))
		return PG_TIME;
	if (!strcmp(type_name, "time with time zone") || !strcmp(type_name, "timetz"))
		return PG_TIME_WITH_TIMEZONE;
	if (!strcmp(type_name, "timestamp without time zone") || !strcmp(type_name, "timestamp"))
		return PG_TIMESTAMP;
	if (!strcmp(type_name, "timestamp with time zone") || !strcmp(type_name, "timestamptz"))
		return PG_TIMESTAMP_WITH_TIMEZONE;
	if (!strcmp(type_name, "interval"))
		return PG_INTERVAL;
	/* binary */
	if (!strcmp(type_name, "bytea"))
		return PG_BYTEA;
	/* network */
	if (!strcmp(type_name, "cidr"))
		return PG_CIDR;
	if (!strcmp(type_name, "inet"))
		return PG_INET;
	if (!strcmp(type_name, "macaddr"))
		return PG_MACADDR;
	/* bit */
	if (!strcmp(type_name, "bit"))
		return PG_BIT;
	if (!strcmp(type_name, "bit varying"))
		return PG_VARBIT;
	/* geometric */
	if (!strcmp(type_name, "line"))
		return PG_LINE;
	if (!strcmp(type_name, "lseg"))
		return PG_LSEG;
	if (!strcmp(type_name, "box"))
		return PG_BOX;
	if (!strcmp(type_name, "path"))
		return PG_PATH;
	if (!strcmp(type_name, "point"))
		return PG_POINT;
	if (!strcmp(type_name, "polygon"))
		return PG_POLYGON;
	if (!strcmp(type_name, "circle"))
		return PG_CIRCLE;
		
	return PG_UNKNOWN;
}
/* }}} */

/* {{{ php_pgsql_convert_match
 * test field value with regular expression specified.  
 */
static int php_pgsql_convert_match(const char *str, const char *regex , int icase TSRMLS_DC)
{
	regex_t re;	
	regmatch_t *subs;
	int regopt = REG_EXTENDED;
	int regerr, ret = SUCCESS;

	if (icase) {
		regopt |= REG_ICASE;
	}
	
	regerr = regcomp(&re, regex, regopt);
	if (regerr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot compile regex");
		regfree(&re);
		return FAILURE;
	}
	subs = (regmatch_t *)ecalloc(sizeof(regmatch_t), re.re_nsub+1);

	regerr = regexec(&re, str, re.re_nsub+1, subs, 0);
	if (regerr == REG_NOMATCH) {
#ifdef PHP_DEBUG		
		php_error_docref(NULL TSRMLS_CC, E_NOTICE, "'%s' does not match with '%s'", str, regex);
#endif
		ret = FAILURE;
	}
	else if (regerr) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Cannot exec regex");
		ret = FAILURE;
	}
	regfree(&re);
	efree(subs);
	return ret;
}

/* }}} */

/* {{{ php_pgsql_add_quote
 * add quotes around string.
 */
static int php_pgsql_add_quotes(zval *src, zend_bool should_free TSRMLS_DC) 
{
	smart_str str = {0};
	
	assert(Z_TYPE_P(src) == IS_STRING);
	assert(should_free == 1 || should_free == 0);

	smart_str_appendc(&str, '\'');
	smart_str_appendl(&str, Z_STRVAL_P(src), Z_STRLEN_P(src));
	smart_str_appendc(&str, '\'');
	smart_str_0(&str);
	
	if (should_free) {
		efree(Z_STRVAL_P(src));
	}
	Z_STRVAL_P(src) = str.c;
	Z_STRLEN_P(src) = str.len;

	return SUCCESS;
}
/* }}} */

#define PGSQL_CONV_CHECK_IGNORE() \
				if (!err && Z_TYPE_P(new_val) == IS_STRING && !strcmp(Z_STRVAL_P(new_val), "NULL")) { \
					/* if new_value is string "NULL" and field has default value, remove element to use default value */ \
					if (!(opt & PGSQL_CONV_IGNORE_DEFAULT) && Z_BVAL_PP(has_default)) { \
						zval_dtor(new_val); \
						FREE_ZVAL(new_val); \
						skip_field = 1; \
					} \
					/* raise error if it's not null and cannot be ignored */ \
					else if (!(opt & PGSQL_CONV_IGNORE_NOT_NULL) && Z_BVAL_PP(not_null)) { \
						php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected NULL for 'NOT NULL' field '%s'", field ); \
						err = 1; \
					} \
				} 		

/* {{{ php_pgsql_convert
 * check and convert array values (fieldname=>vlaue pair) for sql
 */
PHP_PGSQL_API int php_pgsql_convert(PGconn *pg_link, const char *table_name, const zval *values, zval *result, ulong opt TSRMLS_DC) 
{
	HashPosition pos;
	char *field = NULL;
	uint field_len = -1;
	ulong num_idx = -1;
	zval *meta, **def, **type, **not_null, **has_default, **val, *new_val;
	int new_len, key_type, err = 0, skip_field;
	
	assert(pg_link != NULL);
	assert(Z_TYPE_P(values) == IS_ARRAY);
	assert(Z_TYPE_P(result) == IS_ARRAY);
	assert(!(opt & ~PGSQL_CONV_OPTS));

	if (!table_name) {
		return FAILURE;
	}
	MAKE_STD_ZVAL(meta);
	array_init(meta);
	if (php_pgsql_meta_data(pg_link, table_name, meta TSRMLS_CC) == FAILURE) {
		zval_dtor(meta);
		FREE_ZVAL(meta);
		return FAILURE;
	}
	for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(values), &pos);
		 zend_hash_get_current_data_ex(Z_ARRVAL_P(values), (void **)&val, &pos) == SUCCESS;
		 zend_hash_move_forward_ex(Z_ARRVAL_P(values), &pos)) {
		skip_field = 0;
		new_val = NULL;
		
		if ((key_type = zend_hash_get_current_key_ex(Z_ARRVAL_P(values), &field, &field_len, &num_idx, 0, &pos)) == HASH_KEY_NON_EXISTANT) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to get array key type");
			err = 1;
		}
		if (!err && key_type == HASH_KEY_IS_LONG) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Accepts only string key for values");
			err = 1;
		}
		if (!err && key_type == HASH_KEY_NON_EXISTANT) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Accepts only string key for values");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_P(meta), field, field_len, (void **)&def) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Invalid field name (%s) in values", field);
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "type", sizeof("type"), (void **)&type) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'type'");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "not null", sizeof("not null"), (void **)&not_null) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'not null'");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "has default", sizeof("has default"), (void **)&has_default) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'has default'");
			err = 1;
		}
		if (!err && (Z_TYPE_PP(val) == IS_ARRAY ||
			 Z_TYPE_PP(val) == IS_OBJECT ||
			 Z_TYPE_PP(val) == IS_CONSTANT_ARRAY)) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects scaler values as field values");
			err = 1;
		}
		if (err) {
			break; /* break out for() */
		}
		ALLOC_INIT_ZVAL(new_val);
		switch(php_pgsql_get_data_type(Z_STRVAL_PP(type), Z_STRLEN_PP(type)))
		{
			case PG_BOOL:
				switch (Z_TYPE_PP(val)) {
					case IS_STRING:
						if (Z_STRLEN_PP(val) == 0) {
							ZVAL_STRING(new_val, "NULL", 1);
						}
						else {
							if (!strcmp(Z_STRVAL_PP(val), "t") || !strcmp(Z_STRVAL_PP(val), "T") ||
								!strcmp(Z_STRVAL_PP(val), "y") || !strcmp(Z_STRVAL_PP(val), "Y") ||
								!strcmp(Z_STRVAL_PP(val), "true") || !strcmp(Z_STRVAL_PP(val), "True") ||
								!strcmp(Z_STRVAL_PP(val), "yes") || !strcmp(Z_STRVAL_PP(val), "Yes") ||
								!strcmp(Z_STRVAL_PP(val), "1")) {
								ZVAL_STRING(new_val, "'t'", 1);
							}
							else if (!strcmp(Z_STRVAL_PP(val), "f") || !strcmp(Z_STRVAL_PP(val), "F") ||
									 !strcmp(Z_STRVAL_PP(val), "n") || !strcmp(Z_STRVAL_PP(val), "N") ||
									 !strcmp(Z_STRVAL_PP(val), "false") ||  !strcmp(Z_STRVAL_PP(val), "False") ||
									 !strcmp(Z_STRVAL_PP(val), "no") ||  !strcmp(Z_STRVAL_PP(val), "No") ||
									 !strcmp(Z_STRVAL_PP(val), "0")) {
								ZVAL_STRING(new_val, "'f'", 1);
							}
							else {
								php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected invalid value (%s) for PostgreSQL %s field (%s)", Z_STRVAL_PP(val), Z_STRVAL_PP(type), field);
								err = 1;
							}
						}
						break;
						
					case IS_LONG:
					case IS_BOOL:
						if (Z_LVAL_PP(val)) {
							ZVAL_STRING(new_val, "'t'", 1);
						}
						else {
							ZVAL_STRING(new_val, "'f'", 1);
						}
						break;

					case IS_NULL:
						ZVAL_STRING(new_val, "NULL", 1);
						break;

					default:
						err = 1;
				}
{
	HashPosition pos;
	char *field = NULL;
	uint field_len = -1;
	ulong num_idx = -1;
	zval *meta, **def, **type, **not_null, **has_default, **val, *new_val;
	int new_len, key_type, err = 0, skip_field;
	
	assert(pg_link != NULL);
	assert(Z_TYPE_P(values) == IS_ARRAY);
	assert(Z_TYPE_P(result) == IS_ARRAY);
	assert(!(opt & ~PGSQL_CONV_OPTS));

	if (!table_name) {
		return FAILURE;
	}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "has default", sizeof("has default"), (void **)&has_default) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'has default'");
			err = 1;
		}
		if (!err && (Z_TYPE_PP(val) == IS_ARRAY ||
			 Z_TYPE_PP(val) == IS_OBJECT ||
			 Z_TYPE_PP(val) == IS_CONSTANT_ARRAY)) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects scaler values as field values");
			err = 1;
		}
		if (err) {
			break; /* break out for() */
		}
		ALLOC_INIT_ZVAL(new_val);
		switch(php_pgsql_get_data_type(Z_STRVAL_PP(type), Z_STRLEN_PP(type)))
		{
			case PG_BOOL:
				switch (Z_TYPE_PP(val)) {
					case IS_STRING:
						if (Z_STRLEN_PP(val) == 0) {
							ZVAL_STRING(new_val, "NULL", 1);
						}
						else {
							if (!strcmp(Z_STRVAL_PP(val), "t") || !strcmp(Z_STRVAL_PP(val), "T") ||
								!strcmp(Z_STRVAL_PP(val), "y") || !strcmp(Z_STRVAL_PP(val), "Y") ||
								!strcmp(Z_STRVAL_PP(val), "true") || !strcmp(Z_STRVAL_PP(val), "True") ||
								!strcmp(Z_STRVAL_PP(val), "yes") || !strcmp(Z_STRVAL_PP(val), "Yes") ||
								!strcmp(Z_STRVAL_PP(val), "1")) {
								ZVAL_STRING(new_val, "'t'", 1);
							}
							else if (!strcmp(Z_STRVAL_PP(val), "f") || !strcmp(Z_STRVAL_PP(val), "F") ||
									 !strcmp(Z_STRVAL_PP(val), "n") || !strcmp(Z_STRVAL_PP(val), "N") ||
									 !strcmp(Z_STRVAL_PP(val), "false") ||  !strcmp(Z_STRVAL_PP(val), "False") ||
									 !strcmp(Z_STRVAL_PP(val), "no") ||  !strcmp(Z_STRVAL_PP(val), "No") ||
									 !strcmp(Z_STRVAL_PP(val), "0")) {
								ZVAL_STRING(new_val, "'f'", 1);
							}
							else {
								php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected invalid value (%s) for PostgreSQL %s field (%s)", Z_STRVAL_PP(val), Z_STRVAL_PP(type), field);
								err = 1;
							}
						}
						break;
						
					case IS_LONG:
					case IS_BOOL:
						if (Z_LVAL_PP(val)) {
							ZVAL_STRING(new_val, "'t'", 1);
						}
						else {
							ZVAL_STRING(new_val, "'f'", 1);
						}
						break;

					case IS_NULL:
						ZVAL_STRING(new_val, "NULL", 1);
						break;

					default:
						err = 1;
				}