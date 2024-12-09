	{
		zend_object_value zvalue;
		memset(&zvalue, 0, sizeof(zend_object_value));
		zvalue.handle = Z_OBJ_HANDLE_P(obj);
		zvalue.handlers = Z_OBJ_HT_P(obj);
		zend_hash_del(&intern->storage, (char*)&zvalue, sizeof(zend_object_value));
	}
#endif

	zend_hash_internal_pointer_reset_ex(&intern->storage, &intern->pos);
	intern->index = 0;
} /* }}} */

int spl_object_storage_contains(spl_SplObjectStorage *intern, zval *obj TSRMLS_DC) /* {{{ */
{
#if HAVE_PACKED_OBJECT_VALUE
	return zend_hash_exists(&intern->storage, (char*)&Z_OBJVAL_P(obj), sizeof(zend_object_value));
#else
	{
		zend_object_value zvalue;
		memset(&zvalue, 0, sizeof(zend_object_value));
		zvalue.handle = Z_OBJ_HANDLE_P(obj);
		zvalue.handlers = Z_OBJ_HT_P(obj);
		return zend_hash_exists(&intern->storage, (char*)&zvalue, sizeof(zend_object_value));
	}
#endif
} /* }}} */

/* {{{ proto bool SplObjectStorage::contains($obj)
 Determine whethe an object is contained in the storage */
SPL_METHOD(SplObjectStorage, contains)
{
	zval *obj;
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &obj) == FAILURE) {
		return;
	}

	RETURN_BOOL(spl_object_storage_contains(intern, obj TSRMLS_CC));
} /* }}} */

/* {{{ proto int SplObjectStorage::count()
 Determine number of objects in storage */
SPL_METHOD(SplObjectStorage, count)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_LONG(zend_hash_num_elements(&intern->storage));
} /* }}} */

/* {{{ proto void SplObjectStorage::rewind()
 */
SPL_METHOD(SplObjectStorage, rewind)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	zend_hash_internal_pointer_reset_ex(&intern->storage, &intern->pos);
	intern->index = 0;
} /* }}} */

/* {{{ proto bool SplObjectStorage::valid()
 */
SPL_METHOD(SplObjectStorage, valid)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_BOOL(zend_hash_has_more_elements_ex(&intern->storage, &intern->pos) == SUCCESS);
} /* }}} */

/* {{{ proto mixed SplObjectStorage::key()
 */
SPL_METHOD(SplObjectStorage, key)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_LONG(intern->index);
} /* }}} */

/* {{{ proto mixed SplObjectStorage::current()
 */
SPL_METHOD(SplObjectStorage, current)
{
	zval **entry;
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_hash_get_current_data_ex(&intern->storage, (void**)&entry, &intern->pos) == FAILURE) {
		return;
	}
	RETVAL_ZVAL(*entry, 1, 0);
} /* }}} */

/* {{{ proto void SplObjectStorage::next()
 */
SPL_METHOD(SplObjectStorage, next)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	zend_hash_move_forward_ex(&intern->storage, &intern->pos);
	intern->index++;
} /* }}} */

/* {{{ proto string SplObjectStorage::serialize()
 */
SPL_METHOD(SplObjectStorage, serialize)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	zval **entry, members, *pmembers;
	HashPosition      pos;
	php_serialize_data_t var_hash;
	smart_str buf = {0};

	PHP_VAR_SERIALIZE_INIT(var_hash);

	/* storage */
	smart_str_appendl(&buf, "x:i:", 4);
	smart_str_append_long(&buf, zend_hash_num_elements(&intern->storage));
	smart_str_appendc(&buf, ';');

	zend_hash_internal_pointer_reset_ex(&intern->storage, &pos);

	while(zend_hash_has_more_elements_ex(&intern->storage, &pos) == SUCCESS) {
		if (zend_hash_get_current_data_ex(&intern->storage, (void**)&entry, &pos) == FAILURE) {
			smart_str_free(&buf);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);
			RETURN_NULL();
		}
		php_var_serialize(&buf, entry, &var_hash TSRMLS_CC);
		smart_str_appendc(&buf, ';');
		zend_hash_move_forward_ex(&intern->storage, &pos);
	}

	/* members */
	smart_str_appendl(&buf, "m:", 2);
	INIT_PZVAL(&members);
	Z_ARRVAL(members) = intern->std.properties;
	Z_TYPE(members) = IS_ARRAY;
	pmembers = &members;
	php_var_serialize(&buf, &pmembers, &var_hash TSRMLS_CC); /* finishes the string */

	/* done */
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	} else {
		RETURN_NULL();
	}
	
} /* }}} */

/* {{{ proto void SplObjectStorage::unserialize(string serialized)
 */
SPL_METHOD(SplObjectStorage, unserialize)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	char *buf;
	int buf_len;
	const unsigned char *p, *s;
	php_unserialize_data_t var_hash;
	zval *pentry, *pmembers, *pcount = NULL;
	long count;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf, &buf_len) == FAILURE) {
		return;
	}

	if (buf_len == 0) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Empty serialized string cannot be empty");
		return;
	}

	/* storage */
	s = p = (const unsigned char*)buf;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	if (*p!= 'x' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pcount);
	if (!php_var_unserialize(&pcount, &p, s + buf_len, NULL TSRMLS_CC) || Z_TYPE_P(pcount) != IS_LONG) {
		zval_ptr_dtor(&pcount);
		goto outexcept;
	}

	--p; /* for ';' */
	count = Z_LVAL_P(pcount);
	zval_ptr_dtor(&pcount);
		
	while(count-- > 0) {
		if (*p != ';') {
			goto outexcept;
		}
		++p;
		if(*p != 'O' && *p != 'C' && *p != 'r') {
			goto outexcept;
		}
		ALLOC_INIT_ZVAL(pentry);
		if (!php_var_unserialize(&pentry, &p, s + buf_len, &var_hash TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(Z_TYPE_P(pentry) != IS_OBJECT) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(spl_object_storage_contains(intern, pentry TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			continue;
		}
		spl_object_storage_attach(intern, pentry TSRMLS_CC);
		zval_ptr_dtor(&pentry);
	}

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pmembers);
	if (!php_var_unserialize(&pmembers, &p, s + buf_len, &var_hash TSRMLS_CC)) {
		zval_ptr_dtor(&pmembers);
		goto outexcept;
	}

	/* copy members */
	zend_hash_copy(intern->std.properties, Z_ARRVAL_P(pmembers), (copy_ctor_func_t) zval_add_ref, (void *) NULL, sizeof(zval *));
	zval_ptr_dtor(&pmembers);

	/* done reading $serialized */

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

outexcept:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Error at offset %ld of %d bytes", (long)((char*)p - buf), buf_len);
	return;

} /* }}} */

static
ZEND_BEGIN_ARG_INFO(arginfo_Object, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_Serialized, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

static zend_function_entry spl_funcs_SplObjectStorage[] = {
	SPL_ME(SplObjectStorage,  attach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  detach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  contains,    arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  count,       NULL,                  0)
	SPL_ME(SplObjectStorage,  rewind,      NULL,                  0)
	SPL_ME(SplObjectStorage,  valid,       NULL,                  0)
	SPL_ME(SplObjectStorage,  key,         NULL,                  0)
	SPL_ME(SplObjectStorage,  current,     NULL,                  0)
	SPL_ME(SplObjectStorage,  next,        NULL,                  0)
	SPL_ME(SplObjectStorage,  unserialize, arginfo_Serialized,    0)
	SPL_ME(SplObjectStorage,  serialize,   NULL,                  0)
	{NULL, NULL, NULL}
};

/* {{{ PHP_MINIT_FUNCTION(spl_observer) */
PHP_MINIT_FUNCTION(spl_observer)
{
	REGISTER_SPL_INTERFACE(SplObserver);
	REGISTER_SPL_INTERFACE(SplSubject);

	REGISTER_SPL_STD_CLASS_EX(SplObjectStorage, spl_SplObjectStorage_new, spl_funcs_SplObjectStorage);
	memcpy(&spl_handler_SplObjectStorage, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Countable);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Iterator);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Serializable);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &obj) == FAILURE) {
		return;
	}

	RETURN_BOOL(spl_object_storage_contains(intern, obj TSRMLS_CC));
} /* }}} */

/* {{{ proto int SplObjectStorage::count()
 Determine number of objects in storage */
SPL_METHOD(SplObjectStorage, count)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_LONG(zend_hash_num_elements(&intern->storage));
} /* }}} */

/* {{{ proto void SplObjectStorage::rewind()
 */
SPL_METHOD(SplObjectStorage, rewind)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	zend_hash_internal_pointer_reset_ex(&intern->storage, &intern->pos);
	intern->index = 0;
} /* }}} */

/* {{{ proto bool SplObjectStorage::valid()
 */
SPL_METHOD(SplObjectStorage, valid)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_BOOL(zend_hash_has_more_elements_ex(&intern->storage, &intern->pos) == SUCCESS);
} /* }}} */

/* {{{ proto mixed SplObjectStorage::key()
 */
SPL_METHOD(SplObjectStorage, key)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	RETURN_LONG(intern->index);
} /* }}} */

/* {{{ proto mixed SplObjectStorage::current()
 */
SPL_METHOD(SplObjectStorage, current)
{
	zval **entry;
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	if (zend_hash_get_current_data_ex(&intern->storage, (void**)&entry, &intern->pos) == FAILURE) {
		return;
	}
	RETVAL_ZVAL(*entry, 1, 0);
} /* }}} */

/* {{{ proto void SplObjectStorage::next()
 */
SPL_METHOD(SplObjectStorage, next)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	
	zend_hash_move_forward_ex(&intern->storage, &intern->pos);
	intern->index++;
} /* }}} */

/* {{{ proto string SplObjectStorage::serialize()
 */
SPL_METHOD(SplObjectStorage, serialize)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	zval **entry, members, *pmembers;
	HashPosition      pos;
	php_serialize_data_t var_hash;
	smart_str buf = {0};

	PHP_VAR_SERIALIZE_INIT(var_hash);

	/* storage */
	smart_str_appendl(&buf, "x:i:", 4);
	smart_str_append_long(&buf, zend_hash_num_elements(&intern->storage));
	smart_str_appendc(&buf, ';');

	zend_hash_internal_pointer_reset_ex(&intern->storage, &pos);

	while(zend_hash_has_more_elements_ex(&intern->storage, &pos) == SUCCESS) {
		if (zend_hash_get_current_data_ex(&intern->storage, (void**)&entry, &pos) == FAILURE) {
			smart_str_free(&buf);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);
			RETURN_NULL();
		}
		php_var_serialize(&buf, entry, &var_hash TSRMLS_CC);
		smart_str_appendc(&buf, ';');
		zend_hash_move_forward_ex(&intern->storage, &pos);
	}

	/* members */
	smart_str_appendl(&buf, "m:", 2);
	INIT_PZVAL(&members);
	Z_ARRVAL(members) = intern->std.properties;
	Z_TYPE(members) = IS_ARRAY;
	pmembers = &members;
	php_var_serialize(&buf, &pmembers, &var_hash TSRMLS_CC); /* finishes the string */

	/* done */
	PHP_VAR_SERIALIZE_DESTROY(var_hash);

	if (buf.c) {
		RETURN_STRINGL(buf.c, buf.len, 0);
	} else {
		RETURN_NULL();
	}
	
} /* }}} */

/* {{{ proto void SplObjectStorage::unserialize(string serialized)
 */
SPL_METHOD(SplObjectStorage, unserialize)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	char *buf;
	int buf_len;
	const unsigned char *p, *s;
	php_unserialize_data_t var_hash;
	zval *pentry, *pmembers, *pcount = NULL;
	long count;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &buf, &buf_len) == FAILURE) {
		return;
	}

	if (buf_len == 0) {
		zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Empty serialized string cannot be empty");
		return;
	}

	/* storage */
	s = p = (const unsigned char*)buf;
	PHP_VAR_UNSERIALIZE_INIT(var_hash);

	if (*p!= 'x' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pcount);
	if (!php_var_unserialize(&pcount, &p, s + buf_len, NULL TSRMLS_CC) || Z_TYPE_P(pcount) != IS_LONG) {
		zval_ptr_dtor(&pcount);
		goto outexcept;
	}

	--p; /* for ';' */
	count = Z_LVAL_P(pcount);
	zval_ptr_dtor(&pcount);
		
	while(count-- > 0) {
		if (*p != ';') {
			goto outexcept;
		}
		++p;
		if(*p != 'O' && *p != 'C' && *p != 'r') {
			goto outexcept;
		}
		ALLOC_INIT_ZVAL(pentry);
		if (!php_var_unserialize(&pentry, &p, s + buf_len, &var_hash TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(Z_TYPE_P(pentry) != IS_OBJECT) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(spl_object_storage_contains(intern, pentry TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			continue;
		}
		spl_object_storage_attach(intern, pentry TSRMLS_CC);
		zval_ptr_dtor(&pentry);
	}

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pmembers);
	if (!php_var_unserialize(&pmembers, &p, s + buf_len, &var_hash TSRMLS_CC)) {
		zval_ptr_dtor(&pmembers);
		goto outexcept;
	}

	/* copy members */
	zend_hash_copy(intern->std.properties, Z_ARRVAL_P(pmembers), (copy_ctor_func_t) zval_add_ref, (void *) NULL, sizeof(zval *));
	zval_ptr_dtor(&pmembers);

	/* done reading $serialized */

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

outexcept:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Error at offset %ld of %d bytes", (long)((char*)p - buf), buf_len);
	return;

} /* }}} */

static
ZEND_BEGIN_ARG_INFO(arginfo_Object, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_Serialized, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

static zend_function_entry spl_funcs_SplObjectStorage[] = {
	SPL_ME(SplObjectStorage,  attach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  detach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  contains,    arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  count,       NULL,                  0)
	SPL_ME(SplObjectStorage,  rewind,      NULL,                  0)
	SPL_ME(SplObjectStorage,  valid,       NULL,                  0)
	SPL_ME(SplObjectStorage,  key,         NULL,                  0)
	SPL_ME(SplObjectStorage,  current,     NULL,                  0)
	SPL_ME(SplObjectStorage,  next,        NULL,                  0)
	SPL_ME(SplObjectStorage,  unserialize, arginfo_Serialized,    0)
	SPL_ME(SplObjectStorage,  serialize,   NULL,                  0)
	{NULL, NULL, NULL}
};

/* {{{ PHP_MINIT_FUNCTION(spl_observer) */
PHP_MINIT_FUNCTION(spl_observer)
{
	REGISTER_SPL_INTERFACE(SplObserver);
	REGISTER_SPL_INTERFACE(SplSubject);

	REGISTER_SPL_STD_CLASS_EX(SplObjectStorage, spl_SplObjectStorage_new, spl_funcs_SplObjectStorage);
	memcpy(&spl_handler_SplObjectStorage, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Countable);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Iterator);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Serializable);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
		if (*p != ';') {
			goto outexcept;
		}
		++p;
		if(*p != 'O' && *p != 'C' && *p != 'r') {
			goto outexcept;
		}
		ALLOC_INIT_ZVAL(pentry);
		if (!php_var_unserialize(&pentry, &p, s + buf_len, &var_hash TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(Z_TYPE_P(pentry) != IS_OBJECT) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		if(spl_object_storage_contains(intern, pentry TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			continue;
		}
		spl_object_storage_attach(intern, pentry TSRMLS_CC);
		zval_ptr_dtor(&pentry);
	}

	if (*p != ';') {
		goto outexcept;
	}
	++p;

	/* members */
	if (*p!= 'm' || *++p != ':') {
		goto outexcept;
	}
	++p;

	ALLOC_INIT_ZVAL(pmembers);
	if (!php_var_unserialize(&pmembers, &p, s + buf_len, &var_hash TSRMLS_CC)) {
		zval_ptr_dtor(&pmembers);
		goto outexcept;
	}

	/* copy members */
	zend_hash_copy(intern->std.properties, Z_ARRVAL_P(pmembers), (copy_ctor_func_t) zval_add_ref, (void *) NULL, sizeof(zval *));
	zval_ptr_dtor(&pmembers);

	/* done reading $serialized */

	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return;

outexcept:
	PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	zend_throw_exception_ex(spl_ce_UnexpectedValueException, 0 TSRMLS_CC, "Error at offset %ld of %d bytes", (long)((char*)p - buf), buf_len);
	return;

} /* }}} */

static
ZEND_BEGIN_ARG_INFO(arginfo_Object, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

static
ZEND_BEGIN_ARG_INFO(arginfo_Serialized, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

static zend_function_entry spl_funcs_SplObjectStorage[] = {
	SPL_ME(SplObjectStorage,  attach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  detach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  contains,    arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  count,       NULL,                  0)
	SPL_ME(SplObjectStorage,  rewind,      NULL,                  0)
	SPL_ME(SplObjectStorage,  valid,       NULL,                  0)
	SPL_ME(SplObjectStorage,  key,         NULL,                  0)
	SPL_ME(SplObjectStorage,  current,     NULL,                  0)
	SPL_ME(SplObjectStorage,  next,        NULL,                  0)
	SPL_ME(SplObjectStorage,  unserialize, arginfo_Serialized,    0)
	SPL_ME(SplObjectStorage,  serialize,   NULL,                  0)
	{NULL, NULL, NULL}
};

/* {{{ PHP_MINIT_FUNCTION(spl_observer) */
PHP_MINIT_FUNCTION(spl_observer)
{
	REGISTER_SPL_INTERFACE(SplObserver);
	REGISTER_SPL_INTERFACE(SplSubject);

	REGISTER_SPL_STD_CLASS_EX(SplObjectStorage, spl_SplObjectStorage_new, spl_funcs_SplObjectStorage);
	memcpy(&spl_handler_SplObjectStorage, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Countable);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Iterator);
	REGISTER_SPL_IMPLEMENTS(SplObjectStorage, Serializable);

	return SUCCESS;
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */