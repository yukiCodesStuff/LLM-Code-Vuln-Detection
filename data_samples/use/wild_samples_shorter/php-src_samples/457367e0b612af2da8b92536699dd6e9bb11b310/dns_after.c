		return;
	}

	if(hostname_len > MAXHOSTNAMELEN) {
		/* name too long, protect from CVE-2015-0235 */
		php_error_docref(NULL, E_WARNING, "Host name is too long, the limit is %d characters", MAXHOSTNAMELEN);
		RETURN_STRINGL(hostname, hostname_len);
	}

	RETURN_STR(php_gethostbyname(hostname));
}
/* }}} */

		return;
	}

	if(hostname_len > MAXHOSTNAMELEN) {
		/* name too long, protect from CVE-2015-0235 */
		php_error_docref(NULL, E_WARNING, "Host name is too long, the limit is %d characters", MAXHOSTNAMELEN);
		RETURN_FALSE;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL || hp->h_addr_list == NULL) {
		RETURN_FALSE;
	}