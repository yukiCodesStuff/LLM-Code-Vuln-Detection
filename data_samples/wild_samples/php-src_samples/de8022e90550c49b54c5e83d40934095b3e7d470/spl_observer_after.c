	if (!php_var_unserialize(&pcount, &p, s + buf_len, &var_hash TSRMLS_CC) || Z_TYPE_P(pcount) != IS_LONG) {
		zval_ptr_dtor(&pcount);
		goto outexcept;
	}

	--p; /* for ';' */
	count = Z_LVAL_P(pcount);
	zval_ptr_dtor(&pcount);
		
	while(count-- > 0) {
		spl_SplObjectStorageElement *pelement;
		char *hash;
		int hash_len;
		
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
		ALLOC_INIT_ZVAL(pinf);
		if (*p == ',') { /* new version has inf */
			++p;
			if (!php_var_unserialize(&pinf, &p, s + buf_len, &var_hash TSRMLS_CC)) {
				zval_ptr_dtor(&pinf);
				goto outexcept;
			}
		}
			if (!php_var_unserialize(&pinf, &p, s + buf_len, &var_hash TSRMLS_CC)) {
				zval_ptr_dtor(&pinf);
				goto outexcept;
			}
		}

		hash = spl_object_storage_get_hash(intern, getThis(), pentry, &hash_len TSRMLS_CC);
		if (!hash) {
			zval_ptr_dtor(&pentry);
			zval_ptr_dtor(&pinf);
			goto outexcept;
		}
		pelement = spl_object_storage_get(intern, hash, hash_len TSRMLS_CC);
		spl_object_storage_free_hash(intern, hash);
		if(pelement) {
			if(pelement->inf) {
				var_push_dtor(&var_hash, &pelement->inf);
			}
			if(pelement->obj) {
				var_push_dtor(&var_hash, &pelement->obj);
			}
		} 
		spl_object_storage_attach(intern, getThis(), pentry, pinf TSRMLS_CC);
		zval_ptr_dtor(&pentry);
		zval_ptr_dtor(&pinf);
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
	if (!intern->std.properties) {
		rebuild_object_properties(&intern->std);
	}
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

ZEND_BEGIN_ARG_INFO(arginfo_Object, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_attach, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, inf)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_Serialized, 0)
	ZEND_ARG_INFO(0, serialized)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_setInfo, 0)
	ZEND_ARG_INFO(0, info)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO(arginfo_getHash, 0)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO();

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetGet, 0, 0, 1)
	ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, object)
	ZEND_ARG_INFO(0, info)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_splobject_void, 0)
ZEND_END_ARG_INFO()

static const zend_function_entry spl_funcs_SplObjectStorage[] = {
	SPL_ME(SplObjectStorage,  attach,      arginfo_attach,        0)
	SPL_ME(SplObjectStorage,  detach,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  contains,    arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  addAll,      arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  removeAll,   arginfo_Object,        0)
	SPL_ME(SplObjectStorage,  getInfo,     arginfo_splobject_void,0)
	SPL_ME(SplObjectStorage,  setInfo,     arginfo_setInfo,       0)
	SPL_ME(SplObjectStorage,  getHash,     arginfo_getHash,       0)
	/* Countable */
	SPL_ME(SplObjectStorage,  count,       arginfo_splobject_void,0)
	/* Iterator */
	SPL_ME(SplObjectStorage,  rewind,      arginfo_splobject_void,0)
	SPL_ME(SplObjectStorage,  valid,       arginfo_splobject_void,0)
	SPL_ME(SplObjectStorage,  key,         arginfo_splobject_void,0)
	SPL_ME(SplObjectStorage,  current,     arginfo_splobject_void,0)
	SPL_ME(SplObjectStorage,  next,        arginfo_splobject_void,0)
	/* Serializable */
	SPL_ME(SplObjectStorage,  unserialize, arginfo_Serialized,    0)
	SPL_ME(SplObjectStorage,  serialize,   arginfo_splobject_void,0)
	/* ArrayAccess */
	SPL_MA(SplObjectStorage, offsetExists, SplObjectStorage, contains, arginfo_offsetGet, 0)
	SPL_MA(SplObjectStorage, offsetSet,    SplObjectStorage, attach,   arginfo_offsetSet, 0)
	SPL_MA(SplObjectStorage, offsetUnset,  SplObjectStorage, detach,   arginfo_offsetGet, 0)
	SPL_ME(SplObjectStorage, offsetGet,    arginfo_offsetGet,     0)
	{NULL, NULL, NULL}
};

typedef enum {
	MIT_NEED_ANY     = 0,
	MIT_NEED_ALL     = 1,
	MIT_KEYS_NUMERIC = 0,
	MIT_KEYS_ASSOC   = 2
} MultipleIteratorFlags;

#define SPL_MULTIPLE_ITERATOR_GET_ALL_CURRENT   1
#define SPL_MULTIPLE_ITERATOR_GET_ALL_KEY       2

/* {{{ proto void MultipleIterator::__construct([int flags = MIT_NEED_ALL|MIT_KEYS_NUMERIC])
   Iterator that iterates over several iterators one after the other */
SPL_METHOD(MultipleIterator, __construct)
{
	spl_SplObjectStorage   *intern;
	long                    flags = MIT_NEED_ALL|MIT_KEYS_NUMERIC;
	zend_error_handling error_handling;

	zend_replace_error_handling(EH_THROW, spl_ce_InvalidArgumentException, &error_handling TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		zend_restore_error_handling(&error_handling TSRMLS_CC);
		return;
	}

	intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	intern->flags = flags;
	zend_restore_error_handling(&error_handling TSRMLS_CC);
}
/* }}} */

/* {{{ proto int MultipleIterator::getFlags()
   Return current flags */
SPL_METHOD(MultipleIterator, getFlags)
{
	spl_SplObjectStorage *intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(intern->flags);
}
/* }}} */

/* {{{ proto int MultipleIterator::setFlags(int flags)
   Set flags */
SPL_METHOD(MultipleIterator, setFlags)
{
	spl_SplObjectStorage *intern;
	intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &intern->flags) == FAILURE) {
		return;
	}
}
/* }}} */

/* {{{ proto void attachIterator(Iterator iterator[, mixed info]) throws InvalidArgumentException
   Attach a new iterator */
SPL_METHOD(MultipleIterator, attachIterator)
{
	spl_SplObjectStorage        *intern;
	zval                        *iterator = NULL, *info = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|z!", &iterator, zend_ce_iterator, &info) == FAILURE) {
		return;
	}

	intern = (spl_SplObjectStorage*)zend_object_store_get_object(getThis() TSRMLS_CC);

	if (info != NULL) {
		spl_SplObjectStorageElement *element;
		zval                         compare_result;

		if (Z_TYPE_P(info) != IS_LONG && Z_TYPE_P(info) != IS_STRING) {
			zend_throw_exception(spl_ce_InvalidArgumentException, "Info must be NULL, integer or string", 0 TSRMLS_CC);
			return;
		}