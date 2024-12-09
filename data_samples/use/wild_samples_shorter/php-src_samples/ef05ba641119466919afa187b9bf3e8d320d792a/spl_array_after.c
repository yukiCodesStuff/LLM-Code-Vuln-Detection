{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	php_int_t index;
	zval *rv, *value = NULL, **tmp;

	if (check_inherited && intern->fptr_offset_has) {
		zval *offset_tmp = offset;
		SEPARATE_ARG_IF_REF(offset_tmp);
		zend_call_method_with_1_params(&object, Z_OBJCE_P(object), &intern->fptr_offset_has, "offsetExists", &rv, offset_tmp);
		zval_ptr_dtor(&offset_tmp);

		if (rv && zend_is_true(rv)) {
			zval_ptr_dtor(&rv);
			if (check_empty == 2) {
				return 1;
			} else if (intern->fptr_offset_get) {
				value = spl_array_read_dimension_ex(1, object, offset, BP_VAR_R TSRMLS_CC);
			}
		} else {
			if (rv) {
				zval_ptr_dtor(&rv);
			}
			return 0;
		}
	}

	if (!value) {
		HashTable *ht = spl_array_get_hash_table(intern, 0 TSRMLS_CC);

		switch(Z_TYPE_P(offset)) {
			case IS_STRING: 
				if (zend_symtable_find(ht, Z_STRVAL_P(offset), Z_STRSIZE_P(offset)+1, (void **) &tmp) != FAILURE) {
					if (check_empty == 2) {
						return 1;
					}
				} else {
					return 0;
				}
				break;
			case IS_DOUBLE:
			case IS_RESOURCE:
			case IS_BOOL: 
			case IS_INT:
				if (offset->type == IS_DOUBLE) {
					index = (php_int_t)Z_DVAL_P(offset);
				} else {
					index = Z_IVAL_P(offset);
				}
				if (zend_hash_index_find(ht, index, (void **)&tmp) != FAILURE) {
					if (check_empty == 2) {
						return 1;
					}
				} else {
					return 0;
				}
				break;
			default:
				zend_error(E_WARNING, "Illegal offset type");
				return 0;
		}

		if (check_inherited && intern->fptr_offset_get) {
			value = spl_array_read_dimension_ex(1, object, offset, BP_VAR_R TSRMLS_CC);
		} else {
			value = *tmp;
		}
	}

	switch (check_empty) {
		case 0:
			return Z_TYPE_P(value) != IS_NULL;
		case 2:
			return 1;
		case 1:
			return zend_is_true(value);
	}

	return 0;
} /* }}} */

static int spl_array_has_dimension(zval *object, zval *offset, int check_empty TSRMLS_DC) /* {{{ */