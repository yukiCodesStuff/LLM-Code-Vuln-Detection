			if (!RETURN_VALUE_USED(opline)) {
				zval_ptr_dtor(&EX_T(opline->result.u.var).var.ptr);
			}
		} else if (RETURN_VALUE_USED(opline)) {
			EX_T(opline->result.u.var).var.ptr = NULL;
		}
	} else if (EX(function_state).function->type == ZEND_USER_FUNCTION) {
		EX(original_return_value) = EG(return_value_ptr_ptr);
		EG(active_symbol_table) = NULL;