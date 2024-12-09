	zend_string_release_ex(key, 0);
}

/* Discard variable if mangling made it start with __Host-, where pre-mangling it did not start with __Host-
 * Discard variable if mangling made it start with __Secure-, where pre-mangling it did not start with __Secure- */
static bool php_is_forbidden_variable_name(const char *mangled_name, size_t mangled_name_len, const char *pre_mangled_name)
{
	if (mangled_name_len >= sizeof("__Host-")-1 && strncmp(mangled_name, "__Host-", sizeof("__Host-")-1) == 0 && strncmp(pre_mangled_name, "__Host-", sizeof("__Host-")-1) != 0) {
		return true;
	}

	if (mangled_name_len >= sizeof("__Secure-")-1 && strncmp(mangled_name, "__Secure-", sizeof("__Secure-")-1) == 0 && strncmp(pre_mangled_name, "__Secure-", sizeof("__Secure-")-1) != 0) {
		return true;
	}

	return false;
}

PHPAPI void php_register_variable_ex(const char *var_name, zval *val, zval *track_vars_array)
{
	char *p = NULL;
	char *ip = NULL;		/* index pointer */
	}
	var_len = p - var;

	if (var_len==0) { /* empty variable name, or variable name with a space in it */
		zval_ptr_dtor_nogc(val);
		free_alloca(var_orig, use_heap);
		return;
					return;
				}
			} else {
				if (php_is_forbidden_variable_name(index, index_len, var_name)) {
					zval_ptr_dtor_nogc(val);
					free_alloca(var_orig, use_heap);
					return;
				}

				gpc_element_p = zend_symtable_str_find(symtable1, index, index_len);
				if (!gpc_element_p) {
					zval tmp;
					array_init(&tmp);
				zval_ptr_dtor_nogc(val);
			}
		} else {
			if (php_is_forbidden_variable_name(index, index_len, var_name)) {
				zval_ptr_dtor_nogc(val);
				free_alloca(var_orig, use_heap);
				return;
			}

			zend_ulong idx;

			/*
			 * According to rfc2965, more specific paths are listed above the less specific ones.