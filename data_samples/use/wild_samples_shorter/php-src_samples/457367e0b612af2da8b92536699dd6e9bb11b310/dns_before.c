		return;
	}

	RETURN_STR(php_gethostbyname(hostname));
}
/* }}} */

		return;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL || hp->h_addr_list == NULL) {
		RETURN_FALSE;
	}