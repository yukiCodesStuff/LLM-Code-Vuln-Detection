{
/*
	This is how the time string is formatted:

   snprintf(p, sizeof(p), "%02d%02d%02d%02d%02d%02dZ",ts->tm_year%100,
      ts->tm_mon+1,ts->tm_mday,ts->tm_hour,ts->tm_min,ts->tm_sec);
*/

	time_t ret;
	struct tm thetime;
	char * strbuf;
	char * thestr;
	long gmadjust = 0;

	if (ASN1_STRING_type(timestr) != V_ASN1_UTCTIME) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "illegal ASN1 data type for timestamp");
		return (time_t)-1;
	}