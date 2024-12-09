	zval_ptr_dtor(&element->inf);
} /* }}} */


spl_SplObjectStorageElement* spl_object_storage_get(spl_SplObjectStorage *intern, zval *obj TSRMLS_DC) /* {{{ */
{
	spl_SplObjectStorageElement *element;
	zend_object_value *pzvalue;	
	zval_ptr_dtor(&pcount);
		
	while(count-- > 0) {
		spl_SplObjectStorageElement *pelement;
		
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
				goto outexcept;
			}
		}
		
		pelement = spl_object_storage_get(intern, pentry TSRMLS_CC);
		if(pelement) {
			if(pelement->inf) {
				var_push_dtor(&var_hash, &pelement->inf);
			}
			if(pelement->obj) {
				var_push_dtor(&var_hash, &pelement->obj);
			}
		} 
		spl_object_storage_attach(intern, pentry, pinf TSRMLS_CC);
		zval_ptr_dtor(&pentry);
		zval_ptr_dtor(&pinf);
	}