				return 0;
			}

			if (m->str_flags & REGEX_LINE_COUNT) {
				linecnt = m->str_range;
				bytecnt = linecnt * 80;
			} else {
				linecnt = 0;
				bytecnt = m->str_range;
			}

			if (bytecnt == 0 || bytecnt > nbytes - offset)
				bytecnt = nbytes - offset;
			if (bytecnt > ms->regex_max)
				bytecnt = ms->regex_max;

			buf = RCAST(const char *, s) + offset;
			end = last = RCAST(const char *, s) + bytecnt + offset;
			/* mget() guarantees buf <= last */
			for (lines = linecnt, b = buf; lines && b < end &&
			     ((b = CAST(const char *,
				 memchr(c = b, '\n', CAST(size_t, (end - b)))))
					b++;
			}
			if (lines)
				last = end;

			ms->search.s = buf;
			ms->search.s_len = last - buf;
			ms->search.offset = offset;