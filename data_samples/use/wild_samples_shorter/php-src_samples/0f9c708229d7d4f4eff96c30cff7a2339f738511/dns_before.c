		return;
	}

	addr = php_gethostbyname(hostname);

	RETVAL_STRING(addr, 0);
}
		return;
	}

	hp = gethostbyname(hostname);
	if (hp == NULL || hp->h_addr_list == NULL) {
		RETURN_FALSE;
	}