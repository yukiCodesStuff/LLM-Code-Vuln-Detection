				&index, &flags) == FAILURE) {
		return;
	}

	PHP_ZIP_STAT_INDEX(intern, index, 0, sb);
	comment = zip_get_file_comment(intern, index, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::deleteIndex(int index)
Delete a file using its index */
static ZIPARCHIVE_METHOD(deleteIndex)
{
	struct zip *intern;
	zval *this = getThis();
	long index;

	if (!this) {
		RETURN_FALSE;
	}