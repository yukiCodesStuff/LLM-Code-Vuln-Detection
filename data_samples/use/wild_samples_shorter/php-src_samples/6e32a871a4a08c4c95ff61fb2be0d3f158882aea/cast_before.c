
/* Under BSD, emulate fopencookie using funopen */
#if defined(HAVE_FUNOPEN) && !defined(HAVE_FOPENCOOKIE)
typedef struct {
	int (*reader)(void *, char *, int);
	int (*writer)(void *, const char *, int);
	fpos_t (*seeker)(void *, fpos_t, int);
	int (*closer)(void *);
} COOKIE_IO_FUNCTIONS_T;

FILE *fopencookie(void *cookie, const char *mode, COOKIE_IO_FUNCTIONS_T *funcs)
	return php_stream_write((php_stream *)cookie, (char *)buffer, size);
}

static fpos_t stream_cookie_seeker(void *cookie, off_t position, int whence)
{
	TSRMLS_FETCH();

	return (fpos_t)php_stream_seek((php_stream *)cookie, position, whence);
}

static int stream_cookie_closer(void *cookie)
{