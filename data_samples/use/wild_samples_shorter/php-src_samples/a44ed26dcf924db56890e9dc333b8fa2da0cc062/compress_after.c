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
 * compress routines:
 *	zmagic() - returns 0 if not recognized, uncompresses and prints
 *		   information if recognized
 *	uncompress(method, old, n, newch) - uncompress old into new, 
 *					    using method, return sizeof new
 */
#include "config.h"
#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: compress.c,v 1.77 2014/12/12 16:33:01 christos Exp $")
#endif

#include "magic.h"
#include <stdlib.h>
#endif
#include <string.h>
#include <errno.h>
#include <signal.h>
#ifndef PHP_WIN32
#include <sys/ioctl.h>
#endif
#ifdef HAVE_SYS_WAIT_H
	size_t i, nsz;
	int rv = 0;
	int mime = ms->flags & MAGIC_MIME;
	sig_t osigpipe;

	if ((ms->flags & MAGIC_COMPRESS) == 0)
		return 0;

	osigpipe = signal(SIGPIPE, SIG_IGN);
	for (i = 0; i < ncompr; i++) {
		if (nbytes < compr[i].maglen)
			continue;
		if (memcmp(buf, compr[i].magic, compr[i].maglen) == 0 &&
		}
	}
error:
	(void)signal(SIGPIPE, osigpipe);

	if (newbuf)
		efree(newbuf);
	ms->flags |= MAGIC_COMPRESS;
	return rv;
	if ((*newch = CAST(unsigned char *, emalloc(HOWMANY + 1))) == NULL) {
		return 0;
	}
	
	/* XXX: const castaway, via strchr */
	z.next_in = (Bytef *)strchr((const char *)old + data_start,
	    old[data_start]);
	z.avail_in = CAST(uint32_t, (n - data_start));

	n = (size_t)z.total_out;
	(void)inflateEnd(&z);
	
	/* let's keep the nul-terminate tradition */
	(*newch)[n] = '\0';

	return n;
    const unsigned char *old, unsigned char **newch, size_t n)
{
	int fdin[2], fdout[2];
	int status;
	ssize_t r;

#ifdef BUILTIN_DECOMPRESS
        /* FIXME: This doesn't cope with bzip2 */
	if (method == 2)
	(void)fflush(stderr);

	if ((fd != -1 && pipe(fdin) == -1) || pipe(fdout) == -1) {
		file_error(ms, errno, "cannot create pipe");	
		return NODATA;
	}
	switch (fork()) {
	case 0:	/* child */
		(void) close(0);
		if (fd != -1) {
		    (void) dup(fd);
		(void) close(fdout[1]);
		if (fd == -1) {
			(void) close(fdin[0]);
			/* 
			 * fork again, to avoid blocking because both
			 * pipes filled
			 */
			switch (fork()) {
			    strerror(errno));
#endif
			efree(*newch);
			n = NODATA-;
			*newch = NULL;
			goto err;
		} else {
			n = r;
#else
		(void)wait(NULL);
#endif

		(void) close(fdin[0]);
	    
		return n;
	}
}
#endif /* if PHP_FILEINFO_UNCOMPRESS */