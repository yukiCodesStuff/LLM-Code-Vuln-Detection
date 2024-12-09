		error_filename = "Unknown";
	}

	va_start(args, format);

#ifdef HAVE_DTRACE
	if(DTRACE_ERROR_ENABLED()) {
		char *dtrace_error_buffer;
		zend_vspprintf(&dtrace_error_buffer, 0, format, args);
		DTRACE_ERROR(dtrace_error_buffer, (char *)error_filename, error_lineno);
		efree(dtrace_error_buffer);
	}
#endif /* HAVE_DTRACE */

	/* if we don't have a user defined error handler */
	if (!EG(user_error_handler)
		|| !(EG(user_error_handler_error_reporting) & type)
		|| EG(error_handling) != EH_NORMAL) {