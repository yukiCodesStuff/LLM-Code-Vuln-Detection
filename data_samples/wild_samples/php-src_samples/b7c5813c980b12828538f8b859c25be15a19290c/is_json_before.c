/*-
 * Copyright (c) 2018 Christos Zoulas
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Parse JSON object serialization format (RFC-7159)
 */

#ifndef TEST
#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: is_json.c,v 1.26 2022/09/13 18:46:07 christos Exp $")
#endif

#include "magic.h"
#else
#include <stdio.h>
#include <stddef.h>
#endif
#include <string.h>

#ifdef DEBUG
#include <stdio.h>
#define DPRINTF(a, b, c)	\
    printf("%*s%s [%.2x/%c] %.*s\n", (int)lvl, "", (a), *(b), *(b), \
	(int)(b - c), (const char *)(c))
#define __file_debugused
#else
#define DPRINTF(a, b, c)	do { } while (/*CONSTCOND*/0)
#define __file_debugused __attribute__((__unused__))
#endif

#define JSON_ARRAY	0
#define JSON_CONSTANT	1
#define JSON_NUMBER	2
#define JSON_OBJECT	3
#define JSON_STRING	4
#define JSON_ARRAYN	5
#define JSON_MAX	6

/*
 * if JSON_COUNT != 0:
 *	count all the objects, require that we have the whole data file
 * otherwise:
 *	stop if we find an object or an array
 */
#ifndef JSON_COUNT
#define JSON_COUNT 0
#endif

static int json_parse(const unsigned char **, const unsigned char *, size_t *,
	size_t);

static int
json_isspace(const unsigned char uc)
{
	switch (uc) {
	case ' ':
	case '\n':
	case '\r':
	case '\t':
		return 1;
	default:
		return 0;
	}
}
	if (mime) {
		if (file_printf(ms, "application/%s",
		    jt == 1 ? "json" : "x-ndjason") == -1)
			return -1;
		return 1;
	}
	if (mime) {
		if (file_printf(ms, "application/%s",
		    jt == 1 ? "json" : "x-ndjason") == -1)
			return -1;
		return 1;
	}
	if (file_printf(ms, "%sJSON text data",
	    jt == 1 ? "" : "New Line Delimited ") == -1)
		return -1;
#if JSON_COUNT
#define P(n) st[n], st[n] > 1 ? "s" : ""
	if (file_printf(ms, " (%" SIZE_T_FORMAT "u object%s, %" SIZE_T_FORMAT
	    "u array%s, %" SIZE_T_FORMAT "u string%s, %" SIZE_T_FORMAT
	    "u constant%s, %" SIZE_T_FORMAT "u number%s, %" SIZE_T_FORMAT
	    "u >1array%s)",
	    P(JSON_OBJECT), P(JSON_ARRAY), P(JSON_STRING), P(JSON_CONSTANT),
	    P(JSON_NUMBER), P(JSON_ARRAYN))
	    == -1)
		return -1;
#endif
	return 1;
}

#else

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <err.h>

int
main(int argc, char *argv[])
{
	int fd, rv;
	struct stat st;
	unsigned char *p;
	size_t stats[JSON_MAX];

	if ((fd = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "Can't open `%s'", argv[1]);

	if (fstat(fd, &st) == -1)
		err(EXIT_FAILURE, "Can't stat `%s'", argv[1]);

	if ((p = CAST(char *, malloc(st.st_size))) == NULL)
		err(EXIT_FAILURE, "Can't allocate %jd bytes",
		    (intmax_t)st.st_size);
	if (read(fd, p, st.st_size) != st.st_size)
		err(EXIT_FAILURE, "Can't read %jd bytes",
		    (intmax_t)st.st_size);
	memset(stats, 0, sizeof(stats));
	printf("is json %d\n", json_parse((const unsigned char **)&p,
	    p + st.st_size, stats, 0));
	return 0;
}