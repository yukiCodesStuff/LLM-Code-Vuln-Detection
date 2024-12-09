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

    free(prog_name_wide);
    prog_name_wide = NULL;

    if (GetFullPathNameW(long_name, MAX_PATH, full_name, &file_part) == 0 || file_part == NULL) {
        return false;
    }

    bool uses_cmd = false;
    if (_wcsicmp(file_part, L"cmd.exe") == 0 || _wcsicmp(file_part, L"cmd") == 0) {
        uses_cmd = true;
    } else {
        const WCHAR *extension_dot = wcsrchr(file_part, L'.');
        if (extension_dot && (_wcsicmp(extension_dot, L".bat") == 0 || _wcsicmp(extension_dot, L".cmd") == 0)) {
            uses_cmd = true;
        }
    }

    return uses_cmd;
}

static zend_string *create_win_command_from_args(HashTable *args)
{
		}

		if (is_prog_name) {
			is_cmd_execution = is_executed_by_cmd(ZSTR_VAL(arg_str), ZSTR_LEN(arg_str));
		} else {
			smart_str_appendc(&str, ' ');
		}
