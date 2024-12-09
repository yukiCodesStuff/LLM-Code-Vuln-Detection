#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: cdf_time.c,v 1.24 2023/07/17 15:54:44 christos Exp $")
#endif

#include <time.h>
#ifdef TEST
		return -1;
	}
	*t = (ts->ts_nsec / 100) * CDF_TIME_PREC;
	*t += tm.tm_sec;
	*t += tm.tm_min * 60;
	*t += tm.tm_hour * 60 * 60;
	*t += tm.tm_mday * 60 * 60 * 24;
#endif
char *
cdf_ctime(const time_t *sec, char *buf)
{
	char *ptr = *sec > MAX_CTIME ? NULL : ctime_r(sec, buf);
	if (ptr != NULL)
		return buf;
#ifdef WIN32
	(void)snprintf(buf, 26, "*Bad* 0x%16.16I64x\n",