/* end vxworks */

/* system-specific variants defining ossl_sleep() */
#if defined(OPENSSL_SYS_UNIX) || defined(__DJGPP__)
# include <unistd.h>
static ossl_inline void ossl_sleep(unsigned long millis)
{
# ifdef OPENSSL_SYS_VXWORKS