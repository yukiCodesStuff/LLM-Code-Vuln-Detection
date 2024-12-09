	if (_options != NULL) {
		unsigned barker;

		options_orig = kstrdup(_options, GFP_KERNEL);
		if (options_orig == NULL)
			goto error_parse;
		options = options_orig;

		while ((token = strsep(&options, ",")) != NULL) {
			if (*token == '\0')	/* eat joint commas */
				continue;
			if (sscanf(token, "%x", &barker) != 1
			    || barker > 0xffffffff) {
				printk(KERN_ERR "%s: can't recognize "
				       "i2400m.barkers value '%s' as "
				       "a 32-bit number\n",
				       __func__, token);
				result = -EINVAL;
				goto error_parse;
			}
			if (barker == 0) {
				/* clean list and start new */
				i2400m_barker_db_exit();
				continue;
			}
			result = i2400m_barker_db_add(barker);
			if (result < 0)
				goto error_add;
		}