	/* now catch the FreeBSD style of "no matches" */
	if (!globbuf.gl_pathc || !globbuf.gl_pathv) {
no_results:
		if (PG(open_basedir) && *PG(open_basedir)) {
			if (php_check_open_basedir_ex(pattern, 0 TSRMLS_CC)) {
				RETURN_FALSE;
			}
		}
		array_init(return_value);
		return;
	}
