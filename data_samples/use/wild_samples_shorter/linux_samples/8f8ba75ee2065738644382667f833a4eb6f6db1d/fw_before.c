		unsigned barker;

		options_orig = kstrdup(_options, GFP_KERNEL);
		if (options_orig == NULL)
			goto error_parse;
		options = options_orig;

		while ((token = strsep(&options, ",")) != NULL) {
			if (*token == '\0')	/* eat joint commas */