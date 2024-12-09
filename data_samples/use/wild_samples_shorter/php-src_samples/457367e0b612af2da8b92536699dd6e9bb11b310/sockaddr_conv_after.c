#include <arpa/inet.h>
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 255
#endif

extern int php_string_to_if_index(const char *val, unsigned *out);

#if HAVE_IPV6
/* Sets addr by hostname, or by ip in string form (AF_INET6) */
	if (inet_aton(string, &tmp)) {
		sin->sin_addr.s_addr = tmp.s_addr;
	} else {
		if (strlen(string) > MAXHOSTNAMELEN || ! (host_entry = gethostbyname(string))) {
			/* Note: < -10000 indicates a host lookup error */
#ifdef PHP_WIN32
			PHP_SOCKET_ERROR(php_sock, "Host lookup failed", WSAGetLastError());
#else