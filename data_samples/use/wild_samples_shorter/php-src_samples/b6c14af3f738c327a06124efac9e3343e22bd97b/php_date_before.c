static char *date_format(char *format, int format_len, timelib_time *t, int localtime)
{
	smart_str            string = {0};
	int                  i, length;
	char                 buffer[97];
	timelib_time_offset *offset = NULL;
	timelib_sll          isoweek, isoyear;
	int                  rfc_colon;
	timelib_time   *now;
	timelib_tzinfo *tzi = NULL;
	timelib_error_container *err = NULL;
	int type = TIMELIB_ZONETYPE_ID, new_dst;
	char *new_abbr;
	timelib_sll     new_offset;
	
	if (dateobj->time) {
		timelib_time_dtor(dateobj->time);