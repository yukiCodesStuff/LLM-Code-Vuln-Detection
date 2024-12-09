	zend_replace_error_handling(EH_THROW, spl_ce_UnexpectedValueException, &error_handling TSRMLS_CC);
	if (data && *data) {
		RETVAL_ZVAL(*data, 1, 0);
	}
	if (Z_TYPE_P(return_value) == IS_ARRAY) {
		zval_dtor(return_value);
		ZVAL_STRINGL(return_value, "Array", sizeof("Array")-1, 1);
	} else {
		convert_to_string(return_value);
	}
	zend_restore_error_handling(&error_handling TSRMLS_CC);
}

		}
	}

	spl_recursive_tree_iterator_get_prefix(object, &prefix TSRMLS_CC);
	spl_recursive_tree_iterator_get_entry(object, &entry TSRMLS_CC);
	spl_recursive_tree_iterator_get_postfix(object, &postfix TSRMLS_CC);

	str_len = Z_STRLEN(prefix) + Z_STRLEN(entry) + Z_STRLEN(postfix);
	str = (char *) emalloc(str_len + 1U);