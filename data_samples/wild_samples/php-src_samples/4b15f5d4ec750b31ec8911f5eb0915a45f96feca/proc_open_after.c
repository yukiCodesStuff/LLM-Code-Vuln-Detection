	if (has_special_character) {
		/* Escape double quote with ^ if executed by cmd.exe. */
		smart_str_appendc(str, '^');
	}
	smart_str_appendc(str, '"');
}

static bool is_executed_by_cmd(const char *prog_name, size_t prog_name_length)
{
    size_t out_len;
    WCHAR long_name[MAX_PATH];
    WCHAR full_name[MAX_PATH];
    LPWSTR file_part = NULL;

    wchar_t *prog_name_wide = php_win32_cp_conv_any_to_w(prog_name, prog_name_length, &out_len);

    if (GetLongPathNameW(prog_name_wide, long_name, MAX_PATH) == 0) {
        /* This can fail for example with ERROR_FILE_NOT_FOUND (short path resolution only works for existing files)
         * in which case we'll pass the path verbatim to the FullPath transformation. */
        lstrcpynW(long_name, prog_name_wide, MAX_PATH);
    }
		if (!arg_str) {
			smart_str_free(&str);
			return NULL;
		}

		if (is_prog_name) {
			is_cmd_execution = is_executed_by_cmd(ZSTR_VAL(arg_str), ZSTR_LEN(arg_str));
		} else {
			smart_str_appendc(&str, ' ');
		}

		append_win_escaped_arg(&str, arg_str, !is_prog_name && is_cmd_execution);

		is_prog_name = 0;
		zend_string_release(arg_str);
	} ZEND_HASH_FOREACH_END();
	smart_str_0(&str);
	return str.s;
}