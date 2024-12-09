#include "file.h"

#ifndef lint
FILE_RCSID("@(#)$File: compress.c,v 1.157 2023/05/21 15:59:58 christos Exp $")
#endif

#include "magic.h"
#include <stdlib.h>
#include <lzma.h>
#endif

#if defined(HAVE_ZSTD_H) && defined(ZSTDLIBSUPPORT)
#define BUILTIN_ZSTDLIB
#include <zstd.h>
#include <zstd_errors.h>
#endif

#if defined(HAVE_LZLIB_H) && defined(LZLIBSUPPORT)
#define BUILTIN_LZLIB
#include <lzlib.h>
#endif

#ifdef DEBUG
int tty = -1;
#define DPRINTF(...)	do { \
	if (tty == -1) \
}

#define gzip_flags "-cd"
#define lzip_flags gzip_flags

static const char *gzip_args[] = {
	"gzip", gzip_flags, NULL
	"xz", "-cd", NULL
};
static const char *lrzip_args[] = {
	"lrzip", "-qdf", "-", NULL
};
static const char *lz4_args[] = {
	"lz4", "-cd", NULL
};
#define	do_zlib		NULL
#define	do_bzlib	NULL

file_private const struct {
	union {
		const char *magic;
		int (*func)(const unsigned char *);
	} u;
#define METH_FROZEN	2
#define METH_BZIP	7
#define METH_XZ		9
#define METH_LZIP	8
#define METH_ZSTD	12
#define METH_LZMA	13
#define METH_ZLIB	14
    { { .magic = "\037\235" },	2, gzip_args, NULL },	/* 0, compressed */
    /* Uncompress can get stuck; so use gzip first if we have it
#define NODATA	1
#define ERRDATA	2

file_private ssize_t swrite(int, const void *, size_t);
#if HAVE_FORK
file_private size_t ncompr = __arraycount(compr);
file_private int uncompressbuf(int, size_t, size_t, int, const unsigned char *,
    unsigned char **, size_t *);
#ifdef BUILTIN_DECOMPRESS
file_private int uncompresszlib(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
file_private int uncompressgzipped(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
#endif
#ifdef BUILTIN_BZLIB
file_private int uncompressbzlib(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
#endif
#ifdef BUILTIN_XZLIB
file_private int uncompressxzlib(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
#endif
#ifdef BUILTIN_ZSTDLIB
file_private int uncompresszstd(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
#endif
#ifdef BUILTIN_LZLIB
file_private int uncompresslzlib(const unsigned char *, unsigned char **, size_t,
    size_t *, int);
#endif

static int makeerror(unsigned char **, size_t *, const char *, ...)
    __attribute__((__format__(__printf__, 3, 4)));
file_private const char *methodname(size_t);

file_private int
format_decompression_error(struct magic_set *ms, size_t i, unsigned char *buf)
{
	unsigned char *p;
	int mime = ms->flags & MAGIC_MIME;
	    methodname(i), buf);
}

file_protected int
file_zmagic(struct magic_set *ms, const struct buffer *b, const char *name)
{
	unsigned char *newbuf = NULL;
	size_t i, nsz;
		}

		nsz = nbytes;
		efree(newbuf);
		urv = uncompressbuf(fd, ms->bytes_max, i, 
		    (ms->flags & MAGIC_NO_COMPRESS_FORK), buf, &newbuf, &nsz);
		DPRINTF("uncompressbuf = %d, %s, %" SIZE_T_FORMAT "u\n", urv,
		    (char *)newbuf, nsz);
		switch (urv) {
		case OKDATA:
			if (urv == ERRDATA)
				prv = format_decompression_error(ms, i, newbuf);
			else
				prv = file_buffer(ms, NULL, NULL, name, newbuf,
				    nsz);
			if (prv == -1)
				goto error;
			rv = 1;
			if ((ms->flags & MAGIC_COMPRESS_TRANSP) != 0)
			 * XXX: If file_buffer fails here, we overwrite
			 * the compressed text. FIXME.
			 */
			if (file_buffer(ms, NULL, NULL, NULL, buf, nbytes) == -1)
			{
				if (file_pop_buffer(ms, pb) != NULL)
					abort();
				goto error;
			}
/*
 * `safe' write for sockets and pipes.
 */
file_private ssize_t
swrite(int fd, const void *buf, size_t n)
{
	ssize_t rv;
	size_t rn = n;
/*
 * `safe' read for sockets and pipes.
 */
file_protected ssize_t
sread(int fd, void *buf, size_t n, int canbepipe __attribute__((__unused__)))
{
	ssize_t rv;
#if defined(FIONREAD) && !defined(__MINGW32__)
	int t = 0;
#endif
	size_t rn = n;

	if (fd == STDIN_FILENO)
		goto nocheck;

#if defined(FIONREAD) && !defined(__MINGW32__)
	if (canbepipe && (ioctl(fd, FIONREAD, &t) == -1 || t == 0)) {
#ifdef FD_ZERO
		ssize_t cnt;
		for (cnt = 0;; cnt++) {
	return rn;
}

file_protected int
file_pipe2file(struct magic_set *ms, int fd, const void *startbuf,
    size_t nbytes)
{
	char buf[4096];
#define FCOMMENT	(1 << 4)


file_private int
uncompressgzipped(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int extra __attribute__((__unused__)))
{
	unsigned char flg;
	size_t data_start = 10;

	if (*n < 4) {
		goto err;	
	}

	flg = old[3];

	if (flg & FEXTRA) {
		if (data_start + 1 >= *n)
			goto err;
		data_start += 2 + old[data_start] + old[data_start + 1] * 256;
	return makeerror(newch, n, "File too short");
}

file_private int
uncompresszlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int zlib)
{
	int rc;
	z_stream z;

	DPRINTF("builtin zlib decompression\n");
	z.next_in = CCAST(Bytef *, old);
	z.avail_in = CAST(uint32_t, *n);
	z.next_out = *newch;
	z.avail_out = CAST(unsigned int, bytes_max);
		goto err;

	rc = inflate(&z, Z_SYNC_FLUSH);
	if (rc != Z_OK && rc != Z_STREAM_END) {
		inflateEnd(&z);
		goto err;
	}

	*n = CAST(size_t, z.total_out);
	rc = inflateEnd(&z);
	if (rc != Z_OK)

	return OKDATA;
err:
	return makeerror(newch, n, "%s", z.msg ? z.msg : zError(rc));
}
#endif

#ifdef BUILTIN_BZLIB
file_private int
uncompressbzlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int extra __attribute__((__unused__)))
{
	int rc;
	bz_stream bz;

	DPRINTF("builtin bzlib decompression\n");
	memset(&bz, 0, sizeof(bz));
	rc = BZ2_bzDecompressInit(&bz, 0, 0);
	if (rc != BZ_OK)
		goto err;

	bz.next_in = CCAST(char *, RCAST(const char *, old));
	bz.avail_in = CAST(uint32_t, *n);
	bz.next_out = RCAST(char *, *newch);
	bz.avail_out = CAST(unsigned int, bytes_max);

	rc = BZ2_bzDecompress(&bz);
	if (rc != BZ_OK && rc != BZ_STREAM_END) {
		BZ2_bzDecompressEnd(&bz);
		goto err;
	}

	/* Assume byte_max is within 32bit */
	/* assert(bz.total_out_hi32 == 0); */
	*n = CAST(size_t, bz.total_out_lo32);

	return OKDATA;
err:
	return makeerror(newch, n, "bunzip error %d", rc);
}
#endif

#ifdef BUILTIN_XZLIB
file_private int
uncompressxzlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int extra __attribute__((__unused__)))
{
	int rc;
	lzma_stream xz;

	DPRINTF("builtin xzlib decompression\n");
	memset(&xz, 0, sizeof(xz));
	rc = lzma_auto_decoder(&xz, UINT64_MAX, 0);
	if (rc != LZMA_OK)
		goto err;

	xz.next_in = CCAST(const uint8_t *, old);
	xz.avail_in = CAST(uint32_t, *n);
	xz.next_out = RCAST(uint8_t *, *newch);
	xz.avail_out = CAST(unsigned int, bytes_max);

	rc = lzma_code(&xz, LZMA_RUN);
	if (rc != LZMA_OK && rc != LZMA_STREAM_END) {
		lzma_end(&xz);
		goto err;
	}

	*n = CAST(size_t, xz.total_out);

	lzma_end(&xz);

	return OKDATA;
err:
	return makeerror(newch, n, "unxz error %d", rc);
}
#endif

#ifdef BUILTIN_ZSTDLIB
file_private int
uncompresszstd(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int extra __attribute__((__unused__)))
{
	size_t rc;
	ZSTD_DStream *zstd;
	ZSTD_inBuffer in;
	ZSTD_outBuffer out;

	DPRINTF("builtin zstd decompression\n");
	if ((zstd = ZSTD_createDStream()) == NULL) {
		return makeerror(newch, n, "No ZSTD decompression stream, %s",
		    strerror(errno));
	}

	rc = ZSTD_DCtx_reset(zstd, ZSTD_reset_session_only);
	if (ZSTD_isError(rc))
		goto err;

	in.src = CCAST(const void *, old);
	in.size = *n;
	in.pos = 0;
	out.dst = RCAST(void *, *newch);
	out.size = bytes_max;
	out.pos = 0;

	rc = ZSTD_decompressStream(zstd, &out, &in);
	if (ZSTD_isError(rc))
		goto err;

	*n = out.pos;

	ZSTD_freeDStream(zstd);

	/* let's keep the nul-terminate tradition */
	(*newch)[*n] = '\0';

	return OKDATA;
err:
	ZSTD_freeDStream(zstd);
	return makeerror(newch, n, "zstd error %d", ZSTD_getErrorCode(rc));
}
#endif

#ifdef BUILTIN_LZLIB
file_private int
uncompresslzlib(const unsigned char *old, unsigned char **newch,
    size_t bytes_max, size_t *n, int extra __attribute__((__unused__)))
{
	enum LZ_Errno err;
	size_t old_remaining = *n;
	size_t new_remaining = bytes_max;
	size_t total_read = 0;
	unsigned char *bufp;
	struct LZ_Decoder *dec;

	bufp = *newch;

	DPRINTF("builtin lzlib decompression\n");
	dec = LZ_decompress_open();
	if (!dec) {
		return makeerror(newch, n, "unable to allocate LZ_Decoder");
	}
	if (LZ_decompress_errno(dec) != LZ_ok)
		goto err;

	for (;;) {
		// LZ_decompress_read() stops at member boundaries, so we may
		// have more than one successful read after writing all data
		// we have.
		if (old_remaining > 0) {
			int wr = LZ_decompress_write(dec, old, old_remaining);
			if (wr < 0)
				goto err;
			old_remaining -= wr;
			old += wr;
		}

		int rd = LZ_decompress_read(dec, bufp, new_remaining);
		if (rd > 0) {
			new_remaining -= rd;
			bufp += rd;
			total_read += rd;
		}

		if (rd < 0 || LZ_decompress_errno(dec) != LZ_ok)
			goto err;
		if (new_remaining == 0)
			break;
		if (old_remaining == 0 && rd == 0)
			break;
	}

	LZ_decompress_close(dec);
	*n = total_read;

	/* let's keep the nul-terminate tradition */
	*bufp = '\0';

	return OKDATA;
err:
	err = LZ_decompress_errno(dec);
	LZ_decompress_close(dec);
	return makeerror(newch, n, "lzlib error: %s", LZ_strerror(err));
}
#endif


	va_list ap;
	int rv;

	DPRINTF("Makeerror %s\n", fmt);
	free(*buf);
	va_start(ap, fmt);
	rv = vasprintf(&msg, fmt, ap);
	va_end(ap);
	if (rv < 0) {
		DPRINTF("Makeerror failed");
		*buf = NULL;
		*len = 0;
		return NODATA;
	}
#else
	if (dup2(fd, i) == -1) {
		DPRINTF("dup(%d, %d) failed (%s)\n", fd, i, strerror(errno));
		exit(EXIT_FAILURE);
	}
	close(v ? fd : fd);
#endif
}
	pid = fork();
	if (pid == -1) {
		DPRINTF("Fork failed (%s)\n", strerror(errno));
		return -1;
	}
	if (pid == 0) {
		/* child */
		if (swrite(fd, old, n) != CAST(ssize_t, n)) {
			DPRINTF("Write failed (%s)\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
		exit(EXIT_SUCCESS);
	}
	/* parent */
	return pid;
}
	return n;
}

file_private const char *
methodname(size_t method)
{
	switch (method) {
#ifdef BUILTIN_DECOMPRESS
	case METH_XZ:
	case METH_LZMA:
		return "xzlib";
#endif
#ifdef BUILTIN_ZSTDLIB
	case METH_ZSTD:
		return "zstd";
#endif
#ifdef BUILTIN_LZLIB
	case METH_LZIP:
		return "lzlib";
#endif
	default:
		return compr[method].argv[0];
	}
}

file_private int (*
getdecompressor(size_t method))(const unsigned char *, unsigned char **, size_t,
    size_t *, int)
{
	switch (method) {
#ifdef BUILTIN_DECOMPRESS
	case METH_FROZEN:
		return uncompressgzipped;
	case METH_ZLIB:
		return uncompresszlib;
#endif
#ifdef BUILTIN_BZLIB
	case METH_BZIP:
		return uncompressbzlib;
#endif
#ifdef BUILTIN_XZLIB
	case METH_XZ:
	case METH_LZMA:
		return uncompressxzlib;
#endif
#ifdef BUILTIN_ZSTDLIB
	case METH_ZSTD:
		return uncompresszstd;
#endif
#ifdef BUILTIN_LZLIB
	case METH_LZIP:
		return uncompresslzlib;
#endif
	default:
		return NULL;
	}
}

file_private int
uncompressbuf(int fd, size_t bytes_max, size_t method, int nofork,
    const unsigned char *old, unsigned char **newch, size_t* n)
{
	int fdp[3][2];
	int status, rv, w;
	pid_t pid;
	pid_t writepid = -1;
	size_t i;
	ssize_t r, re;
	char *const *args;
#ifdef HAVE_POSIX_SPAWNP
	posix_spawn_file_actions_t fa;
#endif
	int (*decompress)(const unsigned char *, unsigned char **,
	    size_t, size_t *, int) = getdecompressor(method);

	*newch = CAST(unsigned char *, emalloc(bytes_max + 1));
	if (*newch == NULL)
		return makeerror(newch, n, "No buffer, %s", strerror(errno));

	if (decompress) {
		if (nofork) {
			return makeerror(newch, n,
			    "Fork is required to uncompress, but disabled");
		}
		return (*decompress)(old, newch, bytes_max, n, 1);
	}

	(void)fflush(stdout);
	(void)fflush(stderr);
	 * analyze two large compressed files, both will spawn
	 * an uncompressing child here, which writes out uncompressed data.
	 * We read some portion, then close the pipe, then waitpid() the child.
	 * If uncompressed data is larger, child should get EPIPE and exit.
	 * However, with *parallel* calls OTHER child may unintentionally
	 * inherit pipe fds, thus keeping pipe open and making writes in
	 * our child block instead of failing with EPIPE!
	 * (For the bug to occur, two threads must mutually inherit their pipes,

	handledesc(&fa, fd, fdp);

	DPRINTF("Executing %s\n", compr[method].argv[0]);
	status = posix_spawnp(&pid, compr[method].argv[0], &fa, NULL,
	    args, NULL);

	posix_spawn_file_actions_destroy(&fa);
		 * do not modify fdp[i][j].
		 */
		handledesc(NULL, fd, fdp);
		DPRINTF("Executing %s\n", compr[method].argv[0]);

		(void)execvp(compr[method].argv[0], args);
		dprintf(STDERR_FILENO, "exec `%s' failed, %s",
		    compr[method].argv[0], strerror(errno));
		_exit(EXIT_FAILURE); /* _exit(), not exit(), because of vfork */
	}
#endif
	/* parent */
	/* Close write sides of child stdout/err pipes */
	if (fd == -1) {
		closefd(fdp[STDIN_FILENO], 0);
		writepid = writechild(fdp[STDIN_FILENO][1], old, *n);
		if (writepid == (pid_t)-1) {
			rv = makeerror(newch, n, "Write to child failed, %s",
			    strerror(errno));
			DPRINTF("Write to child failed\n");
			goto err;
		}
		closefd(fdp[STDIN_FILENO], 1);
	}

	rv = OKDATA;
	r = sread(fdp[STDOUT_FILENO][0], *newch, bytes_max, 0);
	DPRINTF("read got %zd\n", r);
	if (r < 0) {
		rv = ERRDATA;
		DPRINTF("Read stdout failed %d (%s)\n", fdp[STDOUT_FILENO][0],
		        strerror(errno));
		goto err;
	} 
	if (CAST(size_t, r) == bytes_max) {
		/*
		 * close fd so that the child exits with sigpipe and ignore
		 * errors, otherwise we risk the child blocking and never
		 * exiting.
		 */
		DPRINTF("Closing stdout for bytes_max\n");
		closefd(fdp[STDOUT_FILENO], 0);
		goto ok;
	}
	if ((re = sread(fdp[STDERR_FILENO][0], *newch, bytes_max, 0)) > 0) {
		DPRINTF("Got stuff from stderr %s\n", *newch);
		rv = ERRDATA;
		r = filter_error(*newch, r);
		goto ok;
	}
	if  (re == 0)
		goto ok;
	rv = makeerror(newch, n, "Read stderr failed, %s",
	    strerror(errno));
	goto err;
ok:
	*n = r;
	/* NUL terminate, as every buffer is handled here. */
	(*newch)[*n] = '\0';
	w = waitpid(pid, &status, 0);
wait_err:
	if (w == -1) {
		rv = makeerror(newch, n, "Wait failed, %s", strerror(errno));
		DPRINTF("Child wait return %#x\n", status);
	} else if (!WIFEXITED(status)) {
		DPRINTF("Child not exited (%#x)\n", status);