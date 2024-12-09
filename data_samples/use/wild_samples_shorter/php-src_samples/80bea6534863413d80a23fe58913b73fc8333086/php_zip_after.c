
	PHP_ZIP_STAT_INDEX(intern, index, 0, sb);
	comment = zip_get_file_comment(intern, index, &comment_len, (int)flags);
	if(comment==NULL) {
		RETURN_FALSE;
	}
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */
