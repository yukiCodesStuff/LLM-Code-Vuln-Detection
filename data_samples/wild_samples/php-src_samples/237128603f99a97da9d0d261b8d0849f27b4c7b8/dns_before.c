	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &hostname_len) == FAILURE) {
		return;
	}

	addr = php_gethostbyname(hostname);

	RETVAL_STRING(addr, 0);
}
/* }}} */

/* {{{ proto array gethostbynamel(string hostname)
   Return a list of IP addresses that a given hostname resolves to. */
PHP_FUNCTION(gethostbynamel)
{
	char *hostname;
	int hostname_len;
	struct hostent *hp;
	struct in_addr in;
	int i;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &hostname_len) == FAILURE) {
		return;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL || hp->h_addr_list == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0 ; hp->h_addr_list[i] != 0 ; i++) {
		in = *(struct in_addr *) hp->h_addr_list[i];
		add_next_index_string(return_value, inet_ntoa(in), 1);
	}
}
/* }}} */

/* {{{ php_gethostbyname */
static char *php_gethostbyname(char *name)
{
	struct hostent *hp;
	struct in_addr in;

	hp = gethostbyname(name);

	if (!hp || !*(hp->h_addr_list)) {
		return estrdup(name);
	}

	memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));

	return estrdup(inet_ntoa(in));
}
/* }}} */

#if HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32)
# define PHP_DNS_NUM_TYPES	12	/* Number of DNS Types Supported by PHP currently */

# define PHP_DNS_A      0x00000001
# define PHP_DNS_NS     0x00000002
# define PHP_DNS_CNAME  0x00000010
# define PHP_DNS_SOA    0x00000020
# define PHP_DNS_PTR    0x00000800
# define PHP_DNS_HINFO  0x00001000
# define PHP_DNS_MX     0x00004000
# define PHP_DNS_TXT    0x00008000
# define PHP_DNS_A6     0x01000000
# define PHP_DNS_SRV    0x02000000
# define PHP_DNS_NAPTR  0x04000000
# define PHP_DNS_AAAA   0x08000000
# define PHP_DNS_ANY    0x10000000
# define PHP_DNS_ALL    (PHP_DNS_A|PHP_DNS_NS|PHP_DNS_CNAME|PHP_DNS_SOA|PHP_DNS_PTR|PHP_DNS_HINFO|PHP_DNS_MX|PHP_DNS_TXT|PHP_DNS_A6|PHP_DNS_SRV|PHP_DNS_NAPTR|PHP_DNS_AAAA)
#endif /* HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32) */

/* Note: These functions are defined in ext/standard/dns_win32.c for Windows! */
#if !defined(PHP_WIN32) && (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE)))
  
#ifndef HFIXEDSZ
#define HFIXEDSZ        12      /* fixed data in header <arpa/nameser.h> */
#endif /* HFIXEDSZ */

#ifndef QFIXEDSZ
#define QFIXEDSZ        4       /* fixed data in query <arpa/nameser.h> */
#endif /* QFIXEDSZ */

#undef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  1024

#ifndef MAXRESOURCERECORDS
#define MAXRESOURCERECORDS	64
#endif /* MAXRESOURCERECORDS */

typedef union {
	HEADER qb1;
	u_char qb2[65536];
} querybuf;

/* just a hack to free resources allocated by glibc in __res_nsend()
 * See also:
 *   res_thread_freeres() in glibc/resolv/res_init.c
 *   __libc_res_nsend()   in resolv/res_send.c
 * */

#if defined(__GLIBC__) && !defined(HAVE_DEPRECATED_DNS_FUNCS)
#define php_dns_free_res(__res__) _php_dns_free_res(__res__)
static void _php_dns_free_res(struct __res_state res) { /* {{{ */
	int ns;
	for (ns = 0; ns < MAXNS; ns++) {
		if (res._u._ext.nsaddrs[ns] != NULL) {
			free (res._u._ext.nsaddrs[ns]);
			res._u._ext.nsaddrs[ns] = NULL;
		}
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &hostname, &hostname_len) == FAILURE) {
		return;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL || hp->h_addr_list == NULL) {
		RETURN_FALSE;
	}

	array_init(return_value);

	for (i = 0 ; hp->h_addr_list[i] != 0 ; i++) {
		in = *(struct in_addr *) hp->h_addr_list[i];
		add_next_index_string(return_value, inet_ntoa(in), 1);
	}
}
/* }}} */

/* {{{ php_gethostbyname */
static char *php_gethostbyname(char *name)
{
	struct hostent *hp;
	struct in_addr in;

	hp = gethostbyname(name);

	if (!hp || !*(hp->h_addr_list)) {
		return estrdup(name);
	}

	memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));

	return estrdup(inet_ntoa(in));
}
/* }}} */

#if HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32)
# define PHP_DNS_NUM_TYPES	12	/* Number of DNS Types Supported by PHP currently */

# define PHP_DNS_A      0x00000001
# define PHP_DNS_NS     0x00000002
# define PHP_DNS_CNAME  0x00000010
# define PHP_DNS_SOA    0x00000020
# define PHP_DNS_PTR    0x00000800
# define PHP_DNS_HINFO  0x00001000
# define PHP_DNS_MX     0x00004000
# define PHP_DNS_TXT    0x00008000
# define PHP_DNS_A6     0x01000000
# define PHP_DNS_SRV    0x02000000
# define PHP_DNS_NAPTR  0x04000000
# define PHP_DNS_AAAA   0x08000000
# define PHP_DNS_ANY    0x10000000
# define PHP_DNS_ALL    (PHP_DNS_A|PHP_DNS_NS|PHP_DNS_CNAME|PHP_DNS_SOA|PHP_DNS_PTR|PHP_DNS_HINFO|PHP_DNS_MX|PHP_DNS_TXT|PHP_DNS_A6|PHP_DNS_SRV|PHP_DNS_NAPTR|PHP_DNS_AAAA)
#endif /* HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32) */

/* Note: These functions are defined in ext/standard/dns_win32.c for Windows! */
#if !defined(PHP_WIN32) && (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE)))
  
#ifndef HFIXEDSZ
#define HFIXEDSZ        12      /* fixed data in header <arpa/nameser.h> */
#endif /* HFIXEDSZ */

#ifndef QFIXEDSZ
#define QFIXEDSZ        4       /* fixed data in query <arpa/nameser.h> */
#endif /* QFIXEDSZ */

#undef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN  1024

#ifndef MAXRESOURCERECORDS
#define MAXRESOURCERECORDS	64
#endif /* MAXRESOURCERECORDS */

typedef union {
	HEADER qb1;
	u_char qb2[65536];
} querybuf;

/* just a hack to free resources allocated by glibc in __res_nsend()
 * See also:
 *   res_thread_freeres() in glibc/resolv/res_init.c
 *   __libc_res_nsend()   in resolv/res_send.c
 * */

#if defined(__GLIBC__) && !defined(HAVE_DEPRECATED_DNS_FUNCS)
#define php_dns_free_res(__res__) _php_dns_free_res(__res__)
static void _php_dns_free_res(struct __res_state res) { /* {{{ */
	int ns;
	for (ns = 0; ns < MAXNS; ns++) {
		if (res._u._ext.nsaddrs[ns] != NULL) {
			free (res._u._ext.nsaddrs[ns]);
			res._u._ext.nsaddrs[ns] = NULL;
		}