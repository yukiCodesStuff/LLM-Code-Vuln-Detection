	char * thestr;
	long gmadjust = 0;

	if (ASN1_STRING_type(timestr) != V_ASN1_UTCTIME) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "illegal ASN1 data type for timestamp");
		return (time_t)-1;
	}

	if (ASN1_STRING_length(timestr) != strlen(ASN1_STRING_data(timestr))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "illegal length in timestamp");
		return (time_t)-1;
	}

	if (ASN1_STRING_length(timestr) < 13) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unable to parse time string %s correctly", timestr->data);
		return (time_t)-1;
	}

	strbuf = estrdup((char *)ASN1_STRING_data(timestr));

	memset(&thetime, 0, sizeof(thetime));

	/* we work backwards so that we can use atoi more easily */

	thestr = strbuf + ASN1_STRING_length(timestr) - 3;

	thetime.tm_sec = atoi(thestr);
	*thestr = '\0';
	thestr -= 2;