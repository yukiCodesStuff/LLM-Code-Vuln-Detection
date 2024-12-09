	if (has_special_character) {
		/* Escape double quote with ^ if executed by cmd.exe. */
		smart_str_appendc(str, '^');
	}
	smart_str_appendc(str, '"');
}

static inline int stricmp_end(const char* suffix, const char* str) {
    size_t suffix_len = strlen(suffix);
    size_t str_len = strlen(str);

    if (suffix_len > str_len) {
        return -1; /* Suffix is longer than string, cannot match. */
    }
		if (!arg_str) {
			smart_str_free(&str);
			return NULL;
		}

		if (is_prog_name) {
			is_cmd_execution = is_executed_by_cmd(ZSTR_VAL(arg_str));
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