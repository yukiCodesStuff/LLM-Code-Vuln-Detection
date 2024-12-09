	if(DTRACE_ERROR_ENABLED()) {
		char *dtrace_error_buffer;
		zend_vspprintf(&dtrace_error_buffer, 0, format, args);
		DTRACE_ERROR(dtrace_error_buffer, (char *)error_filename, error_lineno);
		efree(dtrace_error_buffer);
	}
#endif /* HAVE_DTRACE */
