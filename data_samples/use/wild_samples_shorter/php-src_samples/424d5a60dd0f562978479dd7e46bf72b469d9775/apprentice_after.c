			return -1;
		}
		if (m->type == FILE_REGEX) {
			zval pattern;
			int options = 0;
			pcre_cache_entry *pce;

			convert_libmagic_pattern(&pattern, m->value.s, strlen(m->value.s), options);

			if ((pce = pcre_get_compiled_regex_cache(Z_STR(pattern))) == NULL) {
				zval_dtor(&pattern);
				return -1;
			}
			zval_dtor(&pattern);

			return 0;
		}
		return 0;
	default:
		if (m->reln == 'x')