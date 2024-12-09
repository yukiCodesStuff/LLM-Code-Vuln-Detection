{
	zend_string *key = zend_string_init_interned(name, name_len, 0);

	zend_hash_update_ind(ht, key, val);
	zend_string_release_ex(key, 0);
}

/* Discard variable if mangling made it start with __Host-, where pre-mangling it did not start with __Host-
 * Discard variable if mangling made it start with __Secure-, where pre-mangling it did not start with __Secure- */
static bool php_is_forbidden_variable_name(const char *mangled_name, size_t mangled_name_len, const char *pre_mangled_name)
{
	if (mangled_name_len >= sizeof("__Host-")-1 && strncmp(mangled_name, "__Host-", sizeof("__Host-")-1) == 0 && strncmp(pre_mangled_name, "__Host-", sizeof("__Host-")-1) != 0) {
		return true;
	}
		} else if (*p == '[') {
			is_array = 1;
			ip = p;
			*p = 0;
			break;
		}
	}
	var_len = p - var;

	if (var_len==0) { /* empty variable name, or variable name with a space in it */
		zval_ptr_dtor_nogc(val);
		free_alloca(var_orig, use_heap);
		return;
	}

	if (var_len == sizeof("this")-1 && EG(current_execute_data)) {
		zend_execute_data *ex = EG(current_execute_data);

		while (ex) {
			if (ex->func && ZEND_USER_CODE(ex->func->common.type)) {
				if ((ZEND_CALL_INFO(ex) & ZEND_CALL_HAS_SYMBOL_TABLE)
						&& ex->symbol_table == symtable1) {
					if (memcmp(var, "this", sizeof("this")-1) == 0) {
						zend_throw_error(NULL, "Cannot re-assign $this");
						zval_ptr_dtor_nogc(val);
						free_alloca(var_orig, use_heap);
						return;
					}
				}
				break;
			}
			ex = ex->prev_execute_data;
		}
				if ((gpc_element_p = zend_hash_next_index_insert(symtable1, &gpc_element)) == NULL) {
					zend_array_destroy(Z_ARR(gpc_element));
					zval_ptr_dtor_nogc(val);
					free_alloca(var_orig, use_heap);
					return;
				}
			} else {
				if (php_is_forbidden_variable_name(index, index_len, var_name)) {
					zval_ptr_dtor_nogc(val);
					free_alloca(var_orig, use_heap);
					return;
				}
			if (zend_hash_next_index_insert(symtable1, val) == NULL) {
				zval_ptr_dtor_nogc(val);
			}
		} else {
			if (php_is_forbidden_variable_name(index, index_len, var_name)) {
				zval_ptr_dtor_nogc(val);
				free_alloca(var_orig, use_heap);
				return;
			}