	char * thestr;
	long gmadjust = 0;

	if (timestr->length < 13) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "extension author too lazy to parse %s correctly", timestr->data);
		return (time_t)-1;
	}

	strbuf = estrdup((char *)timestr->data);

	memset(&thetime, 0, sizeof(thetime));

	/* we work backwards so that we can use atoi more easily */

	thestr = strbuf + timestr->length - 3;

	thetime.tm_sec = atoi(thestr);
	*thestr = '\0';
	thestr -= 2;