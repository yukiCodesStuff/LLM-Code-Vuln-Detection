		error_filename = "Unknown";
	}

#ifdef HAVE_DTRACE
	if(DTRACE_ERROR_ENABLED()) {
		char *dtrace_error_buffer;
		va_start(args, format);
		zend_vspprintf(&dtrace_error_buffer, 0, format, args);
		DTRACE_ERROR(dtrace_error_buffer, (char *)error_filename, error_lineno);
		efree(dtrace_error_buffer);
		va_end(args);
	}
#endif /* HAVE_DTRACE */

	va_start(args, format);

	/* if we don't have a user defined error handler */
	if (!EG(user_error_handler)
		|| !(EG(user_error_handler_error_reporting) & type)
		|| EG(error_handling) != EH_NORMAL) {