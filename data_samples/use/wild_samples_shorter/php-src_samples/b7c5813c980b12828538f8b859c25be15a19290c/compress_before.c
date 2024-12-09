#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: compress.c,v 1.136 2022/09/13 16:08:34 christos Exp $")
#endif

#include "magic.h"
#include <stdlib.h>
#include <lzma.h>
#endif

#ifdef DEBUG
int tty = -1;
#define DPRINTF(...)	do { \
	if (tty == -1) \
}

#define gzip_flags "-cd"
#define lrzip_flags "-do"
#define lzip_flags gzip_flags

static const char *gzip_args[] = {
	"gzip", gzip_flags, NULL
	"xz", "-cd", NULL
};
static const char *lrzip_args[] = {
	"lrzip", lrzip_flags, NULL
};
static const char *lz4_args[] = {
	"lz4", "-cd", NULL
};
#define	do_zlib		NULL
#define	do_bzlib	NULL

private const struct {
	union {
		const char *magic;
		int (*func)(const unsigned char *);
	} u;
#define METH_FROZEN	2
#define METH_BZIP	7
#define METH_XZ		9
#define METH_LZMA	13
#define METH_ZLIB	14
    { { .magic = "\037\235" },	2, gzip_args, NULL },	/* 0, compressed */
    /* Uncompress can get stuck; so use gzip first if we have it
#define NODATA	1
#define ERRDATA	2

private ssize_t swrite(int, const void *, size_t);
#if HAVE_FORK
private size_t ncompr = __arraycount(compr);
private int uncompressbuf(int, size_t, size_t, const unsigned char *,
    unsigned char **, size_t *);
#ifdef BUILTIN_DECOMPRESS
private int uncompresszlib(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
private int uncompressgzipped(const unsigned char *, unsigned char **, size_t,
    size_t *);
#endif
#ifdef BUILTIN_BZLIB
private int uncompressbzlib(const unsigned char *, unsigned char **, size_t,
    size_t *);
#endif
#ifdef BUILTIN_XZLIB
private int uncompressxzlib(const unsigned char *, unsigned char **, size_t,
    size_t *);
#endif

static int makeerror(unsigned char **, size_t *, const char *, ...)
    __attribute__((__format__(__printf__, 3, 4)));
private const char *methodname(size_t);

private int
format_decompression_error(struct magic_set *ms, size_t i, unsigned char *buf)
{
	unsigned char *p;
	int mime = ms->flags & MAGIC_MIME;
	    methodname(i), buf);
}

protected int
file_zmagic(struct magic_set *ms, const struct buffer *b, const char *name)
{
	unsigned char *newbuf = NULL;
	size_t i, nsz;
		}

		nsz = nbytes;
		urv = uncompressbuf(fd, ms->bytes_max, i, buf, &newbuf, &nsz);
		DPRINTF("uncompressbuf = %d, %s, %" SIZE_T_FORMAT "u\n", urv,
		    (char *)newbuf, nsz);
		switch (urv) {
		case OKDATA:
			if (urv == ERRDATA)
				prv = format_decompression_error(ms, i, newbuf);
			else
				prv = file_buffer(ms, NULL, NULL, name, newbuf, nsz);
			if (prv == -1)
				goto error;
			rv = 1;
			if ((ms->flags & MAGIC_COMPRESS_TRANSP) != 0)
			 * XXX: If file_buffer fails here, we overwrite
			 * the compressed text. FIXME.
			 */
			if (file_buffer(ms, NULL, NULL, NULL, buf, nbytes) == -1) {
				if (file_pop_buffer(ms, pb) != NULL)
					abort();
				goto error;
			}
/*
 * `safe' write for sockets and pipes.
 */
private ssize_t
swrite(int fd, const void *buf, size_t n)
{
	ssize_t rv;
	size_t rn = n;
/*
 * `safe' read for sockets and pipes.
 */
protected ssize_t
sread(int fd, void *buf, size_t n, int canbepipe __attribute__((__unused__)))
{
	ssize_t rv;
#ifdef FIONREAD
	int t = 0;
#endif
	size_t rn = n;

	if (fd == STDIN_FILENO)
		goto nocheck;

#ifdef FIONREAD
	if (canbepipe && (ioctl(fd, FIONREAD, &t) == -1 || t == 0)) {
#ifdef FD_ZERO
		ssize_t cnt;
		for (cnt = 0;; cnt++) {
	return rn;
}

protected int
file_pipe2file(struct magic_set *ms, int fd, const void *startbuf,
    size_t nbytes)
{
	char buf[4096];
#define FCOMMENT	(1 << 4)


private int
uncompressgzipped(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n)
{
	unsigned char flg = old[3];
	size_t data_start = 10;

	if (flg & FEXTRA) {
		if (data_start + 1 >= *n)
			goto err;
		data_start += 2 + old[data_start] + old[data_start + 1] * 256;
	return makeerror(newch, n, "File too short");
}

private int
uncompresszlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int zlib)
{
	int rc;
	z_stream z;

	if ((*newch = CAST(unsigned char *, emalloc(bytes_max + 1))) == NULL)
		return makeerror(newch, n, "No buffer, %s", strerror(errno));

	z.next_in = CCAST(Bytef *, old);
	z.avail_in = CAST(uint32_t, *n);
	z.next_out = *newch;
	z.avail_out = CAST(unsigned int, bytes_max);
		goto err;

	rc = inflate(&z, Z_SYNC_FLUSH);
	if (rc != Z_OK && rc != Z_STREAM_END)
		goto err;

	*n = CAST(size_t, z.total_out);
	rc = inflateEnd(&z);
	if (rc != Z_OK)

	return OKDATA;
err:
	strlcpy(RCAST(char *, *newch), z.msg ? z.msg : zError(rc), bytes_max);
	*n = strlen(RCAST(char *, *newch));
	return ERRDATA;
}
#endif

#ifdef BUILTIN_BZLIB
private int
uncompressbzlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n)
{
	int rc;
	bz_stream bz;

	memset(&bz, 0, sizeof(bz));
	rc = BZ2_bzDecompressInit(&bz, 0, 0);
	if (rc != BZ_OK)
		goto err;

	if ((*newch = CAST(unsigned char *, malloc(bytes_max + 1))) == NULL)
		return makeerror(newch, n, "No buffer, %s", strerror(errno));

	bz.next_in = CCAST(char *, RCAST(const char *, old));
	bz.avail_in = CAST(uint32_t, *n);
	bz.next_out = RCAST(char *, *newch);
	bz.avail_out = CAST(unsigned int, bytes_max);

	rc = BZ2_bzDecompress(&bz);
	if (rc != BZ_OK && rc != BZ_STREAM_END)
		goto err;

	/* Assume byte_max is within 32bit */
	/* assert(bz.total_out_hi32 == 0); */
	*n = CAST(size_t, bz.total_out_lo32);

	return OKDATA;
err:
	snprintf(RCAST(char *, *newch), bytes_max, "bunzip error %d", rc);
	*n = strlen(RCAST(char *, *newch));
	return ERRDATA;
}
#endif

#ifdef BUILTIN_XZLIB
private int
uncompressxzlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n)
{
	int rc;
	lzma_stream xz;

	memset(&xz, 0, sizeof(xz));
	rc = lzma_auto_decoder(&xz, UINT64_MAX, 0);
	if (rc != LZMA_OK)
		goto err;

	if ((*newch = CAST(unsigned char *, malloc(bytes_max + 1))) == NULL)
		return makeerror(newch, n, "No buffer, %s", strerror(errno));

	xz.next_in = CCAST(const uint8_t *, old);
	xz.avail_in = CAST(uint32_t, *n);
	xz.next_out = RCAST(uint8_t *, *newch);
	xz.avail_out = CAST(unsigned int, bytes_max);

	rc = lzma_code(&xz, LZMA_RUN);
	if (rc != LZMA_OK && rc != LZMA_STREAM_END)
		goto err;

	*n = CAST(size_t, xz.total_out);

	lzma_end(&xz);

	return OKDATA;
err:
	snprintf(RCAST(char *, *newch), bytes_max, "unxz error %d", rc);
	*n = strlen(RCAST(char *, *newch));
	return ERRDATA;
}
#endif


	va_list ap;
	int rv;

	va_start(ap, fmt);
	rv = vasprintf(&msg, fmt, ap);
	va_end(ap);
	if (rv < 0) {
		*buf = NULL;
		*len = 0;
		return NODATA;
	}
#else
	if (dup2(fd, i) == -1) {
		DPRINTF("dup(%d, %d) failed (%s)\n", fd, i, strerror(errno));
		exit(1);
	}
	close(v ? fd : fd);
#endif
}
	pid = fork();
	if (pid == -1) {
		DPRINTF("Fork failed (%s)\n", strerror(errno));
		exit(1);
	}
	if (pid == 0) {
		/* child */
		if (swrite(fd, old, n) != CAST(ssize_t, n)) {
			DPRINTF("Write failed (%s)\n", strerror(errno));
			exit(1);
		}
		exit(0);
	}
	/* parent */
	return pid;
}
	return n;
}

private const char *
methodname(size_t method)
{
	switch (method) {
#ifdef BUILTIN_DECOMPRESS
	case METH_XZ:
	case METH_LZMA:
		return "xzlib";
#endif
	default:
		return compr[method].argv[0];
	}
}

private int
uncompressbuf(int fd, size_t bytes_max, size_t method, const unsigned char *old,
    unsigned char **newch, size_t* n)
{
	int fdp[3][2];
	int status, rv, w;
	pid_t pid;
	pid_t writepid = -1;
	size_t i;
	ssize_t r;
	char *const *args;
#ifdef HAVE_POSIX_SPAWNP
	posix_spawn_file_actions_t fa;
#endif

	switch (method) {
#ifdef BUILTIN_DECOMPRESS
	case METH_FROZEN:
		return uncompressgzipped(old, newch, bytes_max, n);
	case METH_ZLIB:
		return uncompresszlib(old, newch, bytes_max, n, 1);
#endif
#ifdef BUILTIN_BZLIB
	case METH_BZIP:
		return uncompressbzlib(old, newch, bytes_max, n);
#endif
#ifdef BUILTIN_XZLIB
	case METH_XZ:
	case METH_LZMA:
		return uncompressxzlib(old, newch, bytes_max, n);
#endif
	default:
		break;
	}

	(void)fflush(stdout);
	(void)fflush(stderr);
	 * analyze two large compressed files, both will spawn
	 * an uncompressing child here, which writes out uncompressed data.
	 * We read some portion, then close the pipe, then waitpid() the child.
	 * If uncompressed data is larger, child shound get EPIPE and exit.
	 * However, with *parallel* calls OTHER child may unintentionally
	 * inherit pipe fds, thus keeping pipe open and making writes in
	 * our child block instead of failing with EPIPE!
	 * (For the bug to occur, two threads must mutually inherit their pipes,

	handledesc(&fa, fd, fdp);

	status = posix_spawnp(&pid, compr[method].argv[0], &fa, NULL,
	    args, NULL);

	posix_spawn_file_actions_destroy(&fa);
		 * do not modify fdp[i][j].
		 */
		handledesc(NULL, fd, fdp);

		(void)execvp(compr[method].argv[0], args);
		dprintf(STDERR_FILENO, "exec `%s' failed, %s",
		    compr[method].argv[0], strerror(errno));
		_exit(1); /* _exit(), not exit(), because of vfork */
	}
#endif
	/* parent */
	/* Close write sides of child stdout/err pipes */
	if (fd == -1) {
		closefd(fdp[STDIN_FILENO], 0);
		writepid = writechild(fdp[STDIN_FILENO][1], old, *n);
		closefd(fdp[STDIN_FILENO], 1);
	}

	*newch = CAST(unsigned char *, malloc(bytes_max + 1));
	if (*newch == NULL) {
		rv = makeerror(newch, n, "No buffer, %s",
		    strerror(errno));
		goto err;
	}
	rv = OKDATA;
	errno = 0;
	r = sread(fdp[STDOUT_FILENO][0], *newch, bytes_max, 0);
	if (r == 0 && errno == 0)
		goto ok;
	if (r <= 0) {
		DPRINTF("Read stdout failed %d (%s)\n", fdp[STDOUT_FILENO][0],
		    r != -1 ? strerror(errno) : "no data");

		rv = ERRDATA;
		if (r == 0 &&
		    (r = sread(fdp[STDERR_FILENO][0], *newch, bytes_max, 0)) > 0)
		{
			r = filter_error(*newch, r);
			goto ok;
		}
		free(*newch);
		if  (r == 0)
			rv = makeerror(newch, n, "Read failed, %s",
			    strerror(errno));
		else
			rv = makeerror(newch, n, "No data");
		goto err;
	}
ok:
	*n = r;
	/* NUL terminate, as every buffer is handled here. */
	(*newch)[*n] = '\0';
	w = waitpid(pid, &status, 0);
wait_err:
	if (w == -1) {
		free(*newch);
		rv = makeerror(newch, n, "Wait failed, %s", strerror(errno));
		DPRINTF("Child wait return %#x\n", status);
	} else if (!WIFEXITED(status)) {
		DPRINTF("Child not exited (%#x)\n", status);