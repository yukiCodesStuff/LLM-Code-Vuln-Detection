			if (s == NULL || nbytes < offset) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}

			/* bytecnt checks are to be kept for PHP, see cve-2014-3538.
			 PCRE might get stuck if the input buffer is too big. */
			linecnt = m->str_range;
			bytecnt = linecnt * 80;

			if (bytecnt == 0) {
				bytecnt = 1 << 14;
			}

			if (bytecnt > nbytes) {
				bytecnt = nbytes;
			}
			if (offset > bytecnt) {
				offset = bytecnt;
			}
			if (s == NULL) {
				ms->search.s_len = 0;
				ms->search.s = NULL;
				return 0;
			}
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
				if (b < end - 1 && b[0] == '\r' && b[1] == '\n')
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
					    ((src + 1 < esrc) &&
					    *(src + 1) != '\0'))
						*dst = ' ';
				}
			     lines--, b++) {
				last = b;
				if (b < end - 1 && b[0] == '\r' && b[1] == '\n')
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
					    ((src + 1 < esrc) &&
					    *(src + 1) != '\0'))
						*dst = ' ';
				}