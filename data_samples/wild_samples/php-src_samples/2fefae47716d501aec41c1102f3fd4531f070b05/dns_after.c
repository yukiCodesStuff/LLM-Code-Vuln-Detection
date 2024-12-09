	if (i < 0) {
		RETVAL_FALSE;
	}

	php_dns_free_handle(handle);
}
/* }}} */

#if HAVE_FULL_DNS_FUNCS

#define CHECKCP(n) do { \
	if (cp + n > end) { \
		return NULL; \
	} \
} while (0)

/* {{{ php_parserr */
static u_char *php_parserr(u_char *cp, u_char *end, querybuf *answer, int type_to_fetch, int store, int raw, zval **subarray)
{
	u_short type, class, dlen;
	u_long ttl;
	long n, i;
	u_short s;
	u_char *tp, *p;
	char name[MAXHOSTNAMELEN];
	int have_v6_break = 0, in_v6_break = 0;

	*subarray = NULL;

	n = dn_expand(answer->qb2, end, cp, name, sizeof(name) - 2);
	if (n < 0) {
		return NULL;
	}
	cp += n;

	CHECKCP(10);
	GETSHORT(type, cp);
	GETSHORT(class, cp);
	GETLONG(ttl, cp);
	GETSHORT(dlen, cp);
	CHECKCP(dlen);
	if (type_to_fetch != T_ANY && type != type_to_fetch) {
		cp += dlen;
		return cp;
	}

	if (!store) {
		cp += dlen;
		return cp;
	}

	ALLOC_INIT_ZVAL(*subarray);
	array_init(*subarray);

	add_assoc_string(*subarray, "host", name, 1);
	add_assoc_string(*subarray, "class", "IN", 1);
	add_assoc_long(*subarray, "ttl", ttl);

	if (raw) {
		add_assoc_long(*subarray, "type", type);
		add_assoc_stringl(*subarray, "data", (char*) cp, (uint) dlen, 1);
		cp += dlen;
		return cp;
	}

	switch (type) {
		case DNS_T_A:
			CHECKCP(4);
			add_assoc_string(*subarray, "type", "A", 1);
			snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
			add_assoc_string(*subarray, "ip", name, 1);
			cp += dlen;
			break;
		case DNS_T_MX:
			CHECKCP(2);
			add_assoc_string(*subarray, "type", "MX", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			/* no break; */
		case DNS_T_CNAME:
			if (type == DNS_T_CNAME) {
				add_assoc_string(*subarray, "type", "CNAME", 1);
			}
			/* no break; */
		case DNS_T_NS:
			if (type == DNS_T_NS) {
				add_assoc_string(*subarray, "type", "NS", 1);
			}
			/* no break; */
		case DNS_T_PTR:
			if (type == DNS_T_PTR) {
				add_assoc_string(*subarray, "type", "PTR", 1);
			}
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(*subarray, "type", "HINFO", 1);
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "cpu", (char*)cp, n, 1);
			cp += n;
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "os", (char*)cp, n, 1);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int l1 = 0, l2 = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp + l2 , cp + l1 + 1, n);
						add_next_index_stringl(entries, cp + l1 + 1, n, 1);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
				tp[l2] = '\0';
				cp += dlen;

				add_assoc_stringl(*subarray, "txt", tp, l2, 0);
				add_assoc_zval(*subarray, "entries", entries);
			}
			break;
		case DNS_T_SOA:
			add_assoc_string(*subarray, "type", "SOA", 1);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "mname", name, 1);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "rname", name, 1);
			CHECKCP(5*4);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "serial", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "refresh", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "retry", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "expire", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "minimum-ttl", n);
			break;
		case DNS_T_AAAA:
			tp = (u_char*)name;
			CHECKCP(8*2);
			for(i=0; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "type", "AAAA", 1);
			add_assoc_string(*subarray, "ipv6", name, 1);
			break;
		case DNS_T_A6:
			p = cp;
			add_assoc_string(*subarray, "type", "A6", 1);
			CHECKCP(1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(*subarray, "masklen", n);
			tp = (u_char*)name;
			if (n > 15) {
				have_v6_break = 1;
				in_v6_break = 1;
				tp[0] = ':';
				tp++;
			}
			if (n % 16 > 8) {
				/* Partial short */
				if (cp[0] != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					sprintf((char*)tp, "%x", cp[0] & 0xFF);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				CHECKCP(2);
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_NAPTR:
			CHECKCP(2*2);
			add_assoc_string(*subarray, "type", "NAPTR", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pref", n);

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "flags", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "services", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "regex", (char*)cp, n, 1);
			cp += n;

			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "replacement", name, 1);
			break;
		default:
			zval_ptr_dtor(subarray);
			*subarray = NULL;
			cp += dlen;
			break;
	}

	return cp;
}
/* }}} */

/* {{{ proto array|false dns_get_record(string hostname [, int type[, array authns, array addtl]])
   Get any Resource Record corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_record)
{
	char *hostname;
	int hostname_len;
	long type_param = PHP_DNS_ANY;
	zval *authns = NULL, *addtl = NULL;
	int type_to_fetch;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif
	HEADER *hp;
	querybuf answer;
	u_char *cp = NULL, *end = NULL;
	int n, qd, an, ns = 0, ar = 0;
	int type, first_query = 1, store_results = 1;
	zend_bool raw = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz!z!b",
			&hostname, &hostname_len, &type_param, &authns, &addtl, &raw) == FAILURE) {
		return;
	}

	if (authns) {
		zval_dtor(authns);
		array_init(authns);
	}
	if (addtl) {
		zval_dtor(addtl);
		array_init(addtl);
	}

	if (!raw) {
		if ((type_param & ~PHP_DNS_ALL) && (type_param != PHP_DNS_ANY)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type '%ld' not supported", type_param);
			RETURN_FALSE;
		}
	} else {
		if ((type_param < 1) || (type_param > 0xFFFF)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Numeric DNS record type must be between 1 and 65535, '%ld' given", type_param);
			RETURN_FALSE;
		}
	}

	/* Initialize the return array */
	array_init(return_value);

	/* - We emulate an or'ed type mask by querying type by type. (Steps 0 - NUMTYPES-1 )
	 *   If additional info is wanted we check again with DNS_T_ANY (step NUMTYPES / NUMTYPES+1 )
	 *   store_results is used to skip storing the results retrieved in step
	 *   NUMTYPES+1 when results were already fetched.
	 * - In case of PHP_DNS_ANY we use the directly fetch DNS_T_ANY. (step NUMTYPES+1 )
	 * - In case of raw mode, we query only the requestd type instead of looping type by type
	 *   before going with the additional info stuff.
	 */

	if (raw) {
		type = -1;
	} else if (type_param == PHP_DNS_ANY) {
		type = PHP_DNS_NUM_TYPES + 1;
	} else {
		type = 0;
	}

	for ( ;
		type < (addtl ? (PHP_DNS_NUM_TYPES + 2) : PHP_DNS_NUM_TYPES) || first_query;
		type++
	) {
		first_query = 0;
		switch (type) {
			case -1: /* raw */
				type_to_fetch = type_param;
				/* skip over the rest and go directly to additional records */
				type = PHP_DNS_NUM_TYPES - 1;
				break;
			case 0:
				type_to_fetch = type_param&PHP_DNS_A     ? DNS_T_A     : 0;
				break;
			case 1:
				type_to_fetch = type_param&PHP_DNS_NS    ? DNS_T_NS    : 0;
				break;
			case 2:
				type_to_fetch = type_param&PHP_DNS_CNAME ? DNS_T_CNAME : 0;
				break;
			case 3:
				type_to_fetch = type_param&PHP_DNS_SOA   ? DNS_T_SOA   : 0;
				break;
			case 4:
				type_to_fetch = type_param&PHP_DNS_PTR   ? DNS_T_PTR   : 0;
				break;
			case 5:
				type_to_fetch = type_param&PHP_DNS_HINFO ? DNS_T_HINFO : 0;
				break;
			case 6:
				type_to_fetch = type_param&PHP_DNS_MX    ? DNS_T_MX    : 0;
				break;
			case 7:
				type_to_fetch = type_param&PHP_DNS_TXT   ? DNS_T_TXT   : 0;
				break;
			case 8:
				type_to_fetch = type_param&PHP_DNS_AAAA	 ? DNS_T_AAAA  : 0;
				break;
			case 9:
				type_to_fetch = type_param&PHP_DNS_SRV   ? DNS_T_SRV   : 0;
				break;
			case 10:
				type_to_fetch = type_param&PHP_DNS_NAPTR ? DNS_T_NAPTR : 0;
				break;
			case 11:
				type_to_fetch = type_param&PHP_DNS_A6	 ? DNS_T_A6 : 0;
				break;
			case PHP_DNS_NUM_TYPES:
				store_results = 0;
				continue;
			default:
			case (PHP_DNS_NUM_TYPES + 1):
				type_to_fetch = DNS_T_ANY;
				break;
		}

		if (type_to_fetch) {
#if defined(HAVE_DNS_SEARCH)
			handle = dns_open(NULL);
			if (handle == NULL) {
				zval_dtor(return_value);
				RETURN_FALSE;
			}
#elif defined(HAVE_RES_NSEARCH)
		    memset(&state, 0, sizeof(state));
		    if (res_ninit(handle)) {
		    	zval_dtor(return_value);
				RETURN_FALSE;
			}
#else
			res_init();
#endif

			n = php_dns_search(handle, hostname, C_IN, type_to_fetch, answer.qb2, sizeof answer);

			if (n < 0) {
				php_dns_free_handle(handle);
				continue;
			}

			cp = answer.qb2 + HFIXEDSZ;
			end = answer.qb2 + n;
			hp = (HEADER *)&answer;
			qd = ntohs(hp->qdcount);
			an = ntohs(hp->ancount);
			ns = ntohs(hp->nscount);
			ar = ntohs(hp->arcount);

			/* Skip QD entries, they're only used by dn_expand later on */
			while (qd-- > 0) {
				n = dn_skipname(cp, end);
				if (n < 0) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to parse DNS data received");
					zval_dtor(return_value);
					php_dns_free_handle(handle);
					RETURN_FALSE;
				}
				cp += n + QFIXEDSZ;
			}

			/* YAY! Our real answers! */
			while (an-- && cp && cp < end) {
				zval *retval;

				cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, raw, &retval);
				if (retval != NULL && store_results) {
					add_next_index_zval(return_value, retval);
				}
			}

			if (authns || addtl) {
				/* List of Authoritative Name Servers
				 * Process when only requesting addtl so that we can skip through the section
				 */
				while (ns-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(authns, retval);
					}
				}
			}

			if (addtl) {
				/* Additional records associated with authoritative name servers */
				while (ar-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, 1, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(addtl, retval);
					}
				}
			}
			php_dns_free_handle(handle);
		}
	}
}
/* }}} */

/* {{{ proto bool dns_get_mx(string hostname, array mxhosts [, array weight])
   Get MX records corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_mx)
{
	char *hostname;
	int hostname_len;
	zval *mx_list, *weight_list = NULL;
	int count, qdc;
	u_short type, weight;
	u_char ans[MAXPACKET];
	char buf[MAXHOSTNAMELEN];
	HEADER *hp;
	u_char *cp, *end;
	int i;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &hostname, &hostname_len, &mx_list, &weight_list) == FAILURE) {
		return;
	}

	zval_dtor(mx_list);
	array_init(mx_list);

	if (weight_list) {
		zval_dtor(weight_list);
		array_init(weight_list);
	}

#if defined(HAVE_DNS_SEARCH)
	handle = dns_open(NULL);
	if (handle == NULL) {
		RETURN_FALSE;
	}
#elif defined(HAVE_RES_NSEARCH)
    memset(&state, 0, sizeof(state));
    if (res_ninit(handle)) {
			RETURN_FALSE;
	}
#else
	res_init();
#endif

	i = php_dns_search(handle, hostname, C_IN, DNS_T_MX, (u_char *)&ans, sizeof(ans));
	if (i < 0) {
		RETURN_FALSE;
	}
	if (i > (int)sizeof(ans)) {
		i = sizeof(ans);
	}
	hp = (HEADER *)&ans;
	cp = (u_char *)&ans + HFIXEDSZ;
	end = (u_char *)&ans +i;
	for (qdc = ntohs((unsigned short)hp->qdcount); qdc--; cp += i + QFIXEDSZ) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
	}
	count = ntohs((unsigned short)hp->ancount);
	while (--count >= 0 && cp < end) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		GETSHORT(type, cp);
		cp += INT16SZ + INT32SZ;
		GETSHORT(i, cp);
		if (type != DNS_T_MX) {
			cp += i;
			continue;
		}
		GETSHORT(weight, cp);
		if ((i = dn_expand(ans, end, cp, buf, sizeof(buf)-1)) < 0) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		add_next_index_string(mx_list, buf, 1);
		if (weight_list) {
			add_next_index_long(weight_list, weight);
		}
	}
	php_dns_free_handle(handle);
	RETURN_TRUE;
}
/* }}} */
#endif /* HAVE_FULL_DNS_FUNCS */
#endif /* !defined(PHP_WIN32) && (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE))) */

#if HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32)
PHP_MINIT_FUNCTION(dns) {
	REGISTER_LONG_CONSTANT("DNS_A",     PHP_DNS_A,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NS",    PHP_DNS_NS,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_CNAME", PHP_DNS_CNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SOA",   PHP_DNS_SOA,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_PTR",   PHP_DNS_PTR,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_HINFO", PHP_DNS_HINFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_MX",    PHP_DNS_MX,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_TXT",   PHP_DNS_TXT,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SRV",   PHP_DNS_SRV,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NAPTR", PHP_DNS_NAPTR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_AAAA",  PHP_DNS_AAAA,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_A6",    PHP_DNS_A6,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ANY",   PHP_DNS_ANY,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ALL",   PHP_DNS_ALL,   CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
#endif /* HAVE_FULL_DNS_FUNCS */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
{
	u_short type, class, dlen;
	u_long ttl;
	long n, i;
	u_short s;
	u_char *tp, *p;
	char name[MAXHOSTNAMELEN];
	int have_v6_break = 0, in_v6_break = 0;

	*subarray = NULL;

	n = dn_expand(answer->qb2, end, cp, name, sizeof(name) - 2);
	if (n < 0) {
		return NULL;
	}
	switch (type) {
		case DNS_T_A:
			CHECKCP(4);
			add_assoc_string(*subarray, "type", "A", 1);
			snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
			add_assoc_string(*subarray, "ip", name, 1);
			cp += dlen;
			break;
		case DNS_T_MX:
			CHECKCP(2);
			add_assoc_string(*subarray, "type", "MX", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			/* no break; */
		case DNS_T_CNAME:
			if (type == DNS_T_CNAME) {
				add_assoc_string(*subarray, "type", "CNAME", 1);
			}
			/* no break; */
		case DNS_T_NS:
			if (type == DNS_T_NS) {
				add_assoc_string(*subarray, "type", "NS", 1);
			}
			/* no break; */
		case DNS_T_PTR:
			if (type == DNS_T_PTR) {
				add_assoc_string(*subarray, "type", "PTR", 1);
			}
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(*subarray, "type", "HINFO", 1);
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "cpu", (char*)cp, n, 1);
			cp += n;
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "os", (char*)cp, n, 1);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int l1 = 0, l2 = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp + l2 , cp + l1 + 1, n);
						add_next_index_stringl(entries, cp + l1 + 1, n, 1);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
				tp[l2] = '\0';
				cp += dlen;

				add_assoc_stringl(*subarray, "txt", tp, l2, 0);
				add_assoc_zval(*subarray, "entries", entries);
			}
			break;
		case DNS_T_SOA:
			add_assoc_string(*subarray, "type", "SOA", 1);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "mname", name, 1);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "rname", name, 1);
			CHECKCP(5*4);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "serial", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "refresh", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "retry", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "expire", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "minimum-ttl", n);
			break;
		case DNS_T_AAAA:
			tp = (u_char*)name;
			CHECKCP(8*2);
			for(i=0; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "type", "AAAA", 1);
			add_assoc_string(*subarray, "ipv6", name, 1);
			break;
		case DNS_T_A6:
			p = cp;
			add_assoc_string(*subarray, "type", "A6", 1);
			CHECKCP(1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(*subarray, "masklen", n);
			tp = (u_char*)name;
			if (n > 15) {
				have_v6_break = 1;
				in_v6_break = 1;
				tp[0] = ':';
				tp++;
			}
			if (n % 16 > 8) {
				/* Partial short */
				if (cp[0] != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					sprintf((char*)tp, "%x", cp[0] & 0xFF);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				CHECKCP(2);
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_NAPTR:
			CHECKCP(2*2);
			add_assoc_string(*subarray, "type", "NAPTR", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pref", n);

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "flags", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "services", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "regex", (char*)cp, n, 1);
			cp += n;

			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "replacement", name, 1);
			break;
		default:
			zval_ptr_dtor(subarray);
			*subarray = NULL;
			cp += dlen;
			break;
	}

	return cp;
}
/* }}} */

/* {{{ proto array|false dns_get_record(string hostname [, int type[, array authns, array addtl]])
   Get any Resource Record corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_record)
{
	char *hostname;
	int hostname_len;
	long type_param = PHP_DNS_ANY;
	zval *authns = NULL, *addtl = NULL;
	int type_to_fetch;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif
	HEADER *hp;
	querybuf answer;
	u_char *cp = NULL, *end = NULL;
	int n, qd, an, ns = 0, ar = 0;
	int type, first_query = 1, store_results = 1;
	zend_bool raw = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz!z!b",
			&hostname, &hostname_len, &type_param, &authns, &addtl, &raw) == FAILURE) {
		return;
	}

	if (authns) {
		zval_dtor(authns);
		array_init(authns);
	}
	if (addtl) {
		zval_dtor(addtl);
		array_init(addtl);
	}

	if (!raw) {
		if ((type_param & ~PHP_DNS_ALL) && (type_param != PHP_DNS_ANY)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type '%ld' not supported", type_param);
			RETURN_FALSE;
		}
	} else {
		if ((type_param < 1) || (type_param > 0xFFFF)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Numeric DNS record type must be between 1 and 65535, '%ld' given", type_param);
			RETURN_FALSE;
		}
	}

	/* Initialize the return array */
	array_init(return_value);

	/* - We emulate an or'ed type mask by querying type by type. (Steps 0 - NUMTYPES-1 )
	 *   If additional info is wanted we check again with DNS_T_ANY (step NUMTYPES / NUMTYPES+1 )
	 *   store_results is used to skip storing the results retrieved in step
	 *   NUMTYPES+1 when results were already fetched.
	 * - In case of PHP_DNS_ANY we use the directly fetch DNS_T_ANY. (step NUMTYPES+1 )
	 * - In case of raw mode, we query only the requestd type instead of looping type by type
	 *   before going with the additional info stuff.
	 */

	if (raw) {
		type = -1;
	} else if (type_param == PHP_DNS_ANY) {
		type = PHP_DNS_NUM_TYPES + 1;
	} else {
		type = 0;
	}

	for ( ;
		type < (addtl ? (PHP_DNS_NUM_TYPES + 2) : PHP_DNS_NUM_TYPES) || first_query;
		type++
	) {
		first_query = 0;
		switch (type) {
			case -1: /* raw */
				type_to_fetch = type_param;
				/* skip over the rest and go directly to additional records */
				type = PHP_DNS_NUM_TYPES - 1;
				break;
			case 0:
				type_to_fetch = type_param&PHP_DNS_A     ? DNS_T_A     : 0;
				break;
			case 1:
				type_to_fetch = type_param&PHP_DNS_NS    ? DNS_T_NS    : 0;
				break;
			case 2:
				type_to_fetch = type_param&PHP_DNS_CNAME ? DNS_T_CNAME : 0;
				break;
			case 3:
				type_to_fetch = type_param&PHP_DNS_SOA   ? DNS_T_SOA   : 0;
				break;
			case 4:
				type_to_fetch = type_param&PHP_DNS_PTR   ? DNS_T_PTR   : 0;
				break;
			case 5:
				type_to_fetch = type_param&PHP_DNS_HINFO ? DNS_T_HINFO : 0;
				break;
			case 6:
				type_to_fetch = type_param&PHP_DNS_MX    ? DNS_T_MX    : 0;
				break;
			case 7:
				type_to_fetch = type_param&PHP_DNS_TXT   ? DNS_T_TXT   : 0;
				break;
			case 8:
				type_to_fetch = type_param&PHP_DNS_AAAA	 ? DNS_T_AAAA  : 0;
				break;
			case 9:
				type_to_fetch = type_param&PHP_DNS_SRV   ? DNS_T_SRV   : 0;
				break;
			case 10:
				type_to_fetch = type_param&PHP_DNS_NAPTR ? DNS_T_NAPTR : 0;
				break;
			case 11:
				type_to_fetch = type_param&PHP_DNS_A6	 ? DNS_T_A6 : 0;
				break;
			case PHP_DNS_NUM_TYPES:
				store_results = 0;
				continue;
			default:
			case (PHP_DNS_NUM_TYPES + 1):
				type_to_fetch = DNS_T_ANY;
				break;
		}

		if (type_to_fetch) {
#if defined(HAVE_DNS_SEARCH)
			handle = dns_open(NULL);
			if (handle == NULL) {
				zval_dtor(return_value);
				RETURN_FALSE;
			}
#elif defined(HAVE_RES_NSEARCH)
		    memset(&state, 0, sizeof(state));
		    if (res_ninit(handle)) {
		    	zval_dtor(return_value);
				RETURN_FALSE;
			}
#else
			res_init();
#endif

			n = php_dns_search(handle, hostname, C_IN, type_to_fetch, answer.qb2, sizeof answer);

			if (n < 0) {
				php_dns_free_handle(handle);
				continue;
			}

			cp = answer.qb2 + HFIXEDSZ;
			end = answer.qb2 + n;
			hp = (HEADER *)&answer;
			qd = ntohs(hp->qdcount);
			an = ntohs(hp->ancount);
			ns = ntohs(hp->nscount);
			ar = ntohs(hp->arcount);

			/* Skip QD entries, they're only used by dn_expand later on */
			while (qd-- > 0) {
				n = dn_skipname(cp, end);
				if (n < 0) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to parse DNS data received");
					zval_dtor(return_value);
					php_dns_free_handle(handle);
					RETURN_FALSE;
				}
				cp += n + QFIXEDSZ;
			}

			/* YAY! Our real answers! */
			while (an-- && cp && cp < end) {
				zval *retval;

				cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, raw, &retval);
				if (retval != NULL && store_results) {
					add_next_index_zval(return_value, retval);
				}
			}

			if (authns || addtl) {
				/* List of Authoritative Name Servers
				 * Process when only requesting addtl so that we can skip through the section
				 */
				while (ns-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(authns, retval);
					}
				}
			}

			if (addtl) {
				/* Additional records associated with authoritative name servers */
				while (ar-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, 1, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(addtl, retval);
					}
				}
			}
			php_dns_free_handle(handle);
		}
	}
}
/* }}} */

/* {{{ proto bool dns_get_mx(string hostname, array mxhosts [, array weight])
   Get MX records corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_mx)
{
	char *hostname;
	int hostname_len;
	zval *mx_list, *weight_list = NULL;
	int count, qdc;
	u_short type, weight;
	u_char ans[MAXPACKET];
	char buf[MAXHOSTNAMELEN];
	HEADER *hp;
	u_char *cp, *end;
	int i;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &hostname, &hostname_len, &mx_list, &weight_list) == FAILURE) {
		return;
	}

	zval_dtor(mx_list);
	array_init(mx_list);

	if (weight_list) {
		zval_dtor(weight_list);
		array_init(weight_list);
	}

#if defined(HAVE_DNS_SEARCH)
	handle = dns_open(NULL);
	if (handle == NULL) {
		RETURN_FALSE;
	}
#elif defined(HAVE_RES_NSEARCH)
    memset(&state, 0, sizeof(state));
    if (res_ninit(handle)) {
			RETURN_FALSE;
	}
#else
	res_init();
#endif

	i = php_dns_search(handle, hostname, C_IN, DNS_T_MX, (u_char *)&ans, sizeof(ans));
	if (i < 0) {
		RETURN_FALSE;
	}
	if (i > (int)sizeof(ans)) {
		i = sizeof(ans);
	}
	hp = (HEADER *)&ans;
	cp = (u_char *)&ans + HFIXEDSZ;
	end = (u_char *)&ans +i;
	for (qdc = ntohs((unsigned short)hp->qdcount); qdc--; cp += i + QFIXEDSZ) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
	}
	count = ntohs((unsigned short)hp->ancount);
	while (--count >= 0 && cp < end) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		GETSHORT(type, cp);
		cp += INT16SZ + INT32SZ;
		GETSHORT(i, cp);
		if (type != DNS_T_MX) {
			cp += i;
			continue;
		}
		GETSHORT(weight, cp);
		if ((i = dn_expand(ans, end, cp, buf, sizeof(buf)-1)) < 0) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		add_next_index_string(mx_list, buf, 1);
		if (weight_list) {
			add_next_index_long(weight_list, weight);
		}
	}
	php_dns_free_handle(handle);
	RETURN_TRUE;
}
/* }}} */
#endif /* HAVE_FULL_DNS_FUNCS */
#endif /* !defined(PHP_WIN32) && (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE))) */

#if HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32)
PHP_MINIT_FUNCTION(dns) {
	REGISTER_LONG_CONSTANT("DNS_A",     PHP_DNS_A,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NS",    PHP_DNS_NS,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_CNAME", PHP_DNS_CNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SOA",   PHP_DNS_SOA,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_PTR",   PHP_DNS_PTR,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_HINFO", PHP_DNS_HINFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_MX",    PHP_DNS_MX,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_TXT",   PHP_DNS_TXT,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SRV",   PHP_DNS_SRV,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NAPTR", PHP_DNS_NAPTR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_AAAA",  PHP_DNS_AAAA,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_A6",    PHP_DNS_A6,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ANY",   PHP_DNS_ANY,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ALL",   PHP_DNS_ALL,   CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
#endif /* HAVE_FULL_DNS_FUNCS */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
			if (type == DNS_T_PTR) {
				add_assoc_string(*subarray, "type", "PTR", 1);
			}
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(*subarray, "type", "HINFO", 1);
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "cpu", (char*)cp, n, 1);
			cp += n;
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "os", (char*)cp, n, 1);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int l1 = 0, l2 = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp + l2 , cp + l1 + 1, n);
						add_next_index_stringl(entries, cp + l1 + 1, n, 1);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(*subarray, "type", "HINFO", 1);
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "cpu", (char*)cp, n, 1);
			cp += n;
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "os", (char*)cp, n, 1);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int l1 = 0, l2 = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp + l2 , cp + l1 + 1, n);
						add_next_index_stringl(entries, cp + l1 + 1, n, 1);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
				tp[l2] = '\0';
				cp += dlen;

				add_assoc_stringl(*subarray, "txt", tp, l2, 0);
				add_assoc_zval(*subarray, "entries", entries);
			}
			{
				int l1 = 0, l2 = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp + l2 , cp + l1 + 1, n);
						add_next_index_stringl(entries, cp + l1 + 1, n, 1);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "rname", name, 1);
			CHECKCP(5*4);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "serial", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "refresh", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "retry", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "expire", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "minimum-ttl", n);
			break;
		case DNS_T_AAAA:
			tp = (u_char*)name;
			CHECKCP(8*2);
			for(i=0; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "type", "AAAA", 1);
			add_assoc_string(*subarray, "ipv6", name, 1);
			break;
		case DNS_T_A6:
			p = cp;
			add_assoc_string(*subarray, "type", "A6", 1);
			CHECKCP(1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(*subarray, "masklen", n);
			tp = (u_char*)name;
			if (n > 15) {
				have_v6_break = 1;
				in_v6_break = 1;
				tp[0] = ':';
				tp++;
			}
			if (n % 16 > 8) {
				/* Partial short */
				if (cp[0] != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					sprintf((char*)tp, "%x", cp[0] & 0xFF);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				CHECKCP(2);
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_NAPTR:
			CHECKCP(2*2);
			add_assoc_string(*subarray, "type", "NAPTR", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pref", n);

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "flags", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "services", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "regex", (char*)cp, n, 1);
			cp += n;

			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "replacement", name, 1);
			break;
		default:
			zval_ptr_dtor(subarray);
			*subarray = NULL;
			cp += dlen;
			break;
	}

	return cp;
}
/* }}} */

/* {{{ proto array|false dns_get_record(string hostname [, int type[, array authns, array addtl]])
   Get any Resource Record corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_record)
{
	char *hostname;
	int hostname_len;
	long type_param = PHP_DNS_ANY;
	zval *authns = NULL, *addtl = NULL;
	int type_to_fetch;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif
	HEADER *hp;
	querybuf answer;
	u_char *cp = NULL, *end = NULL;
	int n, qd, an, ns = 0, ar = 0;
	int type, first_query = 1, store_results = 1;
	zend_bool raw = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lz!z!b",
			&hostname, &hostname_len, &type_param, &authns, &addtl, &raw) == FAILURE) {
		return;
	}

	if (authns) {
		zval_dtor(authns);
		array_init(authns);
	}
	if (addtl) {
		zval_dtor(addtl);
		array_init(addtl);
	}

	if (!raw) {
		if ((type_param & ~PHP_DNS_ALL) && (type_param != PHP_DNS_ANY)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type '%ld' not supported", type_param);
			RETURN_FALSE;
		}
	} else {
		if ((type_param < 1) || (type_param > 0xFFFF)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
				"Numeric DNS record type must be between 1 and 65535, '%ld' given", type_param);
			RETURN_FALSE;
		}
	}

	/* Initialize the return array */
	array_init(return_value);

	/* - We emulate an or'ed type mask by querying type by type. (Steps 0 - NUMTYPES-1 )
	 *   If additional info is wanted we check again with DNS_T_ANY (step NUMTYPES / NUMTYPES+1 )
	 *   store_results is used to skip storing the results retrieved in step
	 *   NUMTYPES+1 when results were already fetched.
	 * - In case of PHP_DNS_ANY we use the directly fetch DNS_T_ANY. (step NUMTYPES+1 )
	 * - In case of raw mode, we query only the requestd type instead of looping type by type
	 *   before going with the additional info stuff.
	 */

	if (raw) {
		type = -1;
	} else if (type_param == PHP_DNS_ANY) {
		type = PHP_DNS_NUM_TYPES + 1;
	} else {
		type = 0;
	}

	for ( ;
		type < (addtl ? (PHP_DNS_NUM_TYPES + 2) : PHP_DNS_NUM_TYPES) || first_query;
		type++
	) {
		first_query = 0;
		switch (type) {
			case -1: /* raw */
				type_to_fetch = type_param;
				/* skip over the rest and go directly to additional records */
				type = PHP_DNS_NUM_TYPES - 1;
				break;
			case 0:
				type_to_fetch = type_param&PHP_DNS_A     ? DNS_T_A     : 0;
				break;
			case 1:
				type_to_fetch = type_param&PHP_DNS_NS    ? DNS_T_NS    : 0;
				break;
			case 2:
				type_to_fetch = type_param&PHP_DNS_CNAME ? DNS_T_CNAME : 0;
				break;
			case 3:
				type_to_fetch = type_param&PHP_DNS_SOA   ? DNS_T_SOA   : 0;
				break;
			case 4:
				type_to_fetch = type_param&PHP_DNS_PTR   ? DNS_T_PTR   : 0;
				break;
			case 5:
				type_to_fetch = type_param&PHP_DNS_HINFO ? DNS_T_HINFO : 0;
				break;
			case 6:
				type_to_fetch = type_param&PHP_DNS_MX    ? DNS_T_MX    : 0;
				break;
			case 7:
				type_to_fetch = type_param&PHP_DNS_TXT   ? DNS_T_TXT   : 0;
				break;
			case 8:
				type_to_fetch = type_param&PHP_DNS_AAAA	 ? DNS_T_AAAA  : 0;
				break;
			case 9:
				type_to_fetch = type_param&PHP_DNS_SRV   ? DNS_T_SRV   : 0;
				break;
			case 10:
				type_to_fetch = type_param&PHP_DNS_NAPTR ? DNS_T_NAPTR : 0;
				break;
			case 11:
				type_to_fetch = type_param&PHP_DNS_A6	 ? DNS_T_A6 : 0;
				break;
			case PHP_DNS_NUM_TYPES:
				store_results = 0;
				continue;
			default:
			case (PHP_DNS_NUM_TYPES + 1):
				type_to_fetch = DNS_T_ANY;
				break;
		}

		if (type_to_fetch) {
#if defined(HAVE_DNS_SEARCH)
			handle = dns_open(NULL);
			if (handle == NULL) {
				zval_dtor(return_value);
				RETURN_FALSE;
			}
#elif defined(HAVE_RES_NSEARCH)
		    memset(&state, 0, sizeof(state));
		    if (res_ninit(handle)) {
		    	zval_dtor(return_value);
				RETURN_FALSE;
			}
#else
			res_init();
#endif

			n = php_dns_search(handle, hostname, C_IN, type_to_fetch, answer.qb2, sizeof answer);

			if (n < 0) {
				php_dns_free_handle(handle);
				continue;
			}

			cp = answer.qb2 + HFIXEDSZ;
			end = answer.qb2 + n;
			hp = (HEADER *)&answer;
			qd = ntohs(hp->qdcount);
			an = ntohs(hp->ancount);
			ns = ntohs(hp->nscount);
			ar = ntohs(hp->arcount);

			/* Skip QD entries, they're only used by dn_expand later on */
			while (qd-- > 0) {
				n = dn_skipname(cp, end);
				if (n < 0) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to parse DNS data received");
					zval_dtor(return_value);
					php_dns_free_handle(handle);
					RETURN_FALSE;
				}
				cp += n + QFIXEDSZ;
			}

			/* YAY! Our real answers! */
			while (an-- && cp && cp < end) {
				zval *retval;

				cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, raw, &retval);
				if (retval != NULL && store_results) {
					add_next_index_zval(return_value, retval);
				}
			}

			if (authns || addtl) {
				/* List of Authoritative Name Servers
				 * Process when only requesting addtl so that we can skip through the section
				 */
				while (ns-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(authns, retval);
					}
				}
			}

			if (addtl) {
				/* Additional records associated with authoritative name servers */
				while (ar-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, 1, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(addtl, retval);
					}
				}
			}
			php_dns_free_handle(handle);
		}
	}
}
/* }}} */

/* {{{ proto bool dns_get_mx(string hostname, array mxhosts [, array weight])
   Get MX records corresponding to a given Internet host name */
PHP_FUNCTION(dns_get_mx)
{
	char *hostname;
	int hostname_len;
	zval *mx_list, *weight_list = NULL;
	int count, qdc;
	u_short type, weight;
	u_char ans[MAXPACKET];
	char buf[MAXHOSTNAMELEN];
	HEADER *hp;
	u_char *cp, *end;
	int i;
#if defined(HAVE_DNS_SEARCH)
	struct sockaddr_storage from;
	uint32_t fromsize = sizeof(from);
	dns_handle_t handle;
#elif defined(HAVE_RES_NSEARCH)
	struct __res_state state;
	struct __res_state *handle = &state;
#endif

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|z", &hostname, &hostname_len, &mx_list, &weight_list) == FAILURE) {
		return;
	}

	zval_dtor(mx_list);
	array_init(mx_list);

	if (weight_list) {
		zval_dtor(weight_list);
		array_init(weight_list);
	}

#if defined(HAVE_DNS_SEARCH)
	handle = dns_open(NULL);
	if (handle == NULL) {
		RETURN_FALSE;
	}
#elif defined(HAVE_RES_NSEARCH)
    memset(&state, 0, sizeof(state));
    if (res_ninit(handle)) {
			RETURN_FALSE;
	}
#else
	res_init();
#endif

	i = php_dns_search(handle, hostname, C_IN, DNS_T_MX, (u_char *)&ans, sizeof(ans));
	if (i < 0) {
		RETURN_FALSE;
	}
	if (i > (int)sizeof(ans)) {
		i = sizeof(ans);
	}
	hp = (HEADER *)&ans;
	cp = (u_char *)&ans + HFIXEDSZ;
	end = (u_char *)&ans +i;
	for (qdc = ntohs((unsigned short)hp->qdcount); qdc--; cp += i + QFIXEDSZ) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
	}
	count = ntohs((unsigned short)hp->ancount);
	while (--count >= 0 && cp < end) {
		if ((i = dn_skipname(cp, end)) < 0 ) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		GETSHORT(type, cp);
		cp += INT16SZ + INT32SZ;
		GETSHORT(i, cp);
		if (type != DNS_T_MX) {
			cp += i;
			continue;
		}
		GETSHORT(weight, cp);
		if ((i = dn_expand(ans, end, cp, buf, sizeof(buf)-1)) < 0) {
			php_dns_free_handle(handle);
			RETURN_FALSE;
		}
		cp += i;
		add_next_index_string(mx_list, buf, 1);
		if (weight_list) {
			add_next_index_long(weight_list, weight);
		}
	}
	php_dns_free_handle(handle);
	RETURN_TRUE;
}
/* }}} */
#endif /* HAVE_FULL_DNS_FUNCS */
#endif /* !defined(PHP_WIN32) && (HAVE_DNS_SEARCH_FUNC && !(defined(__BEOS__) || defined(NETWARE))) */

#if HAVE_FULL_DNS_FUNCS || defined(PHP_WIN32)
PHP_MINIT_FUNCTION(dns) {
	REGISTER_LONG_CONSTANT("DNS_A",     PHP_DNS_A,     CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NS",    PHP_DNS_NS,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_CNAME", PHP_DNS_CNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SOA",   PHP_DNS_SOA,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_PTR",   PHP_DNS_PTR,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_HINFO", PHP_DNS_HINFO, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_MX",    PHP_DNS_MX,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_TXT",   PHP_DNS_TXT,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_SRV",   PHP_DNS_SRV,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_NAPTR", PHP_DNS_NAPTR, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_AAAA",  PHP_DNS_AAAA,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_A6",    PHP_DNS_A6,    CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ANY",   PHP_DNS_ANY,   CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("DNS_ALL",   PHP_DNS_ALL,   CONST_CS | CONST_PERSISTENT);
	return SUCCESS;
}
#endif /* HAVE_FULL_DNS_FUNCS */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "type", "AAAA", 1);
			add_assoc_string(*subarray, "ipv6", name, 1);
			break;
		case DNS_T_A6:
			p = cp;
			add_assoc_string(*subarray, "type", "A6", 1);
			CHECKCP(1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(*subarray, "masklen", n);
			tp = (u_char*)name;
			if (n > 15) {
				have_v6_break = 1;
				in_v6_break = 1;
				tp[0] = ':';
				tp++;
			}
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				CHECKCP(2);
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
						tp[0] = ':';
						tp++;
					}
					tp += sprintf((char*)tp,"%x",s);
				} else {
					if (!have_v6_break) {
						have_v6_break = 1;
						in_v6_break = 1;
						tp[0] = ':';
						tp++;
					} else if (!in_v6_break) {
						tp[0] = ':';
						tp++;
						tp[0] = '0';
						tp++;
					}
				}
			}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_NAPTR:
			CHECKCP(2*2);
			add_assoc_string(*subarray, "type", "NAPTR", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pref", n);

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "flags", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "services", (char*)cp, n, 1);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(*subarray, "regex", (char*)cp, n, 1);
			cp += n;

			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "replacement", name, 1);
			break;
		default:
			zval_ptr_dtor(subarray);
			*subarray = NULL;
			cp += dlen;
			break;
	}
			if (have_v6_break && in_v6_break) {
				tp[0] = ':';
				tp++;
			}
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
				if (n < 0) {
					return NULL;
				}
				cp += n;
				add_assoc_string(*subarray, "chain", name, 1);
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			while (an-- && cp && cp < end) {
				zval *retval;

				cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, raw, &retval);
				if (retval != NULL && store_results) {
					add_next_index_zval(return_value, retval);
				}
				while (ns-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(authns, retval);
					}
				while (ar-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, 1, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(addtl, retval);
					}