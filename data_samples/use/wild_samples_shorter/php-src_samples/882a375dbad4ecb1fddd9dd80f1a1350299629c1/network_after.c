#include <errno.h>


#ifdef PHP_WIN32
# include <Ws2tcpip.h>
# include "win32/inet.h"
# define O_RDONLY _O_RDONLY