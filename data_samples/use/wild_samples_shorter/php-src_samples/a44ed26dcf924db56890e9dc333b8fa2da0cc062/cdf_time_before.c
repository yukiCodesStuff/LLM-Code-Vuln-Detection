#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: cdf_time.c,v 1.12 2012/05/15 17:14:36 christos Exp $")
#endif

#include <time.h>
#ifdef TEST

	for (y = CDF_BASE_YEAR; y < year; y++)
		days += isleap(y) + 365;

	return days;
}

/*
	return days;
}

/*
 * Return the 0...11 month number.
 */
static int
cdf_getmonth(int year, int days)
	char *ptr = ctime_r(sec, buf);
	if (ptr != NULL)
		return buf;
	(void)snprintf(buf, 26, "*Bad* 0x%16.16llx\n", (long long)*sec);
	return buf;
}


#ifdef TEST
int
main(int argc, char *argv[])
{
	struct timeval ts;