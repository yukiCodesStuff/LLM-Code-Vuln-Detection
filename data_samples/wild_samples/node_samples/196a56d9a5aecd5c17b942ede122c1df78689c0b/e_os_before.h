#   define EXIT(n)  exit((n) ? (((n) << 3) | 2 | 0x10000000 | 0x35a000) : 1)

#   define DEFAULT_HOME "SYS$LOGIN:"

#  else
     /* !defined VMS */
#   include <unistd.h>
#   include <sys/types.h>
#   ifdef OPENSSL_SYS_WIN32_CYGWIN
#    include <io.h>
#    include <fcntl.h>
#   endif

#   define LIST_SEPARATOR_CHAR ':'
#   define EXIT(n)             exit(n)
#  endif

# endif

/***********************************************/

# if defined(OPENSSL_SYS_WINDOWS)
#  if (_MSC_VER >= 1310) && !defined(_WIN32_WCE)
#   define open _open
#   define fdopen _fdopen
#   define close _close
#   ifndef strdup
#    define strdup _strdup
#   endif
#   define unlink _unlink
#   define fileno _fileno
#  endif
# else
#  include <strings.h>
# endif

/* vxworks */
# if defined(OPENSSL_SYS_VXWORKS)
#  include <ioLib.h>
#  include <tickLib.h>
#  include <sysLib.h>
#  include <vxWorks.h>
#  include <sockLib.h>
#  include <taskLib.h>

#  define TTY_STRUCT int
#  define sleep(a) taskDelay((a) * sysClkRateGet())

/*
 * NOTE: these are implemented by helpers in database app! if the database is
 * not linked, we need to implement them elsewhere
 */
struct hostent *gethostbyname(const char *name);
struct hostent *gethostbyaddr(const char *addr, int length, int type);
struct servent *getservbyname(const char *name, const char *proto);

# endif
/* end vxworks */

/* system-specific variants defining ossl_sleep() */
#ifdef OPENSSL_SYS_UNIX
# include <unistd.h>
static ossl_inline void ossl_sleep(unsigned long millis)
{
# ifdef OPENSSL_SYS_VXWORKS
    struct timespec ts;
    ts.tv_sec = (long int) (millis / 1000);
    ts.tv_nsec = (long int) (millis % 1000) * 1000000ul;
    nanosleep(&ts, NULL);
# elif defined(__TANDEM)
#  if !defined(_REENTRANT)
#   include <cextdecs.h(PROCESS_DELAY_)>
    /* HPNS does not support usleep for non threaded apps */
    PROCESS_DELAY_(millis * 1000);
#  elif defined(_SPT_MODEL_)
#   include <spthread.h>
#   include <spt_extensions.h>
    usleep(millis * 1000);
#  else
    usleep(millis * 1000);
#  endif
# else
    usleep(millis * 1000);
# endif
}