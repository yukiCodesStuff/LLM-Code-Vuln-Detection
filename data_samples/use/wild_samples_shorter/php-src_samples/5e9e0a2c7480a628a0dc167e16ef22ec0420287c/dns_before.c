
#if HAVE_FULL_DNS_FUNCS

/* {{{ php_parserr */
static u_char *php_parserr(u_char *cp, querybuf *answer, int type_to_fetch, int store, int raw, zval **subarray)
{
	u_short type, class, dlen;
	u_long ttl;
	long n, i;

	*subarray = NULL;

	n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, sizeof(name) - 2);
	if (n < 0) {
		return NULL;
	}
	cp += n;

	GETSHORT(type, cp);
	GETSHORT(class, cp);
	GETLONG(ttl, cp);
	GETSHORT(dlen, cp);
	if (type_to_fetch != T_ANY && type != type_to_fetch) {
		cp += dlen;
		return cp;
	}

	switch (type) {
		case DNS_T_A:
			add_assoc_string(*subarray, "type", "A", 1);
			snprintf(name, sizeof(name), "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
			add_assoc_string(*subarray, "ip", name, 1);
			cp += dlen;
			break;
		case DNS_T_MX:
			add_assoc_string(*subarray, "type", "MX", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			/* no break; */
			if (type == DNS_T_PTR) {
				add_assoc_string(*subarray, "type", "PTR", 1);
			}
			n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
		case DNS_T_HINFO:
			/* See RFC 1010 for values */
			add_assoc_string(*subarray, "type", "HINFO", 1);
			n = *cp & 0xFF;
			cp++;
			add_assoc_stringl(*subarray, "cpu", (char*)cp, n, 1);
			cp += n;
			n = *cp & 0xFF;
			cp++;
			add_assoc_stringl(*subarray, "os", (char*)cp, n, 1);
			cp += n;
			break;
		case DNS_T_TXT:
			{
				int ll = 0;
				zval *entries = NULL;

				add_assoc_string(*subarray, "type", "TXT", 1);
				tp = emalloc(dlen + 1);
				MAKE_STD_ZVAL(entries);
				array_init(entries);
				
				while (ll < dlen) {
					n = cp[ll];
					if ((ll + n) >= dlen) {
						// Invalid chunk length, truncate
						n = dlen - (ll + 1);
					}
					memcpy(tp + ll , cp + ll + 1, n);
					add_next_index_stringl(entries, cp + ll + 1, n, 1);
					ll = ll + n + 1;
				}
				tp[dlen] = '\0';
				cp += dlen;

				add_assoc_stringl(*subarray, "txt", tp, (dlen>0)?dlen - 1:0, 0);
				add_assoc_zval(*subarray, "entries", entries);
			}
			break;
		case DNS_T_SOA:
			add_assoc_string(*subarray, "type", "SOA", 1);
			n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "mname", name, 1);
			n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) -2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "rname", name, 1);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "serial", n);
			GETLONG(n, cp);
			add_assoc_long(*subarray, "refresh", n);
			break;
		case DNS_T_AAAA:
			tp = (u_char*)name;
			for(i=0; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
		case DNS_T_A6:
			p = cp;
			add_assoc_string(*subarray, "type", "A6", 1);
			n = ((int)cp[0]) & 0xFF;
			cp++;
			add_assoc_long(*subarray, "masklen", n);
			tp = (u_char*)name;
				cp++;
			}
			for (i = (n + 8) / 16; i < 8; i++) {
				GETSHORT(s, cp);
				if (s != 0) {
					if (tp > (u_char *)name) {
						in_v6_break = 0;
			tp[0] = '\0';
			add_assoc_string(*subarray, "ipv6", name, 1);
			if (cp < p + dlen) {
				n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
				if (n < 0) {
					return NULL;
				}
				cp += n;
			}
			break;
		case DNS_T_SRV:
			add_assoc_string(*subarray, "type", "SRV", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pri", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "weight", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "port", n);
			n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			add_assoc_string(*subarray, "target", name, 1);
			break;
		case DNS_T_NAPTR:
			add_assoc_string(*subarray, "type", "NAPTR", 1);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "order", n);
			GETSHORT(n, cp);
			add_assoc_long(*subarray, "pref", n);
			n = (cp[0] & 0xFF);
			add_assoc_stringl(*subarray, "flags", (char*)++cp, n, 1);
			cp += n;
			n = (cp[0] & 0xFF);
			add_assoc_stringl(*subarray, "services", (char*)++cp, n, 1);
			cp += n;
			n = (cp[0] & 0xFF);
			add_assoc_stringl(*subarray, "regex", (char*)++cp, n, 1);
			cp += n;
			n = dn_expand(answer->qb2, answer->qb2+65536, cp, name, (sizeof name) - 2);
			if (n < 0) {
				return NULL;
			}
			cp += n;
			while (an-- && cp && cp < end) {
				zval *retval;

				cp = php_parserr(cp, &answer, type_to_fetch, store_results, raw, &retval);
				if (retval != NULL && store_results) {
					add_next_index_zval(return_value, retval);
				}
			}
				while (ns-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, &answer, DNS_T_ANY, authns != NULL, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(authns, retval);
					}
				}
				while (ar-- > 0 && cp && cp < end) {
					zval *retval = NULL;

					cp = php_parserr(cp, &answer, DNS_T_ANY, 1, raw, &retval);
					if (retval != NULL) {
						add_next_index_zval(addtl, retval);
					}
				}