 * Copyright (c) Ian F. Darwin 1986-1995.
 * Software written by Ian F. Darwin and others;
 * maintained 1995-present by Christos Zoulas and others.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
#include "cdf.h"

#ifndef lint
FILE_RCSID("@(#)$File: print.c,v 1.78 2015/01/06 02:04:10 christos Exp $")
#endif  /* lint */

#include <stdio.h>
#include <string.h>
		struct timeval ts;
		cdf_timestamp_to_timespec(&ts, t);
		t = ts.tv_sec;
	} else {
		// XXX: perhaps detect and print something if overflow
		// on 32 bit time_t?
		t = (time_t)v;
	}

	if (flags & FILE_T_LOCAL) {
		pp = ctime_r(&t, buf);
			goto out;
		pp = asctime_r(tm, buf);
	}
	if (tm == NULL)
		goto out;
	pp = asctime_r(tm, buf);

	if (pp == NULL)
		goto out;
	pp[strcspn(pp, "\n")] = '\0';