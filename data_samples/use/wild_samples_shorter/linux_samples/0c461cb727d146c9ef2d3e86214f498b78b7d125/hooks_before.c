		return error;

	/* Obtain a SID for the context, if one was specified. */
	if (size && str[1] && str[1] != '\n') {
		if (str[size-1] == '\n') {
			str[size-1] = 0;
			size--;
		}