 */
/*
 * file.h - definitions for file(1) program
 * @(#)$File: file.h,v 1.237 2022/09/10 13:21:42 christos Exp $
 */

#ifndef __file_h__
#define __file_h__
#define PATHSEP	':'
#endif

#define private static

#if HAVE_VISIBILITY && !defined(WIN32)
#define public  __attribute__ ((__visibility__("default")))
#ifndef protected
#define protected __attribute__ ((__visibility__("hidden")))
#endif
#else
#define public
#ifndef protected
#define protected
#endif
#endif

#ifndef __arraycount
# define FD_CLOEXEC 1
#endif

#define FILE_BADSIZE CAST(size_t, ~0ul)
#define MAXDESC	64		/* max len of text description/MIME type */
#define MAXMIME	80		/* max len of text MIME type */
#define MAXstring 128		/* max len of "string" types */
	uint16_t regex_max;
	size_t bytes_max;		/* number of bytes to read from file */
	size_t encoding_max;		/* bytes to look for encoding */
#ifndef FILE_BYTES_MAX
# define FILE_BYTES_MAX (1024 * 1024)	/* how much of the file to look at */
#endif
#define	FILE_ELF_NOTES_MAX		256
#define	FILE_ELF_PHNUM_MAX		2048
#define	FILE_ELF_SHNUM_MAX		32768
#define	FILE_INDIR_MAX			50
#define	FILE_NAME_MAX			50
#define	FILE_REGEX_MAX			8192
#define	FILE_ENCODING_MAX		(64 * 1024)
struct stat;
#define FILE_T_LOCAL	1
#define FILE_T_WINDOWS	2
protected const char *file_fmtdatetime(char *, size_t, uint64_t, int);
protected const char *file_fmtdate(char *, size_t, uint16_t);
protected const char *file_fmttime(char *, size_t, uint16_t);
protected const char *file_fmtvarint(char *, size_t, const unsigned char *,
    int);
protected const char *file_fmtnum(char *, size_t, const char *, int);
protected struct magic_set *file_ms_alloc(int);
protected void file_ms_free(struct magic_set *);
protected int file_buffer(struct magic_set *, php_stream *, zend_stat_t *, const char *, const void *,
    size_t);
protected int file_fsmagic(struct magic_set *, const char *, zend_stat_t *);
protected int file_pipe2file(struct magic_set *, int, const void *, size_t);
protected int file_vprintf(struct magic_set *, const char *, va_list)
    __attribute__((__format__(__printf__, 2, 0)));
protected int file_separator(struct magic_set *);
protected char *file_copystr(char *, size_t, size_t, const char *);
protected int file_checkfmt(char *, size_t, const char *);
protected size_t file_printedlen(const struct magic_set *);
protected int file_print_guid(char *, size_t, const uint64_t *);
protected int file_parse_guid(const char *, uint64_t *);
protected int file_replace(struct magic_set *, const char *, const char *);
protected int file_printf(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
protected int file_reset(struct magic_set *, int);
protected int file_tryelf(struct magic_set *, const struct buffer *);
protected int file_trycdf(struct magic_set *, const struct buffer *);
#ifdef PHP_FILEINFO_UNCOMPRESS
protected int file_zmagic(struct magic_set *, const struct buffer *,
    const char *);
#endif
protected int file_ascmagic(struct magic_set *, const struct buffer *,
    int);
protected int file_ascmagic_with_encoding(struct magic_set *,
    const struct buffer *, file_unichar_t *, size_t, const char *, const char *, int);
protected int file_encoding(struct magic_set *, const struct buffer *,
    file_unichar_t **, size_t *, const char **, const char **, const char **);
protected int file_is_json(struct magic_set *, const struct buffer *);
protected int file_is_csv(struct magic_set *, const struct buffer *, int);
protected int file_is_tar(struct magic_set *, const struct buffer *);
protected int file_softmagic(struct magic_set *, const struct buffer *,
    uint16_t *, uint16_t *, int, int);
protected int file_apprentice(struct magic_set *, const char *, int);
protected int buffer_apprentice(struct magic_set *, struct magic **,
    size_t *, size_t);
protected int file_magicfind(struct magic_set *, const char *, struct mlist *);
protected uint64_t file_signextend(struct magic_set *, struct magic *,
    uint64_t);
protected uintmax_t file_varint2uintmax_t(const unsigned char *, int, size_t *);

protected void file_badread(struct magic_set *);
protected void file_badseek(struct magic_set *);
protected void file_oomem(struct magic_set *, size_t);
protected void file_error(struct magic_set *, int, const char *, ...)
    __attribute__((__format__(__printf__, 3, 4)));
protected void file_magerror(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
protected void file_magwarn(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
protected void file_mdump(struct magic *);
protected void file_showstr(FILE *, const char *, size_t);
protected size_t file_mbswidth(struct magic_set *, const char *);
protected const char *file_getbuffer(struct magic_set *);
protected ssize_t sread(int, void *, size_t, int);
protected int file_check_mem(struct magic_set *, unsigned int);
protected int file_looks_utf8(const unsigned char *, size_t, file_unichar_t *,
    size_t *);
protected size_t file_pstring_length_size(struct magic_set *,
    const struct magic *);
protected size_t file_pstring_get_length(struct magic_set *,
    const struct magic *, const char *);
protected char * file_printable(struct magic_set *, char *, size_t,
    const char *, size_t);
#ifdef __EMX__
protected int file_os2_apptype(struct magic_set *, const char *, const void *,
    size_t);
#endif /* __EMX__ */
protected int file_pipe_closexec(int *);
protected int file_clear_closexec(int);
protected char *file_strtrim(char *);

protected void buffer_init(struct buffer *, int, const zend_stat_t *,
    const void *, size_t);
protected void buffer_fini(struct buffer *);
protected int buffer_fill(const struct buffer *);

typedef struct {
	char *buf;
	size_t blen;
	uint32_t offset;
} file_pushbuf_t;

protected file_pushbuf_t *file_push_buffer(struct magic_set *);
protected char  *file_pop_buffer(struct magic_set *, file_pushbuf_t *);

#ifndef COMPILE_ONLY
extern const char *file_names[];
extern const size_t file_nnames;