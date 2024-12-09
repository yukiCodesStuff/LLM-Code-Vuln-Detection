	if(DTRACE_ERROR_ENABLED()) {
		char *dtrace_error_buffer;
		zend_vspprintf(&dtrace_error_buffer, 0, format, args);
		DTRACE_ERROR(dtrace_error_buffer, error_filename, error_lineno);
		efree(dtrace_error_buffer);
	}
#endif /* HAVE_DTRACE */

	/* if we don't have a user defined error handler */
	if (!EG(user_error_handler)
		|| !(EG(user_error_handler_error_reporting) & type)
		|| EG(error_handling) != EH_NORMAL) {
		zend_error_cb(type, error_filename, error_lineno, format, args);
	} else switch (type) {
		case E_ERROR:
		case E_PARSE:
		case E_CORE_ERROR:
		case E_CORE_WARNING:
		case E_COMPILE_ERROR:
		case E_COMPILE_WARNING:
			/* The error may not be safe to handle in user-space */
			zend_error_cb(type, error_filename, error_lineno, format, args);
			break;
		default:
			/* Handle the error in user space */
			ALLOC_INIT_ZVAL(z_error_message);
			ALLOC_INIT_ZVAL(z_error_type);
			ALLOC_INIT_ZVAL(z_error_filename);
			ALLOC_INIT_ZVAL(z_error_lineno);
			ALLOC_INIT_ZVAL(z_context);

/* va_copy() is __va_copy() in old gcc versions.
 * According to the autoconf manual, using
 * memcpy(&dst, &src, sizeof(va_list))
 * gives maximum portability. */
#ifndef va_copy
# ifdef __va_copy
#  define va_copy(dest, src)	__va_copy((dest), (src))
# else
#  define va_copy(dest, src)	memcpy(&(dest), &(src), sizeof(va_list))
# endif
#endif
			va_copy(usr_copy, args);
			Z_STRLEN_P(z_error_message) = zend_vspprintf(&Z_STRVAL_P(z_error_message), 0, format, usr_copy);
#ifdef va_copy
			va_end(usr_copy);
#endif
			Z_TYPE_P(z_error_message) = IS_STRING;

			Z_LVAL_P(z_error_type) = type;
			Z_TYPE_P(z_error_type) = IS_LONG;

			if (error_filename) {
				ZVAL_STRING(z_error_filename, error_filename, 1);
			}

			Z_LVAL_P(z_error_lineno) = error_lineno;
			Z_TYPE_P(z_error_lineno) = IS_LONG;

			if (!EG(active_symbol_table)) {
				zend_rebuild_symbol_table(TSRMLS_C);
			}

			/* during shutdown the symbol table table can be still null */
			if (!EG(active_symbol_table)) {
				Z_TYPE_P(z_context) = IS_NULL;
			} else {
				Z_ARRVAL_P(z_context) = EG(active_symbol_table);
				Z_TYPE_P(z_context) = IS_ARRAY;
				zval_copy_ctor(z_context);
			}

			params = (zval ***) emalloc(sizeof(zval **)*5);
			params[0] = &z_error_type;
			params[1] = &z_error_message;
			params[2] = &z_error_filename;
			params[3] = &z_error_lineno;
			params[4] = &z_context;

			orig_user_error_handler = EG(user_error_handler);
			EG(user_error_handler) = NULL;

			/* User error handler may include() additinal PHP files.
			 * If an error was generated during comilation PHP will compile
			 * such scripts recursivly, but some CG() variables may be
			 * inconsistent. */

			in_compilation = zend_is_compiling(TSRMLS_C);
			if (in_compilation) {
				saved_class_entry = CG(active_class_entry);
				CG(active_class_entry) = NULL;
				SAVE_STACK(bp_stack);
				SAVE_STACK(function_call_stack);
				SAVE_STACK(switch_cond_stack);
				SAVE_STACK(foreach_copy_stack);
				SAVE_STACK(object_stack);
				SAVE_STACK(declare_stack);
				SAVE_STACK(list_stack);
				SAVE_STACK(context_stack);
			}

			if (call_user_function_ex(CG(function_table), NULL, orig_user_error_handler, &retval, 5, params, 1, NULL TSRMLS_CC) == SUCCESS) {
				if (retval) {
					if (Z_TYPE_P(retval) == IS_BOOL && Z_LVAL_P(retval) == 0) {
						zend_error_cb(type, error_filename, error_lineno, format, args);
					}
					zval_ptr_dtor(&retval);
				}
			} else if (!EG(exception)) {
				/* The user error handler failed, use built-in error handler */
				zend_error_cb(type, error_filename, error_lineno, format, args);
			}

			if (in_compilation) {
				CG(active_class_entry) = saved_class_entry;
				RESTORE_STACK(bp_stack);
				RESTORE_STACK(function_call_stack);
				RESTORE_STACK(switch_cond_stack);
				RESTORE_STACK(foreach_copy_stack);
				RESTORE_STACK(object_stack);
				RESTORE_STACK(declare_stack);
				RESTORE_STACK(list_stack);
				RESTORE_STACK(context_stack);
			}

			if (!EG(user_error_handler)) {
				EG(user_error_handler) = orig_user_error_handler;
			}
			else {
				zval_ptr_dtor(&orig_user_error_handler);
			}

			efree(params);
			zval_ptr_dtor(&z_error_message);
			zval_ptr_dtor(&z_error_type);
			zval_ptr_dtor(&z_error_filename);
			zval_ptr_dtor(&z_error_lineno);
			zval_ptr_dtor(&z_context);
			break;
	}

	va_end(args);

	if (type == E_PARSE) {
		/* eval() errors do not affect exit_status */
		if (!(EG(current_execute_data) &&
			EG(current_execute_data)->opline &&
			EG(current_execute_data)->opline->opcode == ZEND_INCLUDE_OR_EVAL &&
			EG(current_execute_data)->opline->extended_value == ZEND_EVAL)) {
			EG(exit_status) = 255;
		}
		zend_init_compiler_data_structures(TSRMLS_C);
	}
}
/* }}} */

#if defined(__GNUC__) && __GNUC__ >= 3 && !defined(__INTEL_COMPILER) && !defined(DARWIN) && !defined(__hpux) && !defined(_AIX) && !defined(__osf__)
void zend_error_noreturn(int type, const char *format, ...) __attribute__ ((alias("zend_error"),noreturn));
#endif

ZEND_API void zend_output_debug_string(zend_bool trigger_break, const char *format, ...) /* {{{ */
{
#if ZEND_DEBUG
	va_list args;

	va_start(args, format);
#	ifdef ZEND_WIN32
	{
		char output_buf[1024];

		vsnprintf(output_buf, 1024, format, args);
		OutputDebugString(output_buf);
		OutputDebugString("\n");
		if (trigger_break && IsDebuggerPresent()) {
			DebugBreak();
		}
	}
#	else
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
#	endif
	va_end(args);
#endif
}
/* }}} */

ZEND_API int zend_execute_scripts(int type TSRMLS_DC, zval **retval, int file_count, ...) /* {{{ */
{
	va_list files;
	int i;
	zend_file_handle *file_handle;
	zend_op_array *orig_op_array = EG(active_op_array);
	zval **orig_retval_ptr_ptr = EG(return_value_ptr_ptr);
    long orig_interactive = CG(interactive);

	va_start(files, file_count);
	for (i = 0; i < file_count; i++) {
		file_handle = va_arg(files, zend_file_handle *);
		if (!file_handle) {
			continue;
		}

        if (orig_interactive) {
            if (file_handle->filename[0] != '-' || file_handle->filename[1]) {
                CG(interactive) = 0;
            } else {
                CG(interactive) = 1;
            }
        }
       
		EG(active_op_array) = zend_compile_file(file_handle, type TSRMLS_CC);
		if (file_handle->opened_path) {
			int dummy = 1;
			zend_hash_add(&EG(included_files), file_handle->opened_path, strlen(file_handle->opened_path) + 1, (void *)&dummy, sizeof(int), NULL);
		}
		zend_destroy_file_handle(file_handle TSRMLS_CC);
		if (EG(active_op_array)) {
			EG(return_value_ptr_ptr) = retval ? retval : NULL;
			zend_execute(EG(active_op_array) TSRMLS_CC);
			zend_exception_restore(TSRMLS_C);
			if (EG(exception)) {
				if (EG(user_exception_handler)) {
					zval *orig_user_exception_handler;
					zval **params[1], *retval2, *old_exception;
					old_exception = EG(exception);
					EG(exception) = NULL;
					params[0] = &old_exception;
					orig_user_exception_handler = EG(user_exception_handler);
					if (call_user_function_ex(CG(function_table), NULL, orig_user_exception_handler, &retval2, 1, params, 1, NULL TSRMLS_CC) == SUCCESS) {
						if (retval2 != NULL) {
							zval_ptr_dtor(&retval2);
						}
						if (EG(exception)) {
							zval_ptr_dtor(&EG(exception));
							EG(exception) = NULL;
						}
						zval_ptr_dtor(&old_exception);
					} else {
						EG(exception) = old_exception;
						zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
					}
				} else {
					zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
				}
			}
			destroy_op_array(EG(active_op_array) TSRMLS_CC);
			efree(EG(active_op_array));
		} else if (type==ZEND_REQUIRE) {
			va_end(files);
			EG(active_op_array) = orig_op_array;
			EG(return_value_ptr_ptr) = orig_retval_ptr_ptr;
            CG(interactive) = orig_interactive;
			return FAILURE;
		}
	}
	va_end(files);
	EG(active_op_array) = orig_op_array;
	EG(return_value_ptr_ptr) = orig_retval_ptr_ptr;
    CG(interactive) = orig_interactive;

	return SUCCESS;
}
/* }}} */

#define COMPILED_STRING_DESCRIPTION_FORMAT "%s(%d) : %s"

ZEND_API char *zend_make_compiled_string_description(const char *name TSRMLS_DC) /* {{{ */
{
	const char *cur_filename;
	int cur_lineno;
	char *compiled_string_description;

	if (zend_is_compiling(TSRMLS_C)) {
		cur_filename = zend_get_compiled_filename(TSRMLS_C);
		cur_lineno = zend_get_compiled_lineno(TSRMLS_C);
	} else if (zend_is_executing(TSRMLS_C)) {
		cur_filename = zend_get_executed_filename(TSRMLS_C);
		cur_lineno = zend_get_executed_lineno(TSRMLS_C);
	} else {
		cur_filename = "Unknown";
		cur_lineno = 0;
	}

	zend_spprintf(&compiled_string_description, 0, COMPILED_STRING_DESCRIPTION_FORMAT, cur_filename, cur_lineno, name);
	return compiled_string_description;
}
/* }}} */

void free_estring(char **str_p) /* {{{ */
{
	efree(*str_p);
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */