	if (data && *data) {
		RETVAL_ZVAL(*data, 1, 0);
		if (Z_TYPE_P(return_value) == IS_ARRAY) {
			zval_dtor(return_value);
			ZVAL_STRINGL(return_value, "Array", sizeof("Array")-1, 1);
		} else {
			convert_to_string(return_value);
		}
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);
}
		} else {
			RETURN_NULL();
		}
	}

	INIT_ZVAL(prefix);
	INIT_ZVAL(entry);
	spl_recursive_tree_iterator_get_prefix(object, &prefix TSRMLS_CC);
	spl_recursive_tree_iterator_get_entry(object, &entry TSRMLS_CC);
	if (Z_TYPE(entry) != IS_STRING) {
		zval_dtor(&prefix);
		zval_dtor(&entry);
		RETURN_NULL();
	}
	spl_recursive_tree_iterator_get_postfix(object, &postfix TSRMLS_CC);

	str_len = Z_STRLEN(prefix) + Z_STRLEN(entry) + Z_STRLEN(postfix);
	str = (char *) emalloc(str_len + 1U);
	ptr = str;

	memcpy(ptr, Z_STRVAL(prefix), Z_STRLEN(prefix));
	ptr += Z_STRLEN(prefix);
	memcpy(ptr, Z_STRVAL(entry), Z_STRLEN(entry));
	ptr += Z_STRLEN(entry);
	memcpy(ptr, Z_STRVAL(postfix), Z_STRLEN(postfix));
	ptr += Z_STRLEN(postfix);
	*ptr = 0;

	zval_dtor(&prefix);
	zval_dtor(&entry);
	zval_dtor(&postfix);

	RETURN_STRINGL(str, str_len, 0);
} /* }}} */

/* {{{ proto mixed RecursiveTreeIterator::key()
   Returns the current key prefixed and postfixed */
SPL_METHOD(RecursiveTreeIterator, key)
{
	spl_recursive_it_object   *object = (spl_recursive_it_object*)zend_object_store_get_object(getThis() TSRMLS_CC);
	zend_object_iterator      *iterator = object->iterators[object->level].iterator;
	zval                       prefix, key, postfix, key_copy;
	char                      *str, *ptr;
	size_t                     str_len;
	
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}