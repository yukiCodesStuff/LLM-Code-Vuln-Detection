#define	MAGIC_NO_CHECK_TOKENS	0x0100000 /* Don't check tokens */
#define MAGIC_NO_CHECK_ENCODING 0x0200000 /* Don't check text encodings */
#define MAGIC_NO_CHECK_JSON	0x0400000 /* Don't check for JSON files */
#define MAGIC_NO_CHECK_SIMH	0x0800000 /* Don't check for SIMH tape files */

/* No built-in tests; only consult the magic file */
#define MAGIC_NO_CHECK_BUILTIN	( \
	MAGIC_NO_CHECK_COMPRESS	| \
	MAGIC_NO_CHECK_TOKENS	| \
	MAGIC_NO_CHECK_ENCODING	| \
	MAGIC_NO_CHECK_JSON	| \
	MAGIC_NO_CHECK_SIMH | \
	0			  \
)

#define MAGIC_SNPRINTB "\177\020\
#define	MAGIC_NO_CHECK_FORTRAN	0x000000 /* Don't check ascii/fortran */
#define	MAGIC_NO_CHECK_TROFF	0x000000 /* Don't check ascii/troff */

#define MAGIC_VERSION		545	/* This implementation */


#ifdef __cplusplus
extern "C" {
#define MAGIC_PARAM_REGEX_MAX		5
#define	MAGIC_PARAM_BYTES_MAX		6
#define	MAGIC_PARAM_ENCODING_MAX	7
#define	MAGIC_PARAM_ELF_SHSIZE_MAX	8
#define	MAGIC_PARAM_MAGWARN_MAX		9

int magic_setparam(magic_t, int, const void *);
int magic_getparam(magic_t, int, void *);
