	int retval = SUCCESS;

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_STARTUP(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */

#ifdef PHP_WIN32
	PG(com_initialized) = 0;
#endif

#ifdef HAVE_DTRACE
	DTRACE_REQUEST_SHUTDOWN(SAFE_FILENAME(SG(request_info).path_translated), SAFE_FILENAME(SG(request_info).request_uri), SAFE_FILENAME(SG(request_info).request_method));
#endif /* HAVE_DTRACE */
}
/* }}} */
