 */
/*
 * file.h - definitions for file(1) program
 * @(#)$File: file.h,v 1.247 2023/07/27 19:40:22 christos Exp $
 */

#ifndef __file_h__
#define __file_h__
#define PATHSEP	':'
#endif

#define file_private static

#if HAVE_VISIBILITY && !defined(WIN32)
#define file_public  __attribute__ ((__visibility__("default")))
#ifndef file_protected
#define file_protected __attribute__ ((__visibility__("hidden")))
#endif
#else
#define file_public
#ifndef file_protected
#define file_protected
#endif
#endif

#ifndef __arraycount
# define FD_CLOEXEC 1
#endif


/*
 * Dec 31, 23:59:59 9999
 * we need to make sure that we don't exceed 9999 because some libc
 * implementations like muslc crash otherwise
 */
#define	MAX_CTIME	CAST(time_t, 0x3afff487cfULL)

#define FILE_BADSIZE CAST(size_t, ~0ul)
#define MAXDESC	64		/* max len of text description/MIME type */
#define MAXMIME	80		/* max len of text MIME type */
#define MAXstring 128		/* max len of "string" types */
	uint16_t regex_max;
	size_t bytes_max;		/* number of bytes to read from file */
	size_t encoding_max;		/* bytes to look for encoding */
	size_t	elf_shsize_max;
#ifndef FILE_BYTES_MAX
# define FILE_BYTES_MAX (7 * 1024 * 1024)/* how much of the file to look at */
#endif /* above 0x6ab0f4 map offset for HelveticaNeue.dfont */
#define	FILE_ELF_NOTES_MAX		256
#define	FILE_ELF_PHNUM_MAX		2048
#define	FILE_ELF_SHNUM_MAX		32768
#define	FILE_ELF_SHSIZE_MAX		(128 * 1024 * 1024)
#define	FILE_INDIR_MAX			50
#define	FILE_NAME_MAX			50
#define	FILE_REGEX_MAX			8192
#define	FILE_ENCODING_MAX		(64 * 1024)
struct stat;
#define FILE_T_LOCAL	1
#define FILE_T_WINDOWS	2
file_protected const char *file_fmtdatetime(char *, size_t, uint64_t, int);
file_protected const char *file_fmtdate(char *, size_t, uint16_t);
file_protected const char *file_fmttime(char *, size_t, uint16_t);
file_protected const char *file_fmtvarint(char *, size_t, const unsigned char *,
    int);
file_protected const char *file_fmtnum(char *, size_t, const char *, int);
file_protected struct magic_set *file_ms_alloc(int);
file_protected void file_ms_free(struct magic_set *);
file_protected int file_buffer(struct magic_set *, php_stream *, zend_stat_t *, const char *, const void *,
    size_t);
file_protected int file_fsmagic(struct magic_set *, const char *, zend_stat_t *);
file_protected int file_pipe2file(struct magic_set *, int, const void *,
    size_t);
file_protected int file_vprintf(struct magic_set *, const char *, va_list)
    __attribute__((__format__(__printf__, 2, 0)));
file_protected int file_separator(struct magic_set *);
file_protected char *file_copystr(char *, size_t, size_t, const char *);
file_protected int file_checkfmt(char *, size_t, const char *);
file_protected size_t file_printedlen(const struct magic_set *);
file_protected int file_print_guid(char *, size_t, const uint64_t *);
file_protected int file_parse_guid(const char *, uint64_t *);
file_protected int file_replace(struct magic_set *, const char *, const char *);
file_protected int file_printf(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
file_protected int file_reset(struct magic_set *, int);
file_protected int file_tryelf(struct magic_set *, const struct buffer *);
file_protected int file_trycdf(struct magic_set *, const struct buffer *);
#ifdef PHP_FILEINFO_UNCOMPRESS
file_protected int file_zmagic(struct magic_set *, const struct buffer *,
    const char *);
#endif
file_protected int file_ascmagic(struct magic_set *, const struct buffer *,
    int);
file_protected int file_ascmagic_with_encoding(struct magic_set *,
    const struct buffer *, file_unichar_t *, size_t, const char *, const char *, int);
file_protected int file_encoding(struct magic_set *, const struct buffer *,
    file_unichar_t **, size_t *, const char **, const char **, const char **);
file_protected int file_is_json(struct magic_set *, const struct buffer *);
file_protected int file_is_csv(struct magic_set *, const struct buffer *, int,
    const char *);
file_protected int file_is_simh(struct magic_set *, const struct buffer *);
file_protected int file_is_tar(struct magic_set *, const struct buffer *);
file_protected int file_softmagic(struct magic_set *, const struct buffer *,
    uint16_t *, uint16_t *, int, int);
file_protected int file_apprentice(struct magic_set *, const char *, int);
file_protected size_t file_magic_strength(const struct magic *, size_t);
file_protected int buffer_apprentice(struct magic_set *, struct magic **,
    size_t *, size_t);
file_protected int file_magicfind(struct magic_set *, const char *,
    struct mlist *);
file_protected uint64_t file_signextend(struct magic_set *, struct magic *,
    uint64_t);
file_protected uintmax_t file_varint2uintmax_t(const unsigned char *, int,
    size_t *);

file_protected void file_badread(struct magic_set *);
file_protected void file_badseek(struct magic_set *);
file_protected void file_oomem(struct magic_set *, size_t);
file_protected void file_error(struct magic_set *, int, const char *, ...)
    __attribute__((__format__(__printf__, 3, 4)));
file_protected void file_magerror(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
file_protected void file_magwarn(struct magic_set *, const char *, ...)
    __attribute__((__format__(__printf__, 2, 3)));
file_protected void file_mdump(struct magic *);
file_protected void file_showstr(FILE *, const char *, size_t);
file_protected size_t file_mbswidth(struct magic_set *, const char *);
file_protected const char *file_getbuffer(struct magic_set *);
file_protected ssize_t sread(int, void *, size_t, int);
file_protected int file_check_mem(struct magic_set *, unsigned int);
file_protected int file_looks_utf8(const unsigned char *, size_t,
    file_unichar_t *, size_t *);
file_protected size_t file_pstring_length_size(struct magic_set *,
    const struct magic *);
file_protected size_t file_pstring_get_length(struct magic_set *,
    const struct magic *, const char *);
file_protected char * file_printable(struct magic_set *, char *, size_t,
    const char *, size_t);
#ifdef __EMX__
file_protected int file_os2_apptype(struct magic_set *, const char *,
    const void *, size_t);
#endif /* __EMX__ */
file_protected int file_pipe_closexec(int *);
file_protected int file_clear_closexec(int);
file_protected char *file_strtrim(char *);

file_protected void buffer_init(struct buffer *, int, const zend_stat_t *,
    const void *, size_t);
file_protected void buffer_fini(struct buffer *);
file_protected int buffer_fill(const struct buffer *);



typedef struct {
	char *buf;
	size_t blen;
	uint32_t offset;
} file_pushbuf_t;

file_protected file_pushbuf_t *file_push_buffer(struct magic_set *);
file_protected char  *file_pop_buffer(struct magic_set *, file_pushbuf_t *);

#ifndef COMPILE_ONLY
extern const char *file_names[];
extern const size_t file_nnames;