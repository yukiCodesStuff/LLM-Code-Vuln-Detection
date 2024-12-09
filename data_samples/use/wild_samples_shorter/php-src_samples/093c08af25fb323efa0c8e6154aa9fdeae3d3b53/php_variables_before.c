	zend_string_release_ex(key, 0);
}

PHPAPI void php_register_variable_ex(const char *var_name, zval *val, zval *track_vars_array)
{
	char *p = NULL;
	char *ip = NULL;		/* index pointer */
	}
	var_len = p - var;

	/* Discard variable if mangling made it start with __Host-, where pre-mangling it did not start with __Host- */
	if (strncmp(var, "__Host-", sizeof("__Host-")-1) == 0 && strncmp(var_name, "__Host-", sizeof("__Host-")-1) != 0) {
		zval_ptr_dtor_nogc(val);
		free_alloca(var_orig, use_heap);
		return;
	}

	/* Discard variable if mangling made it start with __Secure-, where pre-mangling it did not start with __Secure- */
	if (strncmp(var, "__Secure-", sizeof("__Secure-")-1) == 0 && strncmp(var_name, "__Secure-", sizeof("__Secure-")-1) != 0) {
		zval_ptr_dtor_nogc(val);
		free_alloca(var_orig, use_heap);
		return;
	}

	if (var_len==0) { /* empty variable name, or variable name with a space in it */
		zval_ptr_dtor_nogc(val);
		free_alloca(var_orig, use_heap);
		return;
					return;
				}
			} else {
				gpc_element_p = zend_symtable_str_find(symtable1, index, index_len);
				if (!gpc_element_p) {
					zval tmp;
					array_init(&tmp);
				zval_ptr_dtor_nogc(val);
			}
		} else {
			zend_ulong idx;

			/*
			 * According to rfc2965, more specific paths are listed above the less specific ones.