	if (_options != NULL) {
		unsigned barker;

		options_orig = kstrdup(_options, GFP_KERNEL);
		if (options_orig == NULL) {
			result = -ENOMEM;
			goto error_parse;
		}