#include "php.h"

#include <stddef.h>
#include <errno.h>


#ifdef PHP_WIN32
# include <Ws2tcpip.h>
# define PHP_TIMEOUT_ERROR_VALUE		ETIMEDOUT
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 255
#endif

#if HAVE_GETADDRINFO
#ifdef HAVE_GAI_STRERROR
#  define PHP_GAI_STRERROR(x) (gai_strerror(x))
#else
#else
	if (!inet_aton(host, &in)) {
		/* XXX NOT THREAD SAFE (is safe under win32) */
		if(strlen(host) > MAXHOSTNAMELEN) {
			host_info = NULL;
			errno = E2BIG;
		} else {
			host_info = gethostbyname(host);
		}
		if (host_info == NULL) {
			if (error_string) {
				error_string = strpprintf(0, "php_network_getaddresses: gethostbyname failed. errno=%d", errno);
				php_error_docref(NULL, E_WARNING, "%s", (*error_string)->val);