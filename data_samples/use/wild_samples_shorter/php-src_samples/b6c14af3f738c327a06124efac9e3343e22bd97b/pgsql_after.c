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

	new = (char *) malloc(len + 1);

	if (NULL == new) {
		return (char *)NULL;
	}

	new[len] = '\0';

	return memmove(new, s, len);
}
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
STD_PHP_INI_BOOLEAN( "pgsql.allow_persistent",      "1",  PHP_INI_SYSTEM, OnUpdateBool, allow_persistent,      zend_pgsql_globals, pgsql_globals)
	char *from = NULL, *to = NULL, *tmp = NULL;
	zval *pgsql_link = NULL;
	PGconn *pgsql;
	int from_len;
	int id = -1;

	switch (ZEND_NUM_ARGS()) {
		/* If field is NULL and HAS DEFAULT, should be skipped */
		if (!skip_field) {
			char *escaped;
			size_t field_len = strlen(field);

			if (_php_pgsql_detect_identifier_escape(field, field_len) == SUCCESS) {
				escaped = _php_pgsql_strndup(field, field_len);
			} else {
#if HAVE_PQESCAPELITERAL
				escaped = PQescapeIdentifier(pg_link, field, field_len);
#else
	token = php_strtok_r(table_copy, ".", &tmp);
	len = strlen(token);
	if (_php_pgsql_detect_identifier_escape(token, len) == SUCCESS) {
		escaped = _php_pgsql_strndup(token, len);
	} else {
#if HAVE_PQESCAPELITERAL
		escaped = PQescapeIdentifier(pg_link, token, len);
#else
		len = strlen(tmp);
		/* "schema"."table" format */
		if (_php_pgsql_detect_identifier_escape(tmp, len) == SUCCESS) {
			escaped = _php_pgsql_strndup(tmp, len);
		} else {
#if HAVE_PQESCAPELITERAL
			escaped = PQescapeIdentifier(pg_link, tmp, len);
#else