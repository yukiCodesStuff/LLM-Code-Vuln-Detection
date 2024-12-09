	}

	comment = zip_get_archive_comment(intern, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */
