	intern->index = 0;
} /* }}} */

/* {{{ proto bool SplObjectStorage::contains($obj)
 Determine whethe an object is contained in the storage */
SPL_METHOD(SplObjectStorage, contains)
{
		return;
	}

#if HAVE_PACKED_OBJECT_VALUE
	RETURN_BOOL(zend_hash_exists(&intern->storage, (char*)&Z_OBJVAL_P(obj), sizeof(zend_object_value)));
#else
	{
		zend_object_value zvalue;
		memset(&zvalue, 0, sizeof(zend_object_value));
		zvalue.handle = Z_OBJ_HANDLE_P(obj);
		zvalue.handlers = Z_OBJ_HT_P(obj);
		RETURN_BOOL(zend_hash_exists(&intern->storage, (char*)&zvalue, sizeof(zend_object_value)));
	}
#endif
} /* }}} */

/* {{{ proto int SplObjectStorage::count()
 Determine number of objects in storage */
			goto outexcept;
		}
		++p;
		ALLOC_INIT_ZVAL(pentry);
		if (!php_var_unserialize(&pentry, &p, s + buf_len, &var_hash TSRMLS_CC)) {
			zval_ptr_dtor(&pentry);
			goto outexcept;
		}
		spl_object_storage_attach(intern, pentry TSRMLS_CC);
		zval_ptr_dtor(&pentry);
	}
