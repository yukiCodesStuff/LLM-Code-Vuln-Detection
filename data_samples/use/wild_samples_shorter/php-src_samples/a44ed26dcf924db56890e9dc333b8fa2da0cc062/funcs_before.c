#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: funcs.c,v 1.68 2014/02/18 11:09:31 kim Exp $")
#endif	/* lint */

#include "magic.h"
#include <stdarg.h>
#endif

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#include "php.h"
#include "main/php_network.h"
protected int
file_printf(struct magic_set *ms, const char *fmt, ...)
{
	va_list ap;
	int len;
	char *buf = NULL, *newstr;


	/* try soft magic tests */
	if ((ms->flags & MAGIC_NO_CHECK_SOFT) == 0)
		if ((m = file_softmagic(ms, ubuf, nb, 0, BINTEST,
		    looks_text)) != 0) {
			if ((ms->flags & MAGIC_DEBUG) != 0)
				(void)fprintf(stderr, "softmagic %d\n", m);
#ifdef BUILTIN_ELF
				(void)fprintf(stderr, "ascmagic %d\n", m);
			goto done;
		}

		/* try to discover text encoding */
		if ((ms->flags & MAGIC_NO_CHECK_ENCODING) == 0) {
			if (looks_text == 0)
				if ((m = file_ascmagic_with_encoding( ms, ubuf,
				    nb, u8buf, ulen, code, ftype, looks_text))
				    != 0) {
					if ((ms->flags & MAGIC_DEBUG) != 0)
						(void)fprintf(stderr,
						    "ascmagic/enc %d\n", m);
					goto done;
				}
		}
	}

simple:
	/* give up */
	/* * 4 is for octal representation, + 1 is for NUL */
	len = strlen(ms->o.buf);
	if (len > (SIZE_MAX - 1) / 4) {
		return NULL;
	}
	psize = len * 4 + 1;
	if ((ms->o.pbuf = CAST(char *, erealloc(ms->o.pbuf, psize))) == NULL) {
	return ms->o.buf == NULL ? 0 : strlen(ms->o.buf);
}

file_replace(struct magic_set *ms, const char *pat, const char *rep)
{
	zval patt;
	int opts = 0;
	(void)setlocale(LC_CTYPE, "");
	return rep_cnt;
}