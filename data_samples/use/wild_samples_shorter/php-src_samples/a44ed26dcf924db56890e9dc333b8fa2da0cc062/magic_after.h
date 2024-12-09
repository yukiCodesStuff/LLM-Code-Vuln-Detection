#define	MAGIC_NO_CHECK_FORTRAN	0x000000 /* Don't check ascii/fortran */
#define	MAGIC_NO_CHECK_TROFF	0x000000 /* Don't check ascii/troff */

#define MAGIC_VERSION		522	/* This implementation */


#ifdef __cplusplus
extern "C" {

int magic_version(void);
int magic_load(magic_t, const char *);
int magic_load_buffers(magic_t, void **, size_t *, size_t);

int magic_compile(magic_t, const char *);
int magic_list(magic_t, const char *);
int magic_errno(magic_t);

#define MAGIC_PARAM_INDIR_MAX		0
#define MAGIC_PARAM_NAME_MAX		1
#define MAGIC_PARAM_ELF_PHNUM_MAX	2
#define MAGIC_PARAM_ELF_SHNUM_MAX	3
#define MAGIC_PARAM_ELF_NOTES_MAX	4

int magic_setparam(magic_t, int, const void *);
int magic_getparam(magic_t, int, void *);

#ifdef __cplusplus
};
#endif
