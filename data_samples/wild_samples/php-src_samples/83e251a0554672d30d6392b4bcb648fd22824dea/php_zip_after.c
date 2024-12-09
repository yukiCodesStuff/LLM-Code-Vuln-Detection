{
	struct zip *intern;
	zval *this = getThis();
	char *pattern;
	char *path = NULL;
	char *remove_path = NULL;
	char *add_path = NULL;
	int pattern_len, add_path_len = 0, remove_path_len = 0, path_len = 0;
	long remove_all_path = 0;
	long flags = 0;
	zval *options = NULL;
	int found;

	if (!this) {
		RETURN_FALSE;
	}