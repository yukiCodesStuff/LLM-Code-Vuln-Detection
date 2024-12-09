			if (!RETURN_VALUE_USED(opline)) {
				zval_ptr_dtor(&EX_T(opline->result.u.var).var.ptr);
			}
		} else if (RETURN_VALUE_USED(opline)) {
			EX_T(opline->result.u.var).var.ptr = NULL;
		}
	} else if (EX(function_state).function->type == ZEND_USER_FUNCTION) {
		EX(original_return_value) = EG(return_value_ptr_ptr);
		EG(active_symbol_table) = NULL;
		EG(active_op_array) = &EX(function_state).function->op_array;
		EG(return_value_ptr_ptr) = NULL;
		if (RETURN_VALUE_USED(opline)) {			
			EG(return_value_ptr_ptr) = &EX_T(opline->result.u.var).var.ptr;
			EX_T(opline->result.u.var).var.ptr = NULL;
			EX_T(opline->result.u.var).var.ptr_ptr = &EX_T(opline->result.u.var).var.ptr;
			EX_T(opline->result.u.var).var.fcall_returned_reference = EX(function_state).function->common.return_reference;
		}