	smart_str_appendc(str, '"');
}

static inline int stricmp_end(const char* suffix, const char* str) {
    size_t suffix_len = strlen(suffix);
    size_t str_len = strlen(str);

    if (suffix_len > str_len) {
        return -1; /* Suffix is longer than string, cannot match. */
    }

    /* Compare the end of the string with the suffix, ignoring case. */
    return _stricmp(str + (str_len - suffix_len), suffix);
}

static bool is_executed_by_cmd(const char *prog_name)
{
	/* If program name is cmd.exe, then return true. */
	if (_stricmp("cmd.exe", prog_name) == 0 || _stricmp("cmd", prog_name) == 0
			|| stricmp_end("\\cmd.exe", prog_name) == 0 || stricmp_end("\\cmd", prog_name) == 0) {
		return true;
	}

    /* Find the last occurrence of the directory separator (backslash or forward slash). */
    char *last_separator = strrchr(prog_name, '\\');
    char *last_separator_fwd = strrchr(prog_name, '/');
    if (last_separator_fwd && (!last_separator || last_separator < last_separator_fwd)) {
        last_separator = last_separator_fwd;
    }

    /* Find the last dot in the filename after the last directory separator. */
    char *extension = NULL;
    if (last_separator != NULL) {
        extension = strrchr(last_separator, '.');
    } else {
        extension = strrchr(prog_name, '.');
    }

    if (extension == NULL || extension == prog_name) {
        /* No file extension found, it is not batch file. */
        return false;
    }

    /* Check if the file extension is ".bat" or ".cmd" which is always executed by cmd.exe. */
    return _stricmp(extension, ".bat") == 0 || _stricmp(extension, ".cmd") == 0;
}

static zend_string *create_win_command_from_args(HashTable *args)
{
		}

		if (is_prog_name) {
			is_cmd_execution = is_executed_by_cmd(ZSTR_VAL(arg_str));
		} else {
			smart_str_appendc(&str, ' ');
		}
