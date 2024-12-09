		return;
	}

	if(hostname_len > MAXHOSTNAMELEN) {
		/* name too long, protect from CVE-2015-0235 */
		php_error_docref(NULL, E_WARNING, "Host name is too long, the limit is %d characters", MAXHOSTNAMELEN);
		RETURN_STRINGL(hostname, hostname_len, 1);
	}
	addr = php_gethostbyname(hostname);

	RETVAL_STRING(addr, 0);
}
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