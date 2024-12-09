
#ifdef HAVE_DTRACE
/* PHP DTrace probes {{{ */
static inline const char *dtrace_get_executed_filename(TSRMLS_D)
{
	if (EG(current_execute_data) && EG(current_execute_data)->op_array) {
		return EG(current_execute_data)->op_array->filename;
	} else {
ZEND_API zend_op_array *dtrace_compile_file(zend_file_handle *file_handle, int type TSRMLS_DC)
{
	zend_op_array *res;
	DTRACE_COMPILE_FILE_ENTRY(file_handle->opened_path, (char *)file_handle->filename);
	res = compile_file(file_handle, type TSRMLS_CC);
	DTRACE_COMPILE_FILE_RETURN(file_handle->opened_path, (char *)file_handle->filename);

	return res;
}

ZEND_API void dtrace_execute_ex(zend_execute_data *execute_data TSRMLS_DC)
{
	int lineno;
	const char *scope, *filename, *funcname, *classname;
	scope = filename = funcname = classname = NULL;

	/* we need filename and lineno for both execute and function probes */
	if (DTRACE_EXECUTE_ENTRY_ENABLED() || DTRACE_EXECUTE_RETURN_ENABLED()
	}

	if (DTRACE_EXECUTE_ENTRY_ENABLED()) {
		DTRACE_EXECUTE_ENTRY((char *)filename, lineno);
	}

	if (DTRACE_FUNCTION_ENTRY_ENABLED() && funcname != NULL) {
		DTRACE_FUNCTION_ENTRY((char *)funcname, (char *)filename, lineno, (char *)classname, (char *)scope);
	}

	execute_ex(execute_data TSRMLS_CC);

	if (DTRACE_FUNCTION_RETURN_ENABLED() && funcname != NULL) {
		DTRACE_FUNCTION_RETURN((char *)funcname, (char *)filename, lineno, (char *)classname, (char *)scope);
	}

	if (DTRACE_EXECUTE_RETURN_ENABLED()) {
		DTRACE_EXECUTE_RETURN((char *)filename, lineno);
	}
}

ZEND_API void dtrace_execute_internal(zend_execute_data *execute_data_ptr, zend_fcall_info *fci, int return_value_used TSRMLS_DC)
{
	int lineno;
	const char *filename;
	if (DTRACE_EXECUTE_ENTRY_ENABLED() || DTRACE_EXECUTE_RETURN_ENABLED()) {
		filename = dtrace_get_executed_filename(TSRMLS_C);
		lineno = zend_get_executed_lineno(TSRMLS_C);
	}

	if (DTRACE_EXECUTE_ENTRY_ENABLED()) {
		DTRACE_EXECUTE_ENTRY((char *)filename, lineno);
	}

	execute_internal(execute_data_ptr, fci, return_value_used TSRMLS_CC);

	if (DTRACE_EXECUTE_RETURN_ENABLED()) {
		DTRACE_EXECUTE_RETURN((char *)filename, lineno);
	}
}

/* }}} */