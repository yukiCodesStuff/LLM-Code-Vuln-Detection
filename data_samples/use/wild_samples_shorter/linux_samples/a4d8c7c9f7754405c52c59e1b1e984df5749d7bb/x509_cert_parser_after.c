	if (*p != 'Z')
		goto unsupported_time;

	if (year < 1970 ||
	    mon < 1 || mon > 12)
		goto invalid_time;

	mon_len = month_lengths[mon - 1];
	if (mon == 2) {
		if (year % 4 == 0) {
			mon_len = 29;
			if (year % 100 == 0) {
		}
	}

	if (day < 1 || day > mon_len ||
	    hour > 23 ||
	    min > 59 ||
	    sec > 59)
		goto invalid_time;

	*_t = mktime64(year, mon, day, hour, min, sec);
	return 0;

unsupported_time: