 */
/*
 * file.h - definitions for file(1) program
 * @(#)$File: file.h,v 1.166 2015/01/09 19:28:32 christos Exp $
 */

#ifndef __file_h__
#define __file_h__

#define private static

#if HAVE_VISIBILITY && !defined(WIN32)
#define public  __attribute__ ((__visibility__("default")))
#ifndef protected
#define protected __attribute__ ((__visibility__("hidden")))
#endif
#endif

#ifndef HOWMANY
# define HOWMANY (1024 * 1024)	/* how much of the file to look at */
#endif
#define MAXMAGIS 8192		/* max entries in any one magic file
				   or directory */
#define MAXDESC	64		/* max len of text description/MIME type */
#define MAXstring 64		/* max len of "string" types */

#define MAGICNO		0xF11E041C
#define VERSIONNO	12
#define FILE_MAGICSIZE	248

#define	FILE_LOAD	0
#define FILE_CHECK	1
	 (t) == FILE_LESTRING16 || \
	 (t) == FILE_REGEX || \
	 (t) == FILE_SEARCH || \
	 (t) == FILE_INDIRECT || \
	 (t) == FILE_NAME || \
	 (t) == FILE_USE)

#define FILE_FMT_NONE 0
#define PSTRING_2_LE				BIT(9)
#define PSTRING_4_BE				BIT(10)
#define PSTRING_4_LE				BIT(11)
#define REGEX_LINE_COUNT			BIT(11)
#define PSTRING_LEN	\
    (PSTRING_1_BE|PSTRING_2_LE|PSTRING_2_BE|PSTRING_4_LE|PSTRING_4_BE)
#define PSTRING_LENGTH_INCLUDES_ITSELF		BIT(12)
#define	STRING_TRIM				BIT(13)
#define STRING_IGNORE_CASE		(STRING_IGNORE_LOWERCASE|STRING_IGNORE_UPPERCASE)
#define STRING_DEFAULT_RANGE		100

#define	INDIRECT_RELATIVE			BIT(0)
#define	CHAR_INDIRECT_RELATIVE			'r'

/* list of magic entries */
struct mlist {
	struct magic *magic;		/* array of magic entries */
	/* FIXME: Make the string dynamically allocated so that e.g.
	   strings matched in files can be longer than MAXstring */
	union VALUETYPE ms_value;	/* either number or string */
	uint16_t indir_max;
	uint16_t name_max;
	uint16_t elf_shnum_max;
	uint16_t elf_phnum_max;
	uint16_t elf_notes_max;
#define	FILE_INDIR_MAX			15
#define	FILE_NAME_MAX			30
#define	FILE_ELF_SHNUM_MAX		32768
#define	FILE_ELF_PHNUM_MAX		128
#define	FILE_ELF_NOTES_MAX		256
};

/* Type for Unicode characters */
typedef unsigned long unichar;
    size_t);
protected int file_fsmagic(struct magic_set *, const char *, zend_stat_t *, php_stream *);
protected int file_pipe2file(struct magic_set *, int, const void *, size_t);
protected size_t file_printedlen(const struct magic_set *);
protected int file_replace(struct magic_set *, const char *, const char *);
protected int file_printf(struct magic_set *, const char *, ...);
protected int file_reset(struct magic_set *);
protected int file_trycdf(struct magic_set *, int, const unsigned char *,
    unichar **, size_t *, const char **, const char **, const char **);
protected int file_is_tar(struct magic_set *, const unsigned char *, size_t);
protected int file_softmagic(struct magic_set *, const unsigned char *, size_t,
    uint16_t, uint16_t *, int, int);
protected int file_apprentice(struct magic_set *, const char *, int);
protected int buffer_apprentice(struct magic_set *, struct magic **,
    size_t *, size_t);
protected int file_magicfind(struct magic_set *, const char *, struct mlist *);
protected uint64_t file_signextend(struct magic_set *, struct magic *,
    uint64_t);
protected void file_delmagic(struct magic *, int type, size_t entries);
    size_t *);
protected size_t file_pstring_length_size(const struct magic *);
protected size_t file_pstring_get_length(const struct magic *, const char *);
protected char * file_printable(char *, size_t, const char *);
#ifdef __EMX__
protected int file_os2_apptype(struct magic_set *, const char *, const void *,
    size_t);
#endif /* __EMX__ */

typedef struct {
	char *buf;
	uint32_t offset;
} file_pushbuf_t;

protected file_pushbuf_t *file_push_buffer(struct magic_set *);
protected char  *file_pop_buffer(struct magic_set *, file_pushbuf_t *);

extern const char *file_names[];
extern const size_t file_nnames;

#ifndef HAVE_STRERROR
#define FINFO_LSEEK_FUNC lseek
#define FINFO_READ_FUNC read
#endif
#ifndef __RCSID
#define __RCSID(a)
#endif

#endif /* __file_h__ */