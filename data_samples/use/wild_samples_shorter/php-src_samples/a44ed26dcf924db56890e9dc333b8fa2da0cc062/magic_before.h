#define	MAGIC_NO_CHECK_FORTRAN	0x000000 /* Don't check ascii/fortran */
#define	MAGIC_NO_CHECK_TROFF	0x000000 /* Don't check ascii/troff */

#define MAGIC_VERSION		517	/* This implementation */


#ifdef __cplusplus
extern "C" {

int magic_version(void);
int magic_load(magic_t, const char *);
int magic_compile(magic_t, const char *);
int magic_list(magic_t, const char *);
int magic_errno(magic_t);

#ifdef __cplusplus
};
#endif
