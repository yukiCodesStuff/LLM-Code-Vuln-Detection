{
	spl_array_object *intern = (spl_array_object*)zend_object_store_get_object(object TSRMLS_CC);
	php_int_t index;
	zval *rv, **tmp;

	if (check_inherited && intern->fptr_offset_has) {
		SEPARATE_ARG_IF_REF(offset);
		zend_call_method_with_1_params(&object, Z_OBJCE_P(object), &intern->fptr_offset_has, "offsetExists", &rv, offset);
		zval_ptr_dtor(&offset);
		if (rv && zend_is_true(rv)) {
			zval_ptr_dtor(&rv);
			return 1;
		}
		if (rv) {
			zval_ptr_dtor(&rv);
		}
		return 0;
	}