	 */
	virtual_file_ex(&new_state, file, NULL, CWD_EXPAND);
	path_cleaned =  php_zip_make_relative_path(new_state.cwd, new_state.cwd_length);
	path_cleaned_len = strlen(path_cleaned);

	if (path_cleaned_len >= MAXPATHLEN || zip_stat(za, file, 0, &sb) != 0) {
		return 0;