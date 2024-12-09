	}

	smart_str_appends(&querystr, 
			"SELECT a.attname, a.attnum, t.typname, a.attlen, a.attnotnull, a.atthasdef, a.attndims, t.typtype = 'e' "
			"FROM pg_class as c, pg_attribute a, pg_type t, pg_namespace n "
			"WHERE a.attnum > 0 AND a.attrelid = c.oid AND c.relname = '");
	tmp_name2 = php_addslashes(tmp_name2, strlen(tmp_name2), &new_len, 0 TSRMLS_CC);
	smart_str_appendl(&querystr, tmp_name2, new_len);
			add_assoc_bool(elem, "has default", 0);
		}
		add_assoc_long(elem, "array dims", atoi(PQgetvalue(pg_result,i,6)));
		if (!strcmp(PQgetvalue(pg_result,i,7), "t")) {
			add_assoc_bool(elem, "is enum", 1);
		}
		else {
			add_assoc_bool(elem, "is enum", 0);
		}
		name = PQgetvalue(pg_result,i,0);
		add_assoc_zval(meta, name, elem);
	}
	PQclear(pg_result);
		zval_dtor(return_value); /* destroy array */
		RETURN_FALSE;
	}
	else {
		HashPosition pos;
		zval **val;

		for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(return_value), &pos);
			zend_hash_get_current_data_ex(Z_ARRVAL_P(return_value), (void **)&val, &pos) == SUCCESS;
			zend_hash_move_forward_ex(Z_ARRVAL_P(return_value), &pos)) {
			/* delete newly added entry, in order to keep BC */
			zend_hash_del_key_or_index(Z_ARRVAL_PP(val), "is enum", sizeof("is enum"), 0, HASH_DEL_KEY);
		}
	}
}
/* }}} */

/* {{{ php_pgsql_get_data_type
 */
	char *field = NULL;
	uint field_len = -1;
	ulong num_idx = -1;
	zval *meta, **def, **type, **not_null, **has_default, **is_enum, **val, *new_val;
	int new_len, key_type, err = 0, skip_field;
	php_pgsql_data_type data_type;
	
	assert(pg_link != NULL);
	assert(Z_TYPE_P(values) == IS_ARRAY);
	assert(Z_TYPE_P(result) == IS_ARRAY);
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'has default'");
			err = 1;
		}
		if (!err && zend_hash_find(Z_ARRVAL_PP(def), "is enum", sizeof("is enum"), (void **)&is_enum) == FAILURE) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Detected broken meta data. Missing 'is enum'");
			err = 1;
		}
		if (!err && (Z_TYPE_PP(val) == IS_ARRAY ||
			 Z_TYPE_PP(val) == IS_OBJECT ||
			 Z_TYPE_PP(val) == IS_CONSTANT_ARRAY)) {
			php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Expects scalar values as field values");
			err = 1;
		}
		if (err) {
			break; /* break out for() */
		}
		ALLOC_INIT_ZVAL(new_val);

		if (Z_BVAL_PP(is_enum)) {
			/* enums need to be treated like strings */
			data_type = PG_TEXT;
		}
		else {
			data_type = php_pgsql_get_data_type(Z_STRVAL_PP(type), Z_STRLEN_PP(type));
		}

		switch(data_type)
		{
			case PG_BOOL:
				switch (Z_TYPE_PP(val)) {
					case IS_STRING: