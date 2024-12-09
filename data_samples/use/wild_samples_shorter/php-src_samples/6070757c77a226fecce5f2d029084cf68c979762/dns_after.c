
#if HAVE_FULL_DNS_FUNCS

#define CHECKCP(n) do { \
	if (cp + n > end) { \
		return NULL; \
	} \
} while (0)

/* {{{ php_parserr */
static u_char *php_parserr(u_char *cp, u_char *end, querybuf *answer, int type_to_fetch, int store, int raw, zval *subarray)
{
	u_short type, class, dlen;
	u_long ttl;
	long n, i;

	ZVAL_UNDEF(subarray);

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

	switch (type) {
		case DNS_T_A:
			CHECKCP(4);
			add_assoc_string(subarray, "type", "A");
			snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
			add_assoc_string(subarray, "ip", name);
			cp += dlen;
			break;
		case DNS_T_MX:
			CHECKCP(2);
			add_assoc_string(subarray, "type", "MX");
			GETSHORT(n, cp);
			add_assoc_long(subarray, "pri", n);
			/* no break; */
			if (type == DNS_T_PTR) {
				add_assoc_string(subarray, "type", "PTR");
			}
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(subarray, "type", "HINFO");
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(subarray, "cpu", (char*)cp, n);
			cp += n;
			CHECKCP(1);
			n = *cp & 0xFF;
			cp++;
			CHECKCP(n);
			add_assoc_stringl(subarray, "os", (char*)cp, n);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int l1 = 0, l2 = 0;
				zval entries;
				zend_string *tp;

				add_assoc_string(subarray, "type", "TXT");
				
				array_init(&entries);
				
				while (l1 < dlen) {
					n = cp[l1];
					if ((l1 + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (l1 + 1);
					}
					if (n) {
						memcpy(tp->val + l2 , cp + l1 + 1, n);
						add_next_index_stringl(&entries, cp + l1 + 1, n);
					}
					l1 = l1 + n + 1;
					l2 = l2 + n;
				}
				tp->val[l2] = '\0';
				tp->len = l2;
				cp += dlen;

				add_assoc_str(subarray, "txt", tp);
				add_assoc_zval(subarray, "entries", &entries);
			break;
		case DNS_T_SOA:
			add_assoc_string(subarray, "type", "SOA");
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(subarray, "mname", name);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(subarray, "rname", name);
			CHECKCP(5*4);
			GETLONG(n, cp);
			add_assoc_long(subarray, "serial", n);
			GETLONG(n, cp);
			add_assoc_long(subarray, "refresh", n);
			break;
		case DNS_T_AAAA:
			tp = (u_char*)name;
			CHECKCP(8*2);
			for(i=0; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
		case DNS_T_A6:
			p = cp;
			add_assoc_string(subarray, "type", "A6");
			CHECKCP(1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(subarray, "masklen", n);
			tp = (u_char*)name;
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				CHECKCP(2);
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
			tp[0] = '\0';
			add_assoc_string(subarray, "ipv6", name);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
			}
			break;
		case DNS_T_SRV:
			CHECKCP(3*2);
			add_assoc_string(subarray, "type", "SRV");
			GETSHORT(n, cp);
			add_assoc_long(subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(subarray, "port", n);
			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(subarray, "target", name);
			break;
		case DNS_T_NAPTR:
			CHECKCP(2*2);
			add_assoc_string(subarray, "type", "NAPTR");
			GETSHORT(n, cp);
			add_assoc_long(subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(subarray, "pref", n);

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(subarray, "flags", (char*)cp, n);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(subarray, "services", (char*)cp, n);
			cp += n;

			CHECKCP(1);
			n = (cp[0] & 0xFF);
			cp++;
			CHECKCP(n);
			add_assoc_stringl(subarray, "regex", (char*)cp, n);
			cp += n;

			n = dn_expand(answer->qb2, end, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			while (an-- && cp && cp < end) {
				zval retval;

				cp = php_parserr(cp, end, &answer, type_to_fetch, store_results, raw, &retval);
				if (Z_TYPE(retval) != IS_UNDEF && store_results) {
					add_next_index_zval(return_value, &retval);
				}
			}
				while (ns-- > 0 && cp && cp < end) {
					zval retval;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (Z_TYPE(retval) != IS_UNDEF) {
						add_next_index_zval(authns, &retval);
					}
				}
				while (ar-- > 0 && cp && cp < end) {
					zval retval;

					cp = php_parserr(cp, end, &answer, DNS_T_ANY, 1, raw, &retval);
					if (Z_TYPE(retval) != IS_UNDEF) {
						add_next_index_zval(addtl, &retval);
					}
				}