		case FILE_REGEX: {
			const char *b;
			const char *c;
			const char *last;	/* end of search region */
			const char *buf;	/* start of search region */
			const char *end;
			size_t lines, linecnt, bytecnt, bytecnt_max;

			if (s == NULL) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}
			} else {
				linecnt = 0;
				bytecnt = m->str_range;
			}

			/* XXX bytecnt_max is to be kept for PHP, see cve-2014-3538.
				PCRE might stuck if the input buffer is too big. To ensure
				the correctness, the check for bytecnt > nbytes is also
				kept (might be abundant). */
			bytecnt_max = nbytes - offset;
			bytecnt_max = bytecnt_max > (1 << 14) ? (1 << 14) : bytecnt_max;
			bytecnt_max = bytecnt > nbytes ? nbytes : bytecnt_max;
			if (bytecnt == 0 || bytecnt > bytecnt_max)
				bytecnt = bytecnt_max;

			buf = RCAST(const char *, s) + offset;
			end = last = RCAST(const char *, s) + bytecnt;
			/* mget() guarantees buf <= last */
			for (lines = linecnt, b = buf; lines && b < end &&
			     ((b = CAST(const char *,
				 memchr(c = b, '\n', CAST(size_t, (end - b)))))
			     || (b = CAST(const char *,
				 memchr(c, '\r', CAST(size_t, (end - c))))));
			     lines--, b++) {
				last = b;
				if (b[0] == '\r' && b[1] == '\n')
					b++;
			}
			if (lines)
				last = RCAST(const char *, s) + bytecnt;

			ms->search.s = buf;
			ms->search.s_len = last - buf;
			ms->search.offset = offset;
			ms->search.rm_len = 0;
			return 0;
		}
		case FILE_BESTRING16:
		case FILE_LESTRING16: {
			const unsigned char *src = s + offset;
			const unsigned char *esrc = s + nbytes;
			char *dst = p->s;
			char *edst = &p->s[sizeof(p->s) - 1];

			if (type == FILE_BESTRING16)
				src++;

			/* check that offset is within range */
			if (offset >= nbytes)
				break;
			for (/*EMPTY*/; src < esrc; src += 2, dst++) {
				if (dst < edst)
					*dst = *src;
				else
					break;
				if (*dst == '\0') {
					if (type == FILE_BESTRING16 ?
					    *(src - 1) != '\0' :
					    *(src + 1) != '\0')
						*dst = ' ';
				}