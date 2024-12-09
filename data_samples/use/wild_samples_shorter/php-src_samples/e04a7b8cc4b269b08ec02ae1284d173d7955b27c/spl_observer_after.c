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
		return;
	}

	RETURN_BOOL(spl_object_storage_contains(intern, obj TSRMLS_CC));
} /* }}} */

/* {{{ proto int SplObjectStorage::count()
 Determine number of objects in storage */
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
