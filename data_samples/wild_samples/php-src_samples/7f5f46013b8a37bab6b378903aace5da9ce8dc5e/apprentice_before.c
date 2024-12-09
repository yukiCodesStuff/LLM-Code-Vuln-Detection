		if (*p == NULL) {
			if (ms->flags & MAGIC_CHECK)
				file_magwarn(ms, "cannot get string from `%s'",
				    m->value.s);
			return -1;
		}
		if (m->type == FILE_REGEX) {
			/*  XXX do we need this? */
			/*zval pattern;
			int options = 0;
			pcre_cache_entry *pce;

			convert_libmagic_pattern(&pattern, m->value.s, strlen(m->value.s), options);

			if ((pce = pcre_get_compiled_regex_cache(Z_STR(pattern))) == NULL) {
				return -1;
			}