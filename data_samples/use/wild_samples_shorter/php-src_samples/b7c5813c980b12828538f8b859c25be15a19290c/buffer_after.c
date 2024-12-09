#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: buffer.c,v 1.13 2023/07/02 12:48:39 christos Exp $")
#endif	/* lint */

#include "magic.h"
#ifdef HAVE_UNISTD_H
buffer_fini(struct buffer *b)
{
	efree(b->ebuf);
	b->ebuf = NULL;
	b->elen = 0;
}

int
buffer_fill(const struct buffer *bb)
	if (!S_ISREG(b->st.st_mode))
		goto out;

	b->elen = CAST(size_t, b->st.st_size) < b->flen ?
	    CAST(size_t, b->st.st_size) : b->flen;
	if (b->elen == 0) {
		efree(b->ebuf);
		b->ebuf = NULL;
		return 0;
	}
	if ((b->ebuf = emalloc(b->elen)) == NULL)
		goto out;

	b->eoff = b->st.st_size - b->elen;