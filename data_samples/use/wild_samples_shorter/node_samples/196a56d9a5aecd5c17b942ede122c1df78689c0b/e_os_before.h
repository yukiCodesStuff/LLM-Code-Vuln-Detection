/* end vxworks */

/* system-specific variants defining ossl_sleep() */
#ifdef OPENSSL_SYS_UNIX
# include <unistd.h>
static ossl_inline void ossl_sleep(unsigned long millis)
{
# ifdef OPENSSL_SYS_VXWORKS