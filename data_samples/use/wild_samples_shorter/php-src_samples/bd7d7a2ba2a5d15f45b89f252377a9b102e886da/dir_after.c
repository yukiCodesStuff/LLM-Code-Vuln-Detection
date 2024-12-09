	/* now catch the FreeBSD style of "no matches" */
	if (!globbuf.gl_pathc || !globbuf.gl_pathv) {
no_results:
#ifndef PHP_WIN32
		/* Paths containing '*', '?' and some other chars are
		illegal on Windows but legit on other platforms. For
		this reason the direct basedir check against the glob
		query is senseless on windows. For instance while *.txt
		is a pretty valid filename on EXT3, it's invalid on NTFS. */
		if (PG(open_basedir) && *PG(open_basedir)) {
			if (php_check_open_basedir_ex(pattern, 0 TSRMLS_CC)) {
				RETURN_FALSE;
			}
		}
#endif
		array_init(return_value);
		return;
	}
