#include "php.h"

#include <stddef.h>



#ifdef PHP_WIN32
# define PHP_TIMEOUT_ERROR_VALUE		ETIMEDOUT
#endif

#if HAVE_GETADDRINFO
#ifdef HAVE_GAI_STRERROR
#  define PHP_GAI_STRERROR(x) (gai_strerror(x))
#else
#else
	if (!inet_aton(host, &in)) {
		/* XXX NOT THREAD SAFE (is safe under win32) */
		host_info = gethostbyname(host);
		if (host_info == NULL) {
			if (error_string) {
				spprintf(error_string, 0, "php_network_getaddresses: gethostbyname failed. errno=%d", errno);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", *error_string);