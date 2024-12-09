#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: buffer.c,v 1.8 2020/02/16 15:52:49 christos Exp $")
#endif	/* lint */

#include "magic.h"
#ifdef HAVE_UNISTD_H
buffer_fini(struct buffer *b)
{
	efree(b->ebuf);
}

int
buffer_fill(const struct buffer *bb)
	if (!S_ISREG(b->st.st_mode))
		goto out;

	b->elen =  CAST(size_t, b->st.st_size) < b->flen ?
	    CAST(size_t, b->st.st_size) : b->flen;
	if ((b->ebuf = emalloc(b->elen)) == NULL)
		goto out;

	b->eoff = b->st.st_size - b->elen;