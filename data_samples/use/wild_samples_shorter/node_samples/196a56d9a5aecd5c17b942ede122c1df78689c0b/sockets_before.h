
# elif defined(OPENSSL_SYS_WINDOWS) || defined(OPENSSL_SYS_MSDOS)
#  if defined(__DJGPP__)
#   include <sys/socket.h>
#   include <sys/un.h>
#   include <tcp.h>
#   include <netdb.h>
#  define readsocket(s,b,n)       recv((s),(b),(n),0)
#  define writesocket(s,b,n)      send((s),(b),(n),0)
# elif defined(__DJGPP__)
#  define WATT32
#  define WATT32_NO_OLDIES
#  define closesocket(s)          close_s(s)
#  define readsocket(s,b,n)       read_s(s,b,n)
#  define writesocket(s,b,n)      send(s,b,n,0)
# elif defined(OPENSSL_SYS_VMS)