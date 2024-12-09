
/* Under BSD, emulate fopencookie using funopen */
#if defined(HAVE_FUNOPEN) && !defined(HAVE_FOPENCOOKIE)

/* NetBSD 6.0+ uses off_t instead of fpos_t in funopen */
# if defined(__NetBSD__) && (__NetBSD_Version__ > 600000000)
#  define PHP_FPOS_T off_t
# else
#  define PHP_FPOS_T fpos_t
# endif

typedef struct {
	int (*reader)(void *, char *, int);
	int (*writer)(void *, const char *, int);
	PHP_FPOS_T (*seeker)(void *, PHP_FPOS_T, int);
	int (*closer)(void *);
} COOKIE_IO_FUNCTIONS_T;

FILE *fopencookie(void *cookie, const char *mode, COOKIE_IO_FUNCTIONS_T *funcs)
	return php_stream_write((php_stream *)cookie, (char *)buffer, size);
}

static PHP_FPOS_T stream_cookie_seeker(void *cookie, off_t position, int whence)
{
	TSRMLS_FETCH();

	return (PHP_FPOS_T)php_stream_seek((php_stream *)cookie, position, whence);
}

static int stream_cookie_closer(void *cookie)
{