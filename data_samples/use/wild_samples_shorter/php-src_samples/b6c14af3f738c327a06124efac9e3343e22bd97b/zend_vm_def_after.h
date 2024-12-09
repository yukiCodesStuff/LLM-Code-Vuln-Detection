			if (call->object) {
				Z_ADDREF_P(call->object);
			}
			if (OP2_TYPE == IS_VAR && OP2_FREE && Z_REFCOUNT_P(function_name) == 1 &&
			    call->fbc->common.fn_flags & ZEND_ACC_CLOSURE) {
				/* Delay closure destruction until its invocation */
				call->fbc->common.prototype = (zend_function*)function_name;
			} else {

#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_CAUGHT_ENABLED()) {
		DTRACE_EXCEPTION_CAUGHT((char *)ce->name);
	}
#endif /* HAVE_DTRACE */

	if (ce != catch_ce) {