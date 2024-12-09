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
					b++;
			}
			if (lines)
				last = RCAST(const char *, s) + bytecnt;

			ms->search.s = buf;
			ms->search.s_len = last - buf;
			ms->search.offset = offset;