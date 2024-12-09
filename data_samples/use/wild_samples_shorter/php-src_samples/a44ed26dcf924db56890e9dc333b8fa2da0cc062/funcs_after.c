#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: funcs.c,v 1.79 2014/12/16 20:52:49 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <stdarg.h>
#endif

#ifndef SIZE_MAX
#define SIZE_MAX	((size_t)~0)
#endif

#include "php.h"
#include "main/php_network.h"
protected int
file_printf(struct magic_set *ms, const char *fmt, ...)
{
	int rv;
	va_list ap;
	int len;
	char *buf = NULL, *newstr;


	/* try soft magic tests */
	if ((ms->flags & MAGIC_NO_CHECK_SOFT) == 0)
		if ((m = file_softmagic(ms, ubuf, nb, 0, NULL, BINTEST,
		    looks_text)) != 0) {
			if ((ms->flags & MAGIC_DEBUG) != 0)
				(void)fprintf(stderr, "softmagic %d\n", m);
#ifdef BUILTIN_ELF
				(void)fprintf(stderr, "ascmagic %d\n", m);
			goto done;
		}
	}

simple:
	/* give up */
	/* * 4 is for octal representation, + 1 is for NUL */
	len = strlen(ms->o.buf);
	if (len > (SIZE_MAX - 1) / 4) {
		file_oomem(ms, len);
		return NULL;
	}
	psize = len * 4 + 1;
	if ((ms->o.pbuf = CAST(char *, erealloc(ms->o.pbuf, psize))) == NULL) {
	return ms->o.buf == NULL ? 0 : strlen(ms->o.buf);
}

protected int
file_replace(struct magic_set *ms, const char *pat, const char *rep)
{
	zval patt;
	int opts = 0;
	(void)setlocale(LC_CTYPE, "");
	return rep_cnt;
}

protected file_pushbuf_t *
file_push_buffer(struct magic_set *ms)
{
	file_pushbuf_t *pb;

	if (ms->event_flags & EVENT_HAD_ERR)
		return NULL;

	if ((pb = (CAST(file_pushbuf_t *, emalloc(sizeof(*pb))))) == NULL)
		return NULL;

	pb->buf = ms->o.buf;
	pb->offset = ms->offset;

	ms->o.buf = NULL;
	ms->offset = 0;

	return pb;
}

protected char *
file_pop_buffer(struct magic_set *ms, file_pushbuf_t *pb)
{
	char *rbuf;

	if (ms->event_flags & EVENT_HAD_ERR) {
		efree(pb->buf);
		efree(pb);
		return NULL;
	}

	rbuf = ms->o.buf;

	ms->o.buf = pb->buf;
	ms->offset = pb->offset;

	efree(pb);
	return rbuf;
}

/*
 * convert string to ascii printable format.
 */
protected char *
file_printable(char *buf, size_t bufsiz, const char *str)
{
	char *ptr, *eptr;
	const unsigned char *s = (const unsigned char *)str;

	for (ptr = buf, eptr = ptr + bufsiz - 1; ptr < eptr && *s; s++) {
		if (isprint(*s)) {
			*ptr++ = *s;
			continue;
		}
		if (ptr >= eptr - 3)
			break;
		*ptr++ = '\\';
		*ptr++ = ((*s >> 6) & 7) + '0';
		*ptr++ = ((*s >> 3) & 7) + '0';
		*ptr++ = ((*s >> 0) & 7) + '0';
	}
	*ptr = '\0';
	return buf;
}
