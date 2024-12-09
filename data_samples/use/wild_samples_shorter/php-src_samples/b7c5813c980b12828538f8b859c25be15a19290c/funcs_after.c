#include "file.h"

#ifndef	lint
FILE_RCSID("@(#)$File: funcs.c,v 1.140 2023/05/21 17:08:34 christos Exp $")
#endif	/* lint */

#include "magic.h"
#include <assert.h>
#define SIZE_MAX	((size_t)~0)
#endif

file_protected char *
file_copystr(char *buf, size_t blen, size_t width, const char *str)
{
	if (blen == 0)
		return buf;
	return buf;
}

file_private void
file_clearbuf(struct magic_set *ms)
{
	efree(ms->o.buf);
	ms->o.buf = NULL;
	ms->o.blen = 0;
}

file_private int
file_checkfield(char *msg, size_t mlen, const char *what, const char **pp)
{
	const char *p = *pp;
	int fw = 0;
	return 0;
}

file_protected int
file_checkfmt(char *msg, size_t mlen, const char *fmt)
{
	const char *p;
	for (p = fmt; *p; p++) {
/*
 * Like printf, only we append to a buffer.
 */
file_protected int
file_vprintf(struct magic_set *ms, const char *fmt, va_list ap)
{
	size_t len;
	char *buf, *newstr;
	return 0;
}

file_protected int
file_printf(struct magic_set *ms, const char *fmt, ...)
{
	int rv;
	va_list ap;
 */
/*VARARGS*/
__attribute__((__format__(__printf__, 3, 0)))
file_private void
file_error_core(struct magic_set *ms, int error, const char *f, va_list va,
    size_t lineno)
{
	/* Only the first error is ok */
}

/*VARARGS*/
file_protected void
file_error(struct magic_set *ms, int error, const char *f, ...)
{
	va_list va;
	va_start(va, f);
 * Print an error with magic line number.
 */
/*VARARGS*/
file_protected void
file_magerror(struct magic_set *ms, const char *f, ...)
{
	va_list va;
	va_start(va, f);
	va_end(va);
}

file_protected void
file_oomem(struct magic_set *ms, size_t len)
{
	file_error(ms, errno, "cannot allocate %" SIZE_T_FORMAT "u bytes",
	    len);
}

file_protected void
file_badseek(struct magic_set *ms)
{
	file_error(ms, errno, "error seeking");
}

file_protected void
file_badread(struct magic_set *ms)
{
	file_error(ms, errno, "error reading");
}
#ifndef COMPILE_ONLY
#define FILE_SEPARATOR "\n- "

file_protected int
file_separator(struct magic_set *ms)
{
	return file_printf(ms, FILE_SEPARATOR);
}
	return 0;
}

file_protected int
file_default(struct magic_set *ms, size_t nb)
{
	if (ms->flags & MAGIC_MIME) {
		if ((ms->flags & MAGIC_MIME_TYPE) &&
 *	-1: error
 */
/*ARGSUSED*/
file_protected int
file_buffer(struct magic_set *ms, php_stream *stream, zend_stat_t *st,
    const char *inname __attribute__ ((__unused__)),
    const void *buf, size_t nb)
{
	int m = 0, rv = 0, looks_text = 0;
	const char *code = NULL;

	/* Check if we have a CSV file */
	if ((ms->flags & MAGIC_NO_CHECK_CSV) == 0) {
		m = file_is_csv(ms, &b, looks_text, code);
		if ((ms->flags & MAGIC_DEBUG) != 0)
			(void)fprintf(stderr, "[try csv %d]\n", m);
		if (m) {
			if (checkdone(ms, &rv))
		}
	}

	/* Check if we have a SIMH tape file */
	if ((ms->flags & MAGIC_NO_CHECK_SIMH) == 0) {
		m = file_is_simh(ms, &b);
		if ((ms->flags & MAGIC_DEBUG) != 0)
			(void)fprintf(stderr, "[try simh %d]\n", m);
		if (m) {
			if (checkdone(ms, &rv))
				goto done;
		}
	}

	/* Check if we have a CDF file */
	if ((ms->flags & MAGIC_NO_CHECK_CDF) == 0) {
		m = file_trycdf(ms, &b);
		if ((ms->flags & MAGIC_DEBUG) != 0)
		rv = file_tryelf(ms, &b);
		rbuf = file_pop_buffer(ms, pb);
		if (rv == -1) {
			efree(rbuf);
			rbuf = NULL;
		}
		if ((ms->flags & MAGIC_DEBUG) != 0)
			(void)fprintf(stderr, "[try elf %d]\n", m);
}
#endif

file_protected int
file_reset(struct magic_set *ms, int checkloaded)
{
	if (checkloaded && ms->mlist[0] == NULL) {
		file_error(ms, 0, "no magic files loaded");
	*(n)++ = ((CAST(uint32_t, *(o)) >> 0) & 7) + '0', \
	(o)++)

file_protected const char *
file_getbuffer(struct magic_set *ms)
{
	char *pbuf, *op, *np;
	size_t psize, len;
	return ms->o.pbuf;
}

file_protected int
file_check_mem(struct magic_set *ms, unsigned int level)
{
	size_t len;

	return 0;
}

file_protected size_t
file_printedlen(const struct magic_set *ms)
{
	return ms->o.blen;
}

file_protected int
file_replace(struct magic_set *ms, const char *pat, const char *rep)
{
	zend_string *pattern;
	uint32_t opts = 0;
		goto out;
	}

	memcpy(ms->o.buf, ZSTR_VAL(res), ZSTR_LEN(res));
	ms->o.buf[ZSTR_LEN(res)] = '\0';

	zend_string_release_ex(res, 0);

	return rep_cnt;
}

file_protected file_pushbuf_t *
file_push_buffer(struct magic_set *ms)
{
	file_pushbuf_t *pb;

	return pb;
}

file_protected char *
file_pop_buffer(struct magic_set *ms, file_pushbuf_t *pb)
{
	char *rbuf;

/*
 * convert string to ascii printable format.
 */
file_protected char *
file_printable(struct magic_set *ms, char *buf, size_t bufsiz,
    const char *str, size_t slen)
{
	char *ptr, *eptr = buf + bufsiz - 1;
	uint8_t data4[8];
};

file_protected int
file_parse_guid(const char *s, uint64_t *guid)
{
	struct guid *g = CAST(struct guid *, CAST(void *, guid));
#ifndef WIN32
#endif
}

file_protected int
file_print_guid(char *str, size_t len, const uint64_t *guid)
{
	const struct guid *g = CAST(const struct guid *,
	    CAST(const void *, guid));
}

#if 0
file_protected int
file_pipe_closexec(int *fds)
{
#ifdef __MINGW32__
	return 0;
#elif defined(HAVE_PIPE2)
	return pipe2(fds, O_CLOEXEC);
#else
	if (pipe(fds) == -1)
		return -1;
}
#endif

file_protected int
file_clear_closexec(int fd) {
#ifdef F_SETFD
	return fcntl(fd, F_SETFD, 0);
#else
#endif
}

file_protected char *
file_strtrim(char *str)
{
	char *last;
