	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &flags) == FAILURE) {
		return;
	}

	comment = zip_get_archive_comment(intern, &comment_len, (int)flags);
	RETURN_STRINGL((char *)comment, (long)comment_len, 1);
}
/* }}} */

/* {{{ proto bool ZipArchive::setCommentName(string name, string comment)
Set or remove (NULL/'') the comment of an entry using its Name */
static ZIPARCHIVE_METHOD(setCommentName)
{
	struct zip *intern;
	zval *this = getThis();
	int comment_len, name_len;
	char * comment, *name;
	int idx;

	if (!this) {
		RETURN_FALSE;
	}