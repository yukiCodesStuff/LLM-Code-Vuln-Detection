#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: ascmagic.c,v 1.110 2021/12/06 15:33:00 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <string.h>
#define ISSPC(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n' \
		  || (x) == 0x85 || (x) == '\f')

private unsigned char *encode_utf8(unsigned char *, size_t, file_unichar_t *,
    size_t);
private size_t trim_nuls(const unsigned char *, size_t);

/*
 * Undo the NUL-termination kindly provided by process()
 * but leave at least one byte to look at
 */
private size_t
trim_nuls(const unsigned char *buf, size_t nbytes)
{
	while (nbytes > 1 && buf[nbytes - 1] == '\0')
		nbytes--;
	return nbytes;
}

protected int
file_ascmagic(struct magic_set *ms, const struct buffer *b, int text)
{
	file_unichar_t *ubuf = NULL;
	size_t ulen = 0;
	return rv;
}

protected int
file_ascmagic_with_encoding(struct magic_set *ms, const struct buffer *b,
    file_unichar_t *ubuf, size_t ulen, const char *code, const char *type,
    int text)
{
	int has_backspace = 0;
	int seen_cr = 0;

	int n_crlf = 0;
	int n_lf = 0;
	int n_cr = 0;
	int n_nel = 0;
	int executable = 0;

	size_t last_line_end = CAST(size_t, -1);
	size_t has_long_lines = 0;
			goto done;
		}
		if ((utf8_end = encode_utf8(utf8_buf, mlen, ubuf, ulen))
		    == NULL)
			goto done;
		buffer_init(&bb, b->fd, &b->st, utf8_buf,
		    CAST(size_t, utf8_end - utf8_buf));

		if ((rv = file_softmagic(ms, &bb, NULL, NULL,
			has_backspace = 1;
	}

	/* Beware, if the data has been truncated, the final CR could have
	   been followed by a LF.  If we have ms->bytes_max bytes, it indicates
	   that the data might have been truncated, probably even before
	   this function was called. */
	if (seen_cr && nbytes < ms->bytes_max)
		n_cr++;

	if (strcmp(type, "binary") == 0) {
		rv = 0;
		goto done;
	}
				}
				if (need_separator && file_separator(ms) == -1)
					goto done;
			} else {
				if (file_printf(ms, "text/plain") == -1)
					goto done;
			}
		}
	} else {
		if (len) {
			switch (file_replace(ms, " text$", ", ")) {
 * Encode Unicode string as UTF-8, returning pointer to character
 * after end of string, or NULL if an invalid character is found.
 */
private unsigned char *
encode_utf8(unsigned char *buf, size_t len, file_unichar_t *ubuf, size_t ulen)
{
	size_t i;
	unsigned char *end = buf + len;