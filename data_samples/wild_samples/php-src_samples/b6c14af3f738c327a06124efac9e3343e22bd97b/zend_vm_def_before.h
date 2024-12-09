			if (call->object) {
				Z_ADDREF_P(call->object);
			}
			if (OP2_TYPE == IS_VAR && OP2_FREE &&
			    call->fbc->common.fn_flags & ZEND_ACC_CLOSURE) {
				/* Delay closure destruction until its invocation */
				call->fbc->common.prototype = (zend_function*)function_name;
			} else {
				FREE_OP2();
			}
	} else {
		catch_ce = zend_fetch_class_by_name(Z_STRVAL_P(opline->op1.zv), Z_STRLEN_P(opline->op1.zv), opline->op1.literal + 1, ZEND_FETCH_CLASS_NO_AUTOLOAD TSRMLS_CC);

		CACHE_PTR(opline->op1.literal->cache_slot, catch_ce);
	}
	ce = Z_OBJCE_P(EG(exception));

#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_CAUGHT_ENABLED()) {
		DTRACE_EXCEPTION_CAUGHT(ce->name);
	}