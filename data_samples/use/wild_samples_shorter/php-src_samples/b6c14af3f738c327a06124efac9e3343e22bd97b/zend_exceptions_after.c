{
#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_THROWN_ENABLED()) {
		const char *classname;
		zend_uint name_len;

		if (exception != NULL) {
			zend_get_object_classname(exception, &classname, &name_len TSRMLS_CC);
			DTRACE_EXCEPTION_THROWN((char *)classname);
		} else {
			DTRACE_EXCEPTION_THROWN(NULL);
		}
	}